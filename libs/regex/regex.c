#include "regex.h"

#include <locale.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>

typedef struct
{
	char* data;
	size_t length;
	size_t capacity;
} AdnRegexStringBuilder;

typedef struct
{
	uint32_t start;
	uint32_t end;
} AdnRegexRange;

typedef struct
{
	int negated;
	unsigned int builtins;
	AdnRegexRange* ranges;
	size_t range_count;
	size_t range_capacity;
} AdnRegexCharClass;

typedef enum
{
	ADN_REGEX_NODE_EMPTY,
	ADN_REGEX_NODE_LITERAL,
	ADN_REGEX_NODE_DOT,
	ADN_REGEX_NODE_CLASS,
	ADN_REGEX_NODE_SEQUENCE,
	ADN_REGEX_NODE_ALTERNATION,
	ADN_REGEX_NODE_REPEAT,
	ADN_REGEX_NODE_GROUP,
	ADN_REGEX_NODE_BACKREF,
	ADN_REGEX_NODE_ASSERT_START,
	ADN_REGEX_NODE_ASSERT_END,
	ADN_REGEX_NODE_WORD_BOUNDARY,
	ADN_REGEX_NODE_NOT_WORD_BOUNDARY,
	ADN_REGEX_NODE_LOOK
} AdnRegexNodeKind;

typedef struct AdnRegexNode AdnRegexNode;

struct AdnRegexNode
{
	AdnRegexNodeKind kind;
	union
	{
		uint32_t literal;
		AdnRegexCharClass* char_class;
		struct
		{
			AdnRegexNode** items;
			size_t count;
		} list;
		struct
		{
			AdnRegexNode* child;
			int min;
			int max;
			int greedy;
		} repeat;
		struct
		{
			AdnRegexNode* child;
			int capturing;
			int index;
		} group;
		int backref_index;
		struct
		{
			AdnRegexNode* child;
			int positive;
			int behind;
		} look;
	} as;
};

typedef struct
{
	const char* pattern;
	size_t length;
	size_t index;
	int error;
	int capture_count;
} AdnRegexParser;

typedef struct
{
	AdnRegexNode* root;
	int capture_count;
} AdnRegexProgram;

typedef struct
{
	uint32_t* codepoints;
	size_t* offsets;
	size_t length;
} AdnRegexText;

typedef struct
{
	size_t start;
	size_t end;
	int defined;
} AdnRegexCapture;

typedef struct
{
	size_t start;
	size_t end;
	AdnRegexCapture* captures;
} AdnRegexMatch;

enum
{
	ADN_REGEX_CLASS_DIGIT = 1u << 0,
	ADN_REGEX_CLASS_SPACE = 1u << 1,
	ADN_REGEX_CLASS_WORD = 1u << 2,
	ADN_REGEX_CLASS_ALPHA = 1u << 3,
	ADN_REGEX_CLASS_ALNUM = 1u << 4,
	ADN_REGEX_CLASS_LOWER = 1u << 5,
	ADN_REGEX_CLASS_UPPER = 1u << 6,
	ADN_REGEX_CLASS_XDIGIT = 1u << 7
};

typedef struct
{
	AdnRegexNode** items;
	size_t count;
	size_t capacity;
} AdnRegexNodeList;

typedef struct
{
	int is_literal;
	uint32_t literal;
	unsigned int builtins;
} AdnRegexClassItem;

static void adn_regex_builder_init(AdnRegexStringBuilder* builder)
{
	if (!builder)
	{
		return;
	}
	builder->capacity = 64;
	builder->length = 0;
	builder->data = (char*)calloc(builder->capacity, 1);
}

static void adn_regex_builder_reserve(AdnRegexStringBuilder* builder,
	                                  size_t additional)
{
	if (!builder)
	{
		return;
	}
	if (builder->length + additional + 1 <= builder->capacity)
	{
		return;
	}
	size_t next_capacity = builder->capacity == 0 ? 64 : builder->capacity;
	while (builder->length + additional + 1 > next_capacity)
	{
		next_capacity *= 2;
	}
	char* resized = (char*)realloc(builder->data, next_capacity);
	if (!resized)
	{
		return;
	}
	builder->data = resized;
	builder->capacity = next_capacity;
}

static void adn_regex_builder_append_n(AdnRegexStringBuilder* builder,
	                                   const char* text, size_t length)
{
	if (!builder || !text)
	{
		return;
	}
	adn_regex_builder_reserve(builder, length);
	memcpy(builder->data + builder->length, text, length);
	builder->length += length;
	builder->data[builder->length] = '\0';
}

static void adn_regex_builder_append(AdnRegexStringBuilder* builder,
	                                 const char* text)
{
	if (!text)
	{
		return;
	}
	adn_regex_builder_append_n(builder, text, strlen(text));
}

static void adn_regex_builder_append_char(AdnRegexStringBuilder* builder, char ch)
{
	if (!builder)
	{
		return;
	}
	adn_regex_builder_reserve(builder, 1);
	builder->data[builder->length++] = ch;
	builder->data[builder->length] = '\0';
}

static char* adn_regex_builder_finish(AdnRegexStringBuilder* builder)
{
	if (!builder)
	{
		return NULL;
	}
	if (!builder->data)
	{
		return strdup("");
	}
	char* result = builder->data;
	builder->data = NULL;
	builder->length = 0;
	builder->capacity = 0;
	return result;
}

static void adn_regex_init_locale(void)
{
	static int initialized = 0;
	if (!initialized)
	{
		setlocale(LC_CTYPE, "");
		initialized = 1;
	}
}

static char* adn_regex_unwrap_string(const char* value)
{
	if (!value)
	{
		return strdup("");
	}

	size_t len = strlen(value);
	const char* start = value;
	if (len >= 2 && ((value[0] == '"' && value[len - 1] == '"') ||
	                 (value[0] == '\'' && value[len - 1] == '\'') ||
	                 (value[0] == '`' && value[len - 1] == '`')))
	{
		start = value + 1;
		len -= 2;
	}

	char* result = (char*)malloc(len + 1);
	if (!result)
	{
		return NULL;
	}
	memcpy(result, start, len);
	result[len] = '\0';
	return result;
}

static size_t adn_regex_decode_utf8_one(const char* text, size_t length,
	                                    size_t index, uint32_t* out)
{
	unsigned char c0;
	uint32_t codepoint;

	if (!text || index >= length)
	{
		if (out)
		{
			*out = 0;
		}
		return index;
	}

	c0 = (unsigned char)text[index];
	if (c0 < 0x80)
	{
		if (out)
		{
			*out = (uint32_t)c0;
		}
		return index + 1;
	}

	if ((c0 & 0xE0) == 0xC0 && index + 1 < length)
	{
		unsigned char c1 = (unsigned char)text[index + 1];
		if ((c1 & 0xC0) == 0x80)
		{
			codepoint = ((uint32_t)(c0 & 0x1F) << 6) | (uint32_t)(c1 & 0x3F);
			if (codepoint >= 0x80)
			{
				if (out)
				{
					*out = codepoint;
				}
				return index + 2;
			}
		}
	}

	if ((c0 & 0xF0) == 0xE0 && index + 2 < length)
	{
		unsigned char c1 = (unsigned char)text[index + 1];
		unsigned char c2 = (unsigned char)text[index + 2];
		if ((c1 & 0xC0) == 0x80 && (c2 & 0xC0) == 0x80)
		{
			codepoint = ((uint32_t)(c0 & 0x0F) << 12) |
			            ((uint32_t)(c1 & 0x3F) << 6) | (uint32_t)(c2 & 0x3F);
			if (codepoint >= 0x800)
			{
				if (out)
				{
					*out = codepoint;
				}
				return index + 3;
			}
		}
	}

	if ((c0 & 0xF8) == 0xF0 && index + 3 < length)
	{
		unsigned char c1 = (unsigned char)text[index + 1];
		unsigned char c2 = (unsigned char)text[index + 2];
		unsigned char c3 = (unsigned char)text[index + 3];
		if ((c1 & 0xC0) == 0x80 && (c2 & 0xC0) == 0x80 &&
		    (c3 & 0xC0) == 0x80)
		{
			codepoint = ((uint32_t)(c0 & 0x07) << 18) |
			            ((uint32_t)(c1 & 0x3F) << 12) |
			            ((uint32_t)(c2 & 0x3F) << 6) | (uint32_t)(c3 & 0x3F);
			if (codepoint >= 0x10000 && codepoint <= 0x10FFFF)
			{
				if (out)
				{
					*out = codepoint;
				}
				return index + 4;
			}
		}
	}

	if (out)
	{
		*out = (uint32_t)c0;
	}
	return index + 1;
}

static void adn_regex_text_free(AdnRegexText* text)
{
	if (!text)
	{
		return;
	}
	free(text->codepoints);
	free(text->offsets);
	text->codepoints = NULL;
	text->offsets = NULL;
	text->length = 0;
}

static int adn_regex_text_init(AdnRegexText* view, const char* text)
{
	size_t length;
	size_t index = 0;
	size_t count = 0;

	if (!view)
	{
		return 0;
	}
	memset(view, 0, sizeof(*view));
	if (!text)
	{
		return 1;
	}

	length = strlen(text);
	view->codepoints = (uint32_t*)calloc(length + 1, sizeof(uint32_t));
	view->offsets = (size_t*)calloc(length + 1, sizeof(size_t));
	if (!view->codepoints || !view->offsets)
	{
		adn_regex_text_free(view);
		return 0;
	}

	while (index < length)
	{
		uint32_t codepoint = 0;
		view->offsets[count] = index;
		index = adn_regex_decode_utf8_one(text, length, index, &codepoint);
		view->codepoints[count++] = codepoint;
	}
	view->offsets[count] = length;
	view->length = count;
	return 1;
}

static int adn_regex_is_newline(uint32_t codepoint)
{
	return codepoint == '\n' || codepoint == '\r' || codepoint == 0x2028 ||
	       codepoint == 0x2029;
}

static int adn_regex_is_space(uint32_t codepoint)
{
	adn_regex_init_locale();
	if (codepoint <= (uint32_t)WCHAR_MAX)
	{
		return iswspace((wint_t)codepoint) != 0;
	}
	return codepoint == 0x2028 || codepoint == 0x2029;
}

static int adn_regex_is_digit(uint32_t codepoint)
{
	adn_regex_init_locale();
	if (codepoint <= (uint32_t)WCHAR_MAX)
	{
		return iswdigit((wint_t)codepoint) != 0;
	}
	return 0;
}

static int adn_regex_is_alpha(uint32_t codepoint)
{
	adn_regex_init_locale();
	if (codepoint <= (uint32_t)WCHAR_MAX)
	{
		return iswalpha((wint_t)codepoint) != 0;
	}
	return 0;
}

static int adn_regex_is_alnum(uint32_t codepoint)
{
	adn_regex_init_locale();
	if (codepoint <= (uint32_t)WCHAR_MAX)
	{
		return iswalnum((wint_t)codepoint) != 0;
	}
	return 0;
}

static int adn_regex_is_lower(uint32_t codepoint)
{
	adn_regex_init_locale();
	if (codepoint <= (uint32_t)WCHAR_MAX)
	{
		return iswlower((wint_t)codepoint) != 0;
	}
	return 0;
}

static int adn_regex_is_upper(uint32_t codepoint)
{
	adn_regex_init_locale();
	if (codepoint <= (uint32_t)WCHAR_MAX)
	{
		return iswupper((wint_t)codepoint) != 0;
	}
	return 0;
}

static int adn_regex_is_xdigit(uint32_t codepoint)
{
	adn_regex_init_locale();
	if (codepoint <= (uint32_t)WCHAR_MAX)
	{
		return iswxdigit((wint_t)codepoint) != 0;
	}
	return 0;
}

static int adn_regex_is_word(uint32_t codepoint)
{
	return codepoint == '_' || adn_regex_is_alnum(codepoint);
}

static int adn_regex_hex_value(int ch)
{
	if (ch >= '0' && ch <= '9')
	{
		return ch - '0';
	}
	if (ch >= 'a' && ch <= 'f')
	{
		return 10 + (ch - 'a');
	}
	if (ch >= 'A' && ch <= 'F')
	{
		return 10 + (ch - 'A');
	}
	return -1;
}

static int adn_regex_parse_hex_digits(AdnRegexParser* parser, size_t digits,
	                                  uint32_t* out)
{
	uint32_t value = 0;
	size_t i;

	if (!parser || !out || parser->index + digits > parser->length)
	{
		return 0;
	}

	for (i = 0; i < digits; i++)
	{
		int digit = adn_regex_hex_value((unsigned char)parser->pattern[parser->index + i]);
		if (digit < 0)
		{
			return 0;
		}
		value = (value << 4) | (uint32_t)digit;
	}

	parser->index += digits;
	*out = value;
	return 1;
}

static AdnRegexNode* adn_regex_node_new(AdnRegexNodeKind kind)
{
	AdnRegexNode* node = (AdnRegexNode*)calloc(1, sizeof(AdnRegexNode));
	if (!node)
	{
		return NULL;
	}
	node->kind = kind;
	return node;
}

static AdnRegexCharClass* adn_regex_char_class_new(void)
{
	return (AdnRegexCharClass*)calloc(1, sizeof(AdnRegexCharClass));
}

static int adn_regex_char_class_add_range(AdnRegexCharClass* char_class,
	                                      uint32_t start, uint32_t end)
{
	if (!char_class)
	{
		return 0;
	}
	if (start > end)
	{
		uint32_t swap = start;
		start = end;
		end = swap;
	}
	if (char_class->range_count + 1 > char_class->range_capacity)
	{
		size_t next_capacity = char_class->range_capacity == 0 ? 4 : char_class->range_capacity * 2;
		AdnRegexRange* ranges = (AdnRegexRange*)realloc(
		    char_class->ranges, sizeof(AdnRegexRange) * next_capacity);
		if (!ranges)
		{
			return 0;
		}
		char_class->ranges = ranges;
		char_class->range_capacity = next_capacity;
	}
	char_class->ranges[char_class->range_count].start = start;
	char_class->ranges[char_class->range_count].end = end;
	char_class->range_count++;
	return 1;
}

static void adn_regex_char_class_free(AdnRegexCharClass* char_class)
{
	if (!char_class)
	{
		return;
	}
	free(char_class->ranges);
	free(char_class);
}

static void adn_regex_node_free(AdnRegexNode* node)
{
	size_t i;

	if (!node)
	{
		return;
	}

	switch (node->kind)
	{
		case ADN_REGEX_NODE_CLASS:
			adn_regex_char_class_free(node->as.char_class);
			break;
		case ADN_REGEX_NODE_SEQUENCE:
		case ADN_REGEX_NODE_ALTERNATION:
			for (i = 0; i < node->as.list.count; i++)
			{
				adn_regex_node_free(node->as.list.items[i]);
			}
			free(node->as.list.items);
			break;
		case ADN_REGEX_NODE_REPEAT:
			adn_regex_node_free(node->as.repeat.child);
			break;
		case ADN_REGEX_NODE_GROUP:
			adn_regex_node_free(node->as.group.child);
			break;
		case ADN_REGEX_NODE_LOOK:
			adn_regex_node_free(node->as.look.child);
			break;
		default:
			break;
	}

	free(node);
}

static void adn_regex_program_free(AdnRegexProgram* program)
{
	if (!program)
	{
		return;
	}
	adn_regex_node_free(program->root);
	program->root = NULL;
	program->capture_count = 0;
}

static int adn_regex_node_list_push(AdnRegexNodeList* list, AdnRegexNode* node)
{
	if (!list || !node)
	{
		return 0;
	}
	if (list->count + 1 > list->capacity)
	{
		size_t next_capacity = list->capacity == 0 ? 4 : list->capacity * 2;
		AdnRegexNode** items = (AdnRegexNode**)realloc(list->items,
		                                               sizeof(AdnRegexNode*) * next_capacity);
		if (!items)
		{
			return 0;
		}
		list->items = items;
		list->capacity = next_capacity;
	}
	list->items[list->count++] = node;
	return 1;
}

static AdnRegexNode* adn_regex_node_from_list(AdnRegexNodeKind kind,
	                                          AdnRegexNodeList* list)
{
	AdnRegexNode* node;

	if (!list || list->count == 0)
	{
		return adn_regex_node_new(ADN_REGEX_NODE_EMPTY);
	}
	if (list->count == 1)
	{
		AdnRegexNode* item = list->items[0];
		free(list->items);
		list->items = NULL;
		list->count = 0;
		list->capacity = 0;
		return item;
	}

	node = adn_regex_node_new(kind);
	if (!node)
	{
		return NULL;
	}
	node->as.list.items = list->items;
	node->as.list.count = list->count;
	list->items = NULL;
	list->count = 0;
	list->capacity = 0;
	return node;
}

static AdnRegexNode* adn_regex_parse_expression(AdnRegexParser* parser);

static uint32_t adn_regex_parse_literal_escape(AdnRegexParser* parser, int* ok)
{
	unsigned char ch;
	uint32_t codepoint = 0;

	*ok = 0;
	if (!parser || parser->index >= parser->length)
	{
		return 0;
	}

	ch = (unsigned char)parser->pattern[parser->index++];
	switch (ch)
	{
		case 'n':
			*ok = 1;
			return '\n';
		case 'r':
			*ok = 1;
			return '\r';
		case 't':
			*ok = 1;
			return '\t';
		case 'f':
			*ok = 1;
			return '\f';
		case 'v':
			*ok = 1;
			return '\v';
		case '0':
			*ok = 1;
			return 0;
		case 'x':
			if (adn_regex_parse_hex_digits(parser, 2, &codepoint))
			{
				*ok = 1;
				return codepoint;
			}
			break;
		case 'u':
			if (adn_regex_parse_hex_digits(parser, 4, &codepoint))
			{
				*ok = 1;
				return codepoint;
			}
			break;
		case 'U':
			if (adn_regex_parse_hex_digits(parser, 8, &codepoint))
			{
				*ok = 1;
				return codepoint;
			}
			break;
		default:
			*ok = 1;
			return (uint32_t)ch;
	}

	return 0;
}

static int adn_regex_parse_class_posix(const char* name)
{
	if (strcmp(name, "alpha") == 0)
	{
		return ADN_REGEX_CLASS_ALPHA;
	}
	if (strcmp(name, "alnum") == 0)
	{
		return ADN_REGEX_CLASS_ALNUM;
	}
	if (strcmp(name, "digit") == 0)
	{
		return ADN_REGEX_CLASS_DIGIT;
	}
	if (strcmp(name, "space") == 0)
	{
		return ADN_REGEX_CLASS_SPACE;
	}
	if (strcmp(name, "lower") == 0)
	{
		return ADN_REGEX_CLASS_LOWER;
	}
	if (strcmp(name, "upper") == 0)
	{
		return ADN_REGEX_CLASS_UPPER;
	}
	if (strcmp(name, "xdigit") == 0)
	{
		return ADN_REGEX_CLASS_XDIGIT;
	}
	if (strcmp(name, "word") == 0)
	{
		return ADN_REGEX_CLASS_WORD;
	}
	return 0;
}

static int adn_regex_parse_class_item(AdnRegexParser* parser,
	                                  AdnRegexClassItem* item)
{
	unsigned char ch;
	uint32_t codepoint = 0;
	int ok = 0;

	if (!parser || !item || parser->index >= parser->length)
	{
		return 0;
	}

	memset(item, 0, sizeof(*item));
	ch = (unsigned char)parser->pattern[parser->index];
	if (ch == '\\')
	{
		parser->index++;
		if (parser->index >= parser->length)
		{
			return 0;
		}
		ch = (unsigned char)parser->pattern[parser->index];
		if (ch == 'd')
		{
			parser->index++;
			item->builtins = ADN_REGEX_CLASS_DIGIT;
			return 1;
		}
		if (ch == 's')
		{
			parser->index++;
			item->builtins = ADN_REGEX_CLASS_SPACE;
			return 1;
		}
		if (ch == 'w')
		{
			parser->index++;
			item->builtins = ADN_REGEX_CLASS_WORD;
			return 1;
		}

		item->literal = adn_regex_parse_literal_escape(parser, &ok);
		item->is_literal = ok;
		return ok;
	}

	if (ch == '[' && parser->index + 1 < parser->length &&
	    parser->pattern[parser->index + 1] == ':')
	{
		size_t cursor = parser->index + 2;
		while (cursor + 1 < parser->length &&
		       !(parser->pattern[cursor] == ':' && parser->pattern[cursor + 1] == ']'))
		{
			cursor++;
		}
		if (cursor + 1 < parser->length)
		{
			size_t name_length = cursor - (parser->index + 2);
			char name[32];
			if (name_length > 0 && name_length < sizeof(name))
			{
				memcpy(name, parser->pattern + parser->index + 2, name_length);
				name[name_length] = '\0';
				item->builtins = (unsigned int)adn_regex_parse_class_posix(name);
				if (item->builtins != 0)
				{
					parser->index = cursor + 2;
					return 1;
				}
			}
		}
	}

	parser->index = adn_regex_decode_utf8_one(parser->pattern, parser->length,
	                                         parser->index, &codepoint);
	item->is_literal = 1;
	item->literal = codepoint;
	return 1;
}

static AdnRegexNode* adn_regex_parse_class(AdnRegexParser* parser)
{
	AdnRegexCharClass* char_class;
	AdnRegexNode* node;
	int saw_any = 0;
	int have_pending_literal = 0;
	uint32_t pending_literal = 0;

	if (!parser || parser->index >= parser->length || parser->pattern[parser->index] != '[')
	{
		return NULL;
	}
	parser->index++;

	char_class = adn_regex_char_class_new();
	if (!char_class)
	{
		return NULL;
	}
	if (parser->index < parser->length && parser->pattern[parser->index] == '^')
	{
		char_class->negated = 1;
		parser->index++;
	}

	while (parser->index < parser->length)
	{
		AdnRegexClassItem item = {0};
		unsigned char ch = (unsigned char)parser->pattern[parser->index];

		if (ch == ']' && saw_any)
		{
			parser->index++;
			if (have_pending_literal)
			{
				adn_regex_char_class_add_range(char_class, pending_literal, pending_literal);
			}
			node = adn_regex_node_new(ADN_REGEX_NODE_CLASS);
			if (!node)
			{
				adn_regex_char_class_free(char_class);
				return NULL;
			}
			node->as.char_class = char_class;
			return node;
		}

		if (ch == ']' && !saw_any)
		{
			parser->index++;
			item.is_literal = 1;
			item.literal = ']';
		}
		else
		{
			if (!adn_regex_parse_class_item(parser, &item))
			{
				parser->error = 1;
				adn_regex_char_class_free(char_class);
				return NULL;
			}
		}
		saw_any = 1;

		if (item.builtins != 0)
		{
			if (have_pending_literal)
			{
				adn_regex_char_class_add_range(char_class, pending_literal, pending_literal);
				have_pending_literal = 0;
			}
			char_class->builtins |= item.builtins;
			continue;
		}

		if (item.is_literal)
		{
			if (have_pending_literal)
			{
				if (parser->index < parser->length && parser->pattern[parser->index] == '-' &&
				    parser->index + 1 < parser->length &&
				    parser->pattern[parser->index + 1] != ']')
				{
					AdnRegexClassItem range_item = {0};
					parser->index++;
					if (!adn_regex_parse_class_item(parser, &range_item) ||
					    !range_item.is_literal)
					{
						parser->error = 1;
						adn_regex_char_class_free(char_class);
						return NULL;
					}
					if (!adn_regex_char_class_add_range(char_class, pending_literal,
					                                   range_item.literal))
					{
						parser->error = 1;
						adn_regex_char_class_free(char_class);
						return NULL;
					}
					have_pending_literal = 0;
					continue;
				}
				if (!adn_regex_char_class_add_range(char_class, pending_literal,
				                               pending_literal))
				{
					parser->error = 1;
					adn_regex_char_class_free(char_class);
					return NULL;
				}
			}
			pending_literal = item.literal;
			have_pending_literal = 1;
		}
	}

	parser->error = 1;
	adn_regex_char_class_free(char_class);
	return NULL;
}

static AdnRegexNode* adn_regex_parse_atom(AdnRegexParser* parser)
{
	AdnRegexNode* node;
	unsigned char ch;

	if (!parser || parser->index >= parser->length)
	{
		return NULL;
	}

	ch = (unsigned char)parser->pattern[parser->index];
	if (ch == '^')
	{
		parser->index++;
		return adn_regex_node_new(ADN_REGEX_NODE_ASSERT_START);
	}
	if (ch == '$')
	{
		parser->index++;
		return adn_regex_node_new(ADN_REGEX_NODE_ASSERT_END);
	}
	if (ch == '.')
	{
		parser->index++;
		return adn_regex_node_new(ADN_REGEX_NODE_DOT);
	}
	if (ch == '[')
	{
		return adn_regex_parse_class(parser);
	}
	if (ch == '(')
	{
		int capturing = 1;
		int group_index = 0;
		int look = 0;
		int behind = 0;
		int positive = 1;
		AdnRegexNode* child;

		parser->index++;
		if (parser->index < parser->length && parser->pattern[parser->index] == '?')
		{
			parser->index++;
			if (parser->index >= parser->length)
			{
				parser->error = 1;
				return NULL;
			}
			if (parser->pattern[parser->index] == ':')
			{
				capturing = 0;
				parser->index++;
			}
			else if (parser->pattern[parser->index] == '=')
			{
				look = 1;
				positive = 1;
				parser->index++;
			}
			else if (parser->pattern[parser->index] == '!')
			{
				look = 1;
				positive = 0;
				parser->index++;
			}
			else if (parser->pattern[parser->index] == '<' &&
			         parser->index + 1 < parser->length &&
			         (parser->pattern[parser->index + 1] == '=' ||
			          parser->pattern[parser->index + 1] == '!'))
			{
				look = 1;
				behind = 1;
				positive = parser->pattern[parser->index + 1] == '=';
				parser->index += 2;
			}
			else
			{
				parser->error = 1;
				return NULL;
			}
		}

		if (capturing)
		{
			group_index = ++parser->capture_count;
		}

		child = adn_regex_parse_expression(parser);
		if (parser->error)
		{
			return NULL;
		}
		if (parser->index >= parser->length || parser->pattern[parser->index] != ')')
		{
			parser->error = 1;
			adn_regex_node_free(child);
			return NULL;
		}
		parser->index++;

		if (look)
		{
			node = adn_regex_node_new(ADN_REGEX_NODE_LOOK);
			if (!node)
			{
				adn_regex_node_free(child);
				return NULL;
			}
			node->as.look.child = child;
			node->as.look.positive = positive;
			node->as.look.behind = behind;
			return node;
		}

		node = adn_regex_node_new(ADN_REGEX_NODE_GROUP);
		if (!node)
		{
			adn_regex_node_free(child);
			return NULL;
		}
		node->as.group.child = child;
		node->as.group.capturing = capturing;
		node->as.group.index = group_index;
		return node;
	}
	if (ch == '\\')
	{
		int ok = 0;
		uint32_t codepoint;

		parser->index++;
		if (parser->index >= parser->length)
		{
			parser->error = 1;
			return NULL;
		}

		ch = (unsigned char)parser->pattern[parser->index];
		if (ch >= '1' && ch <= '9')
		{
			int value = 0;
			while (parser->index < parser->length)
			{
				unsigned char digit = (unsigned char)parser->pattern[parser->index];
				if (digit < '0' || digit > '9')
				{
					break;
				}
				value = value * 10 + (digit - '0');
				parser->index++;
			}
			node = adn_regex_node_new(ADN_REGEX_NODE_BACKREF);
			if (!node)
			{
				return NULL;
			}
			node->as.backref_index = value;
			return node;
		}
		if (ch == 'd' || ch == 'D' || ch == 's' || ch == 'S' || ch == 'w' ||
		    ch == 'W')
		{
			AdnRegexCharClass* char_class = adn_regex_char_class_new();
			if (!char_class)
			{
				return NULL;
			}
			switch (ch)
			{
				case 'd':
					char_class->builtins = ADN_REGEX_CLASS_DIGIT;
					break;
				case 'D':
					char_class->builtins = ADN_REGEX_CLASS_DIGIT;
					char_class->negated = 1;
					break;
				case 's':
					char_class->builtins = ADN_REGEX_CLASS_SPACE;
					break;
				case 'S':
					char_class->builtins = ADN_REGEX_CLASS_SPACE;
					char_class->negated = 1;
					break;
				case 'w':
					char_class->builtins = ADN_REGEX_CLASS_WORD;
					break;
				case 'W':
					char_class->builtins = ADN_REGEX_CLASS_WORD;
					char_class->negated = 1;
					break;
			}
			parser->index++;
			node = adn_regex_node_new(ADN_REGEX_NODE_CLASS);
			if (!node)
			{
				adn_regex_char_class_free(char_class);
				return NULL;
			}
			node->as.char_class = char_class;
			return node;
		}
		if (ch == 'b' || ch == 'B')
		{
			parser->index++;
			return adn_regex_node_new(ch == 'b' ? ADN_REGEX_NODE_WORD_BOUNDARY
			                                  : ADN_REGEX_NODE_NOT_WORD_BOUNDARY);
		}

		codepoint = adn_regex_parse_literal_escape(parser, &ok);
		if (!ok)
		{
			parser->error = 1;
			return NULL;
		}
		node = adn_regex_node_new(ADN_REGEX_NODE_LITERAL);
		if (!node)
		{
			return NULL;
		}
		node->as.literal = codepoint;
		return node;
	}

	if (ch == '|' || ch == ')' || ch == '*' || ch == '+' || ch == '?')
	{
		parser->error = 1;
		return NULL;
	}

	node = adn_regex_node_new(ADN_REGEX_NODE_LITERAL);
	if (!node)
	{
		return NULL;
	}
	parser->index = adn_regex_decode_utf8_one(parser->pattern, parser->length,
	                                         parser->index, &node->as.literal);
	return node;
}

static AdnRegexNode* adn_regex_parse_term(AdnRegexParser* parser)
{
	AdnRegexNode* atom = adn_regex_parse_atom(parser);

	if (!atom || parser->error)
	{
		return atom;
	}

	if (parser->index < parser->length)
	{
		unsigned char ch = (unsigned char)parser->pattern[parser->index];
		int min = 0;
		int max = 0;
		int has_quantifier = 0;
		int greedy = 1;

		if (ch == '*')
		{
			min = 0;
			max = -1;
			has_quantifier = 1;
			parser->index++;
		}
		else if (ch == '+')
		{
			min = 1;
			max = -1;
			has_quantifier = 1;
			parser->index++;
		}
		else if (ch == '?')
		{
			min = 0;
			max = 1;
			has_quantifier = 1;
			parser->index++;
		}
		else if (ch == '{')
		{
			size_t saved = parser->index;
			int first = 0;
			int second = -1;
			int saw_digit = 0;

			parser->index++;
			while (parser->index < parser->length &&
			       parser->pattern[parser->index] >= '0' &&
			       parser->pattern[parser->index] <= '9')
			{
				saw_digit = 1;
				first = first * 10 + (parser->pattern[parser->index] - '0');
				parser->index++;
			}
			if (!saw_digit)
			{
				parser->error = 1;
				adn_regex_node_free(atom);
				return NULL;
			}
			if (parser->index < parser->length && parser->pattern[parser->index] == ',')
			{
				parser->index++;
				if (parser->index < parser->length && parser->pattern[parser->index] == '}')
				{
					second = -1;
				}
				else
				{
					second = 0;
					saw_digit = 0;
					while (parser->index < parser->length &&
					       parser->pattern[parser->index] >= '0' &&
					       parser->pattern[parser->index] <= '9')
					{
						saw_digit = 1;
						second = second * 10 + (parser->pattern[parser->index] - '0');
						parser->index++;
					}
					if (!saw_digit)
					{
						parser->error = 1;
						adn_regex_node_free(atom);
						return NULL;
					}
				}
			}
			else
			{
				second = first;
			}
			if (parser->index >= parser->length || parser->pattern[parser->index] != '}')
			{
				parser->index = saved;
			}
			else
			{
				parser->index++;
				min = first;
				max = second;
				has_quantifier = 1;
			}
		}

		if (has_quantifier)
		{
			AdnRegexNode* repeat;
			if (parser->index < parser->length && parser->pattern[parser->index] == '?')
			{
				greedy = 0;
				parser->index++;
			}
			repeat = adn_regex_node_new(ADN_REGEX_NODE_REPEAT);
			if (!repeat)
			{
				adn_regex_node_free(atom);
				return NULL;
			}
			repeat->as.repeat.child = atom;
			repeat->as.repeat.min = min;
			repeat->as.repeat.max = max;
			repeat->as.repeat.greedy = greedy;
			return repeat;
		}
	}

	return atom;
}

static AdnRegexNode* adn_regex_parse_sequence(AdnRegexParser* parser)
{
	AdnRegexNodeList list = {0};

	while (parser && parser->index < parser->length)
	{
		unsigned char ch = (unsigned char)parser->pattern[parser->index];
		AdnRegexNode* term;
		if (ch == '|' || ch == ')')
		{
			break;
		}
		term = adn_regex_parse_term(parser);
		if (!term || parser->error)
		{
			for (size_t i = 0; i < list.count; i++)
			{
				adn_regex_node_free(list.items[i]);
			}
			free(list.items);
			return NULL;
		}
		if (!adn_regex_node_list_push(&list, term))
		{
			adn_regex_node_free(term);
			for (size_t i = 0; i < list.count; i++)
			{
				adn_regex_node_free(list.items[i]);
			}
			free(list.items);
			return NULL;
		}
	}

	return adn_regex_node_from_list(ADN_REGEX_NODE_SEQUENCE, &list);
}

static AdnRegexNode* adn_regex_parse_expression(AdnRegexParser* parser)
{
	AdnRegexNodeList branches = {0};
	AdnRegexNode* first = adn_regex_parse_sequence(parser);

	if (!first || parser->error)
	{
		return NULL;
	}
	if (!adn_regex_node_list_push(&branches, first))
	{
		adn_regex_node_free(first);
		return NULL;
	}

	while (parser->index < parser->length && parser->pattern[parser->index] == '|')
	{
		AdnRegexNode* branch;
		parser->index++;
		branch = adn_regex_parse_sequence(parser);
		if (!branch || parser->error)
		{
			for (size_t i = 0; i < branches.count; i++)
			{
				adn_regex_node_free(branches.items[i]);
			}
			free(branches.items);
			return NULL;
		}
		if (!adn_regex_node_list_push(&branches, branch))
		{
			adn_regex_node_free(branch);
			for (size_t i = 0; i < branches.count; i++)
			{
				adn_regex_node_free(branches.items[i]);
			}
			free(branches.items);
			return NULL;
		}
	}

	return adn_regex_node_from_list(ADN_REGEX_NODE_ALTERNATION, &branches);
}

static int adn_regex_program_init(AdnRegexProgram* program, const char* pattern)
{
	AdnRegexParser parser;

	if (!program)
	{
		return 0;
	}
	memset(program, 0, sizeof(*program));
	memset(&parser, 0, sizeof(parser));
	parser.pattern = pattern ? pattern : "";
	parser.length = strlen(parser.pattern);
	program->root = adn_regex_parse_expression(&parser);
	program->capture_count = parser.capture_count;
	if (!program->root || parser.error || parser.index != parser.length)
	{
		adn_regex_program_free(program);
		return 0;
	}
	return 1;
}

static AdnRegexCapture* adn_regex_captures_clone(const AdnRegexCapture* captures,
	                                            size_t count)
{
	AdnRegexCapture* clone = (AdnRegexCapture*)calloc(count, sizeof(AdnRegexCapture));
	if (!clone)
	{
		return NULL;
	}
	if (captures && count > 0)
	{
		memcpy(clone, captures, sizeof(AdnRegexCapture) * count);
	}
	return clone;
}

static int adn_regex_match_node(const AdnRegexNode* node, const AdnRegexText* text,
	                            size_t position, AdnRegexCapture* captures,
	                            size_t capture_count, size_t* out_position);

static int adn_regex_match_sequence_items(const AdnRegexNode* const* items,
	                                      size_t count, size_t index,
	                                      const AdnRegexText* text,
	                                      size_t position,
	                                      AdnRegexCapture* captures,
	                                      size_t capture_count,
	                                      size_t* out_position);

static int adn_regex_match_class(const AdnRegexCharClass* char_class,
	                             uint32_t codepoint)
{
	int matched = 0;
	size_t i;

	if (!char_class)
	{
		return 0;
	}

	for (i = 0; i < char_class->range_count; i++)
	{
		if (codepoint >= char_class->ranges[i].start &&
		    codepoint <= char_class->ranges[i].end)
		{
			matched = 1;
			break;
		}
	}

	if (!matched && (char_class->builtins & ADN_REGEX_CLASS_DIGIT) != 0)
	{
		matched = adn_regex_is_digit(codepoint);
	}
	if (!matched && (char_class->builtins & ADN_REGEX_CLASS_SPACE) != 0)
	{
		matched = adn_regex_is_space(codepoint);
	}
	if (!matched && (char_class->builtins & ADN_REGEX_CLASS_WORD) != 0)
	{
		matched = adn_regex_is_word(codepoint);
	}
	if (!matched && (char_class->builtins & ADN_REGEX_CLASS_ALPHA) != 0)
	{
		matched = adn_regex_is_alpha(codepoint);
	}
	if (!matched && (char_class->builtins & ADN_REGEX_CLASS_ALNUM) != 0)
	{
		matched = adn_regex_is_alnum(codepoint);
	}
	if (!matched && (char_class->builtins & ADN_REGEX_CLASS_LOWER) != 0)
	{
		matched = adn_regex_is_lower(codepoint);
	}
	if (!matched && (char_class->builtins & ADN_REGEX_CLASS_UPPER) != 0)
	{
		matched = adn_regex_is_upper(codepoint);
	}
	if (!matched && (char_class->builtins & ADN_REGEX_CLASS_XDIGIT) != 0)
	{
		matched = adn_regex_is_xdigit(codepoint);
	}

	return char_class->negated ? !matched : matched;
}

static int adn_regex_is_word_boundary(const AdnRegexText* text, size_t position)
{
	int left = 0;
	int right = 0;

	if (position > 0 && position - 1 < text->length)
	{
		left = adn_regex_is_word(text->codepoints[position - 1]);
	}
	if (position < text->length)
	{
		right = adn_regex_is_word(text->codepoints[position]);
	}
	return left != right;
}

static int adn_regex_match_repeat(const AdnRegexNode* repeat,
	                             const AdnRegexNode* const* items, size_t count,
	                             size_t next_index, const AdnRegexText* text,
	                             size_t position, AdnRegexCapture* captures,
	                             size_t capture_count, size_t* out_position,
	                             int repeats)
{
	int min = repeat->as.repeat.min;
	int max = repeat->as.repeat.max;
	int greedy = repeat->as.repeat.greedy;

	if (!greedy && repeats >= min)
	{
		AdnRegexCapture* snapshot = adn_regex_captures_clone(captures, capture_count);
		if (!snapshot)
		{
			return 0;
		}
		if (adn_regex_match_sequence_items(items, count, next_index, text, position,
		                                  snapshot, capture_count, out_position))
		{
			memcpy(captures, snapshot, sizeof(AdnRegexCapture) * capture_count);
			free(snapshot);
			return 1;
		}
		free(snapshot);
	}

	if (max < 0 || repeats < max)
	{
		AdnRegexCapture* snapshot = adn_regex_captures_clone(captures, capture_count);
		size_t next_position = position;
		if (!snapshot)
		{
			return 0;
		}
		if (adn_regex_match_node(repeat->as.repeat.child, text, position, snapshot,
		                        capture_count, &next_position))
		{
			if (next_position != position)
			{
				if (adn_regex_match_repeat(repeat, items, count, next_index, text,
				                           next_position, snapshot, capture_count,
				                           out_position, repeats + 1))
				{
					memcpy(captures, snapshot,
					       sizeof(AdnRegexCapture) * capture_count);
					free(snapshot);
					return 1;
				}
			}
			else if (repeats + 1 >= min &&
			         adn_regex_match_sequence_items(items, count, next_index, text,
			                                         next_position, snapshot,
			                                         capture_count, out_position))
			{
				memcpy(captures, snapshot, sizeof(AdnRegexCapture) * capture_count);
				free(snapshot);
				return 1;
			}
		}
		free(snapshot);
	}

	if (greedy && repeats >= min)
	{
		AdnRegexCapture* snapshot = adn_regex_captures_clone(captures, capture_count);
		if (!snapshot)
		{
			return 0;
		}
		if (adn_regex_match_sequence_items(items, count, next_index, text, position,
		                                  snapshot, capture_count, out_position))
		{
			memcpy(captures, snapshot, sizeof(AdnRegexCapture) * capture_count);
			free(snapshot);
			return 1;
		}
		free(snapshot);
	}

	return 0;
}

static int adn_regex_match_sequence_items(const AdnRegexNode* const* items,
	                                      size_t count, size_t index,
	                                      const AdnRegexText* text,
	                                      size_t position,
	                                      AdnRegexCapture* captures,
	                                      size_t capture_count,
	                                      size_t* out_position)
{
	AdnRegexCapture* snapshot;
	size_t next_position = position;

	if (index >= count)
	{
		*out_position = position;
		return 1;
	}

	if (items[index]->kind == ADN_REGEX_NODE_REPEAT)
	{
		return adn_regex_match_repeat(items[index], items, count, index + 1, text,
		                              position, captures, capture_count,
		                              out_position, 0);
	}

	snapshot = adn_regex_captures_clone(captures, capture_count);
	if (!snapshot)
	{
		return 0;
	}
	if (adn_regex_match_node(items[index], text, position, snapshot, capture_count,
	                        &next_position) &&
	    adn_regex_match_sequence_items(items, count, index + 1, text, next_position,
	                                  snapshot, capture_count, out_position))
	{
		memcpy(captures, snapshot, sizeof(AdnRegexCapture) * capture_count);
		free(snapshot);
		return 1;
	}
	free(snapshot);
	return 0;
}

static int adn_regex_match_lookbehind(const AdnRegexNode* node,
	                                  const AdnRegexText* text, size_t position,
	                                  AdnRegexCapture* captures,
	                                  size_t capture_count)
{
	for (size_t start = 0; start <= position; start++)
	{
		AdnRegexCapture* snapshot = adn_regex_captures_clone(captures, capture_count);
		size_t end_position = start;
		if (!snapshot)
		{
			return 0;
		}
		if (adn_regex_match_node(node->as.look.child, text, start, snapshot,
		                        capture_count, &end_position) &&
		    end_position == position)
		{
			if (node->as.look.positive)
			{
				memcpy(captures, snapshot, sizeof(AdnRegexCapture) * capture_count);
				free(snapshot);
				return 1;
			}
			free(snapshot);
			return 0;
		}
		free(snapshot);
	}
	return node->as.look.positive ? 0 : 1;
}

static int adn_regex_match_node(const AdnRegexNode* node, const AdnRegexText* text,
	                            size_t position, AdnRegexCapture* captures,
	                            size_t capture_count, size_t* out_position)
{
	if (!node || !text || !out_position)
	{
		return 0;
	}

	switch (node->kind)
	{
		case ADN_REGEX_NODE_EMPTY:
			*out_position = position;
			return 1;
		case ADN_REGEX_NODE_LITERAL:
			if (position < text->length && text->codepoints[position] == node->as.literal)
			{
				*out_position = position + 1;
				return 1;
			}
			return 0;
		case ADN_REGEX_NODE_DOT:
			if (position < text->length &&
			    !adn_regex_is_newline(text->codepoints[position]))
			{
				*out_position = position + 1;
				return 1;
			}
			return 0;
		case ADN_REGEX_NODE_CLASS:
			if (position < text->length &&
			    adn_regex_match_class(node->as.char_class, text->codepoints[position]))
			{
				*out_position = position + 1;
				return 1;
			}
			return 0;
		case ADN_REGEX_NODE_SEQUENCE:
			return adn_regex_match_sequence_items((const AdnRegexNode* const*)node->as.list.items,
			                                      node->as.list.count, 0, text,
			                                      position, captures,
			                                      capture_count, out_position);
		case ADN_REGEX_NODE_ALTERNATION:
			for (size_t i = 0; i < node->as.list.count; i++)
			{
				AdnRegexCapture* snapshot = adn_regex_captures_clone(captures,
				                                                   capture_count);
				size_t end_position = position;
				if (!snapshot)
				{
					return 0;
				}
				if (adn_regex_match_node(node->as.list.items[i], text, position,
				                        snapshot, capture_count, &end_position))
				{
					memcpy(captures, snapshot,
					       sizeof(AdnRegexCapture) * capture_count);
					*out_position = end_position;
					free(snapshot);
					return 1;
				}
				free(snapshot);
			}
			return 0;
		case ADN_REGEX_NODE_REPEAT:
			return adn_regex_match_repeat(node, &node, 1, 1, text, position,
			                              captures, capture_count, out_position, 0);
		case ADN_REGEX_NODE_GROUP:
		{
			AdnRegexCapture* snapshot = adn_regex_captures_clone(captures, capture_count);
			size_t end_position = position;
			if (!snapshot)
			{
				return 0;
			}
			if (adn_regex_match_node(node->as.group.child, text, position, snapshot,
			                        capture_count, &end_position))
			{
				if (node->as.group.capturing &&
				    node->as.group.index >= 0 &&
				    (size_t)node->as.group.index < capture_count)
				{
					snapshot[node->as.group.index].start = position;
					snapshot[node->as.group.index].end = end_position;
					snapshot[node->as.group.index].defined = 1;
				}
				memcpy(captures, snapshot, sizeof(AdnRegexCapture) * capture_count);
				*out_position = end_position;
				free(snapshot);
				return 1;
			}
			free(snapshot);
			return 0;
		}
		case ADN_REGEX_NODE_BACKREF:
		{
			int index = node->as.backref_index;
			if (index <= 0 || (size_t)index >= capture_count ||
			    !captures[index].defined)
			{
				return 0;
			}
			size_t length = captures[index].end - captures[index].start;
			if (position + length > text->length)
			{
				return 0;
			}
			for (size_t i = 0; i < length; i++)
			{
				if (text->codepoints[captures[index].start + i] !=
				    text->codepoints[position + i])
				{
					return 0;
				}
			}
			*out_position = position + length;
			return 1;
		}
		case ADN_REGEX_NODE_ASSERT_START:
			if (position == 0)
			{
				*out_position = position;
				return 1;
			}
			return 0;
		case ADN_REGEX_NODE_ASSERT_END:
			if (position == text->length)
			{
				*out_position = position;
				return 1;
			}
			return 0;
		case ADN_REGEX_NODE_WORD_BOUNDARY:
			if (adn_regex_is_word_boundary(text, position))
			{
				*out_position = position;
				return 1;
			}
			return 0;
		case ADN_REGEX_NODE_NOT_WORD_BOUNDARY:
			if (!adn_regex_is_word_boundary(text, position))
			{
				*out_position = position;
				return 1;
			}
			return 0;
		case ADN_REGEX_NODE_LOOK:
			if (node->as.look.behind)
			{
				if (adn_regex_match_lookbehind(node, text, position, captures,
				                              capture_count))
				{
					*out_position = position;
					return 1;
				}
				return 0;
			}
			else
			{
				AdnRegexCapture* snapshot = adn_regex_captures_clone(captures,
				                                                   capture_count);
				size_t end_position = position;
				int matched;
				if (!snapshot)
				{
					return 0;
				}
				matched = adn_regex_match_node(node->as.look.child, text, position,
				                               snapshot, capture_count,
				                               &end_position);
				if (matched == node->as.look.positive)
				{
					if (matched)
					{
						memcpy(captures, snapshot,
						       sizeof(AdnRegexCapture) * capture_count);
					}
					*out_position = position;
					free(snapshot);
					return 1;
				}
				free(snapshot);
				return 0;
			}
	}

	return 0;
}

static int adn_regex_find_match(const AdnRegexProgram* program,
	                           const AdnRegexText* text, size_t start,
	                           AdnRegexMatch* match)
{
	size_t capture_count;

	if (!program || !program->root || !text || !match)
	{
		return 0;
	}

	memset(match, 0, sizeof(*match));
	capture_count = (size_t)program->capture_count + 1;
	for (size_t position = start; position <= text->length; position++)
	{
		AdnRegexCapture* captures = (AdnRegexCapture*)calloc(capture_count,
		                                                  sizeof(AdnRegexCapture));
		size_t end_position = position;
		if (!captures)
		{
			return 0;
		}
		if (adn_regex_match_node(program->root, text, position, captures,
		                        capture_count, &end_position))
		{
			captures[0].start = position;
			captures[0].end = end_position;
			captures[0].defined = 1;
			match->start = position;
			match->end = end_position;
			match->captures = captures;
			return 1;
		}
		free(captures);
	}

	return 0;
}

static int adn_regex_match_full(const AdnRegexProgram* program,
	                           const AdnRegexText* text)
{
	AdnRegexMatch match = {0};
	if (!adn_regex_find_match(program, text, 0, &match))
	{
		return 0;
	}
	if (match.start == 0 && match.end == text->length)
	{
		free(match.captures);
		return 1;
	}
	free(match.captures);
	return 0;
}

static int adn_regex_match_from_start(const AdnRegexProgram* program,
	                                 const AdnRegexText* text)
{
	size_t capture_count = (size_t)program->capture_count + 1;
	AdnRegexCapture* captures = (AdnRegexCapture*)calloc(capture_count,
	                                                  sizeof(AdnRegexCapture));
	size_t end_position = 0;
	int matched;
	if (!captures)
	{
		return 0;
	}
	matched = adn_regex_match_node(program->root, text, 0, captures, capture_count,
	                              &end_position);
	free(captures);
	return matched;
}

static int adn_regex_match_to_end(const AdnRegexProgram* program,
	                             const AdnRegexText* text)
{
	size_t capture_count = (size_t)program->capture_count + 1;
	for (size_t position = 0; position <= text->length; position++)
	{
		AdnRegexCapture* captures = (AdnRegexCapture*)calloc(capture_count,
		                                                  sizeof(AdnRegexCapture));
		size_t end_position = position;
		if (!captures)
		{
			return 0;
		}
		if (adn_regex_match_node(program->root, text, position, captures,
		                        capture_count, &end_position) &&
		    end_position == text->length)
		{
			free(captures);
			return 1;
		}
		free(captures);
	}
	return 0;
}

static void adn_regex_match_free(AdnRegexMatch* match)
{
	if (!match)
	{
		return;
	}
	free(match->captures);
	match->captures = NULL;
	match->start = 0;
	match->end = 0;
}

static void adn_regex_builder_append_slice(AdnRegexStringBuilder* builder,
	                                       const char* text,
	                                       const AdnRegexText* view,
	                                       size_t start, size_t end)
{
	if (!builder || !text || !view || start > end || end > view->length)
	{
		return;
	}
	adn_regex_builder_append_n(builder, text + view->offsets[start],
	                         view->offsets[end] - view->offsets[start]);
}

static char* adn_regex_expand_replacement(const char* replacement,
	                                     const char* text,
	                                     const AdnRegexText* view,
	                                     const AdnRegexMatch* match,
	                                     size_t capture_count)
{
	AdnRegexStringBuilder builder = {0};
	size_t length = replacement ? strlen(replacement) : 0;
	size_t index = 0;
	adn_regex_builder_init(&builder);

	while (index < length)
	{
		char ch = replacement[index];
		if (ch == '$' && index + 1 < length)
		{
			char next = replacement[index + 1];
			if (next == '$')
			{
				adn_regex_builder_append_char(&builder, '$');
				index += 2;
				continue;
			}
			if (next == '&' && match && match->captures)
			{
				adn_regex_builder_append_slice(&builder, text, view,
				                             match->captures[0].start,
				                             match->captures[0].end);
				index += 2;
				continue;
			}
			if (next >= '0' && next <= '9')
			{
				int value = 0;
				index++;
				while (index < length && replacement[index] >= '0' &&
				       replacement[index] <= '9')
				{
					value = value * 10 + (replacement[index] - '0');
					index++;
				}
				if (match && match->captures && value >= 0 &&
				    (size_t)value < capture_count &&
				    match->captures[value].defined)
				{
					adn_regex_builder_append_slice(&builder, text, view,
					                             match->captures[value].start,
					                             match->captures[value].end);
				}
				continue;
			}
		}
		if (ch == '\\' && index + 1 < length && replacement[index + 1] >= '0' &&
		    replacement[index + 1] <= '9')
		{
			int value = 0;
			index++;
			while (index < length && replacement[index] >= '0' &&
			       replacement[index] <= '9')
			{
				value = value * 10 + (replacement[index] - '0');
				index++;
			}
			if (match && match->captures && value >= 0 && (size_t)value < capture_count &&
			    match->captures[value].defined)
			{
				adn_regex_builder_append_slice(&builder, text, view,
				                             match->captures[value].start,
				                             match->captures[value].end);
			}
			continue;
		}
		adn_regex_builder_append_char(&builder, ch);
		index++;
	}

	return adn_regex_builder_finish(&builder);
}

static int adn_regex_compile_sources(const char* pattern, const char* text,
	                                char** out_pattern,
	                                char** out_text,
	                                AdnRegexProgram* out_program,
	                                AdnRegexText* out_view)
{
	char* raw_pattern = adn_regex_unwrap_string(pattern);
	char* raw_text = adn_regex_unwrap_string(text);
	if (!raw_pattern || !raw_text)
	{
		free(raw_pattern);
		free(raw_text);
		return 0;
	}
	if (!adn_regex_program_init(out_program, raw_pattern))
	{
		free(raw_pattern);
		free(raw_text);
		return 0;
	}
	if (!adn_regex_text_init(out_view, raw_text))
	{
		adn_regex_program_free(out_program);
		free(raw_pattern);
		free(raw_text);
		return 0;
	}
	*out_pattern = raw_pattern;
	*out_text = raw_text;
	return 1;
}

static int adn_regex_compile_pattern(const char* pattern, char** out_pattern,
	                                AdnRegexProgram* out_program)
{
	char* raw_pattern = adn_regex_unwrap_string(pattern);
	if (!raw_pattern)
	{
		return 0;
	}
	if (!adn_regex_program_init(out_program, raw_pattern))
	{
		free(raw_pattern);
		return 0;
	}
	*out_pattern = raw_pattern;
	return 1;
}

int64_t adn_regex_valid(const char* pattern)
{
	AdnRegexProgram program = {0};
	char* raw_pattern = NULL;
	int ok = adn_regex_compile_pattern(pattern, &raw_pattern, &program);
	free(raw_pattern);
	adn_regex_program_free(&program);
	return ok ? 1 : 0;
}

char* adn_regex_escape(const char* text)
{
	static const char* metacharacters = "\\.^$|?*+()[]{}";
	char* raw_text = adn_regex_unwrap_string(text);
	AdnRegexStringBuilder builder = {0};
	if (!raw_text)
	{
		return strdup("");
	}
	adn_regex_builder_init(&builder);
	for (size_t i = 0; raw_text[i] != '\0'; i++)
	{
		if (strchr(metacharacters, raw_text[i]) != NULL)
		{
			adn_regex_builder_append_char(&builder, '\\');
		}
		adn_regex_builder_append_char(&builder, raw_text[i]);
	}
	free(raw_text);
	return adn_regex_builder_finish(&builder);
}

char* adn_regex_compile(const char* pattern)
{
	AdnRegexProgram program = {0};
	char* raw_pattern = NULL;
	if (!adn_regex_compile_pattern(pattern, &raw_pattern, &program))
	{
		return strdup("");
	}
	adn_regex_program_free(&program);
	return raw_pattern;
}

int64_t adn_regex_matches(const char* pattern, const char* text)
{
	AdnRegexProgram program = {0};
	AdnRegexText view = {0};
	char* raw_pattern = NULL;
	char* raw_text = NULL;
	int matched;
	if (!adn_regex_compile_sources(pattern, text, &raw_pattern, &raw_text, &program,
	                             &view))
	{
		return 0;
	}
	matched = adn_regex_match_full(&program, &view);
	adn_regex_text_free(&view);
	adn_regex_program_free(&program);
	free(raw_pattern);
	free(raw_text);
	return matched ? 1 : 0;
}

int64_t adn_regex_find(const char* pattern, const char* text)
{
	AdnRegexProgram program = {0};
	AdnRegexText view = {0};
	AdnRegexMatch match = {0};
	char* raw_pattern = NULL;
	char* raw_text = NULL;
	int64_t result = -1;
	if (!adn_regex_compile_sources(pattern, text, &raw_pattern, &raw_text, &program,
	                             &view))
	{
		return -1;
	}
	if (adn_regex_find_match(&program, &view, 0, &match))
	{
		result = (int64_t)view.offsets[match.start];
	}
	adn_regex_match_free(&match);
	adn_regex_text_free(&view);
	adn_regex_program_free(&program);
	free(raw_pattern);
	free(raw_text);
	return result;
}

int64_t adn_regex_contains(const char* pattern, const char* text)
{
	return adn_regex_find(pattern, text) >= 0 ? 1 : 0;
}

int64_t adn_regex_starts_with(const char* pattern, const char* text)
{
	AdnRegexProgram program = {0};
	AdnRegexText view = {0};
	char* raw_pattern = NULL;
	char* raw_text = NULL;
	int matched;
	if (!adn_regex_compile_sources(pattern, text, &raw_pattern, &raw_text, &program,
	                             &view))
	{
		return 0;
	}
	matched = adn_regex_match_from_start(&program, &view);
	adn_regex_text_free(&view);
	adn_regex_program_free(&program);
	free(raw_pattern);
	free(raw_text);
	return matched ? 1 : 0;
}

int64_t adn_regex_ends_with(const char* pattern, const char* text)
{
	AdnRegexProgram program = {0};
	AdnRegexText view = {0};
	char* raw_pattern = NULL;
	char* raw_text = NULL;
	int matched;
	if (!adn_regex_compile_sources(pattern, text, &raw_pattern, &raw_text, &program,
	                             &view))
	{
		return 0;
	}
	matched = adn_regex_match_to_end(&program, &view);
	adn_regex_text_free(&view);
	adn_regex_program_free(&program);
	free(raw_pattern);
	free(raw_text);
	return matched ? 1 : 0;
}

char* adn_regex_replace(const char* pattern, const char* text, const char* replacement)
{
	AdnRegexProgram program = {0};
	AdnRegexText view = {0};
	AdnRegexMatch match = {0};
	AdnRegexStringBuilder builder = {0};
	char* raw_pattern = NULL;
	char* raw_text = NULL;
	char* raw_replacement = adn_regex_unwrap_string(replacement);
	char* expanded;
	char* result;
	size_t capture_count;

	if (!raw_replacement)
	{
		return strdup("");
	}
	if (!adn_regex_compile_sources(pattern, text, &raw_pattern, &raw_text, &program,
	                             &view))
	{
		free(raw_replacement);
		return adn_regex_unwrap_string(text);
	}
	if (!adn_regex_find_match(&program, &view, 0, &match))
	{
		adn_regex_text_free(&view);
		adn_regex_program_free(&program);
		free(raw_pattern);
		free(raw_replacement);
		return raw_text;
	}

	adn_regex_builder_init(&builder);
	adn_regex_builder_append_slice(&builder, raw_text, &view, 0, match.start);
	capture_count = (size_t)program.capture_count + 1;
	expanded = adn_regex_expand_replacement(raw_replacement, raw_text, &view, &match,
	                                     capture_count);
	adn_regex_builder_append(&builder, expanded ? expanded : "");
	adn_regex_builder_append_slice(&builder, raw_text, &view, match.end, view.length);

	free(expanded);
	free(raw_replacement);
	adn_regex_match_free(&match);
	adn_regex_text_free(&view);
	adn_regex_program_free(&program);
	free(raw_pattern);
	free(raw_text);
	result = adn_regex_builder_finish(&builder);
	return result;
}

char* adn_regex_replace_all(const char* pattern, const char* text,
	                        const char* replacement)
{
	AdnRegexProgram program = {0};
	AdnRegexText view = {0};
	AdnRegexStringBuilder builder = {0};
	char* raw_pattern = NULL;
	char* raw_text = NULL;
	char* raw_replacement = adn_regex_unwrap_string(replacement);
	size_t search_start = 0;
	size_t emit_start = 0;
	size_t capture_count;

	if (!raw_replacement)
	{
		return strdup("");
	}
	if (!adn_regex_compile_sources(pattern, text, &raw_pattern, &raw_text, &program,
	                             &view))
	{
		free(raw_replacement);
		return adn_regex_unwrap_string(text);
	}

	capture_count = (size_t)program.capture_count + 1;
	adn_regex_builder_init(&builder);

	while (search_start <= view.length)
	{
		AdnRegexMatch match = {0};
		char* expanded;
		if (!adn_regex_find_match(&program, &view, search_start, &match))
		{
			break;
		}
		adn_regex_builder_append_slice(&builder, raw_text, &view, emit_start,
		                             match.start);
		expanded = adn_regex_expand_replacement(raw_replacement, raw_text, &view,
		                                     &match, capture_count);
		adn_regex_builder_append(&builder, expanded ? expanded : "");
		free(expanded);

		if (match.end == match.start)
		{
			if (match.start < view.length)
			{
				adn_regex_builder_append_slice(&builder, raw_text, &view, match.start,
				                             match.start + 1);
				emit_start = match.start + 1;
				search_start = match.start + 1;
			}
			else
			{
				emit_start = match.end;
				search_start = view.length + 1;
			}
		}
		else
		{
			emit_start = match.end;
			search_start = match.end;
		}
		adn_regex_match_free(&match);
	}

	if (emit_start <= view.length)
	{
		adn_regex_builder_append_slice(&builder, raw_text, &view, emit_start,
		                             view.length);
	}

	free(raw_replacement);
	adn_regex_text_free(&view);
	adn_regex_program_free(&program);
	free(raw_pattern);
	free(raw_text);
	return adn_regex_builder_finish(&builder);
}

void* adn_regex_split(const char* pattern, const char* text)
{
	AdnRegexProgram program = {0};
	AdnRegexText view = {0};
	char* raw_pattern = NULL;
	char* raw_text = NULL;
	void* result = adn_array_create();
	size_t search_start = 0;
	size_t emit_start = 0;

	if (!result)
	{
		return NULL;
	}
	if (!adn_regex_compile_sources(pattern, text, &raw_pattern, &raw_text, &program,
	                             &view))
	{
		char* copy = adn_regex_unwrap_string(text);
		adn_array_push_string(result, copy ? copy : "");
		free(copy);
		return result;
	}

	while (search_start <= view.length)
	{
		AdnRegexMatch match = {0};
		char* piece;
		if (!adn_regex_find_match(&program, &view, search_start, &match))
		{
			break;
		}
		piece = (char*)malloc(view.offsets[match.start] - view.offsets[emit_start] + 1);
		if (piece)
		{
			size_t length = view.offsets[match.start] - view.offsets[emit_start];
			memcpy(piece, raw_text + view.offsets[emit_start], length);
			piece[length] = '\0';
			adn_array_push_string(result, piece);
			free(piece);
		}

		if (match.end == match.start)
		{
			emit_start = match.start;
			search_start = match.start < view.length ? match.start + 1 : view.length + 1;
		}
		else
		{
			emit_start = match.end;
			search_start = match.end;
		}
		adn_regex_match_free(&match);
	}

	{
		char* tail = (char*)malloc(view.offsets[view.length] - view.offsets[emit_start] + 1);
		if (tail)
		{
			size_t length = view.offsets[view.length] - view.offsets[emit_start];
			memcpy(tail, raw_text + view.offsets[emit_start], length);
			tail[length] = '\0';
			adn_array_push_string(result, tail);
			free(tail);
		}
	}

	adn_regex_text_free(&view);
	adn_regex_program_free(&program);
	free(raw_pattern);
	free(raw_text);
	return result;
}