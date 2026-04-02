#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <limits.h>

#include "helper.h"
#include "stm.h"
#include "frontend/scanner/scanner.h"
#include "frontend/parser/parser.h"
#include "frontend/ast/tree.h"
#include "frontend/semantics/semantic.h"
#include "backend/backend.h"
#include "backend/linker/linker.h"
#include "embedded_libs.h"

#define MAX_BUNDLE_PATH_SIZE 2048

static char* trim_whitespace_inplace(char* text)
{
	char* end;

	if (!text)
	{
		return text;
	}

	while (*text == ' ' || *text == '\t' || *text == '\n' || *text == '\r')
	{
		text++;
	}

	end = text + strlen(text);
	while (end > text && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\n' ||
	                     end[-1] == '\r'))
	{
		end--;
	}
	*end = '\0';
	return text;
}

static bool csv_contains_value(const char* csv, const char* value)
{
	const char* cursor;
	size_t value_len;

	if (!csv || !value || value[0] == '\0')
	{
		return false;
	}

	value_len = strlen(value);
	cursor = csv;
	while (*cursor)
	{
		const char* end = strchr(cursor, ',');
		size_t token_len = end ? (size_t)(end - cursor) : strlen(cursor);
		if (token_len == value_len && strncmp(cursor, value, value_len) == 0)
		{
			return true;
		}
		if (!end)
		{
			break;
		}
		cursor = end + 1;
	}

	return false;
}

static bool append_csv_value(char** csv, const char* value)
{
	char* resized;
	size_t current_len;
	size_t value_len;

	if (!csv || !value || value[0] == '\0')
	{
		return true;
	}

	if (*csv && csv_contains_value(*csv, value))
	{
		return true;
	}

	current_len = *csv ? strlen(*csv) : 0;
	value_len = strlen(value);
	resized = realloc(*csv, current_len + value_len + (current_len ? 2 : 1));
	if (!resized)
	{
		return false;
	}

	*csv = resized;
	if (current_len)
	{
		(*csv)[current_len++] = ',';
	}
	memcpy(*csv + current_len, value, value_len + 1);
	return true;
}

static bool append_csv_values(char** csv, const char* values)
{
	char* copy;
	char* token;

	if (!values || values[0] == '\0')
	{
		return true;
	}

	copy = clone_string(values, strlen(values));
	if (!copy)
	{
		return false;
	}

	token = strtok(copy, ",");
	while (token)
	{
		char* trimmed = trim_whitespace_inplace(token);
		if (trimmed[0] != '\0' && !append_csv_value(csv, trimmed))
		{
			free(copy);
			return false;
		}
		token = strtok(NULL, ",");
	}

	free(copy);
	return true;
}

static bool append_space_value(char** text, const char* value)
{
	char* resized;
	size_t current_len;
	size_t value_len;

	if (!text || !value || value[0] == '\0')
	{
		return true;
	}

	current_len = *text ? strlen(*text) : 0;
	value_len = strlen(value);
	resized = realloc(*text, current_len + value_len + (current_len ? 2 : 1));
	if (!resized)
	{
		return false;
	}

	*text = resized;
	if (current_len)
	{
		(*text)[current_len++] = ' ';
	}
	memcpy(*text + current_len, value, value_len + 1);
	return true;
}

static bool build_combined_bundle_paths(const char* bundle_paths, char* output,
	                                    size_t output_size)
{
	int written;

	if (!output || output_size == 0)
	{
		return false;
	}

	if (bundle_paths && bundle_paths[0] != '\0')
	{
		written = snprintf(output, output_size, "src/backend/runtime,%s",
		                   bundle_paths);
	}
	else
	{
		written = snprintf(output, output_size, "src/backend/runtime");
	}

	return written >= 0 && (size_t)written < output_size;
}

bool has_valid_extension(const char* filename)
{
	size_t len = strlen(filename);
	if (len < 5)
	{
		return false;
	}

	if (strcmp(filename + len - 4, ".adn") == 0)
	{
		return true;
	}
	if (len >= 6 && strcmp(filename + len - 5, ".adan") == 0)
	{
		return true;
	}

	return false;
}

void print_usage(const char* program_name)
{
	printf("Usage: %s -f <file.adn> [options]\n", program_name);
	printf("File must have .adn or .adan extension\n");
}

void print_help(const char* program_name)
{
	printf("Usage: %s -f <file.adn> [options]\n\n", program_name);
	printf("Options:\n");
	printf("  -f, --file <file>          Source file to compile (.adn or .adan)\n");
	printf("  -o, --output <path>        Output path for the linked binary.\n");
	printf("                             If <path> is a directory, the binary is placed\n");
	printf("                             inside it named after the source file.\n");
	printf("  -l, --link-lib <name>      Link a native library name or path. Repeatable.\n");
	printf("  -L, --link-search <path>   Add a native library search path. Repeatable.\n");
	printf("      --link-arg <arg>       Pass a raw argument through to the final clang link.\n");
	printf("  -r, --rawir                Stop after emitting LLVM IR (.ll file)\n");
	printf("  -h, --help                 Show this help message and exit\n");
}

int main(int argc, char* argv[])
{
	char* file_path = NULL;
	char* out_path = NULL;
	char* cli_link_args = NULL;
	char* cli_link_libraries = NULL;
	char* cli_link_search_paths = NULL;
	bool stop_at_ir = false;

	if (argc < 2)
	{
		print_usage(argv[0]);
		return 1;
	}

	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
		{
			print_help(argv[0]);
			return 0;
		}
		else if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--file") == 0)
		{
			if (i + 1 < argc)
			{
				file_path = argv[i + 1];
				i++;
			}
			else
			{
				fprintf(stderr, "Error: -f/--file requires a filename argument\n");
				print_usage(argv[0]);
				return 1;
			}
		}
		else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0)
		{
			if (i + 1 < argc)
			{
				out_path = argv[i + 1];
				i++;
			}
			else
			{
				fprintf(stderr, "Error: -o/--output requires a path argument\n");
				print_usage(argv[0]);
				return 1;
			}
		}
		else if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--link-lib") == 0)
		{
			if (i + 1 < argc)
			{
				if (!append_csv_value(&cli_link_libraries, argv[i + 1]))
				{
					fprintf(stderr, "Error: failed to store linker library option\n");
					free(cli_link_args);
					free(cli_link_libraries);
					free(cli_link_search_paths);
					return 1;
				}
				i++;
			}
			else
			{
				fprintf(stderr, "Error: -l/--link-lib requires a library argument\n");
				print_usage(argv[0]);
				free(cli_link_args);
				free(cli_link_libraries);
				free(cli_link_search_paths);
				return 1;
			}
		}
		else if (strcmp(argv[i], "-L") == 0 || strcmp(argv[i], "--link-search") == 0)
		{
			if (i + 1 < argc)
			{
				if (!append_csv_value(&cli_link_search_paths, argv[i + 1]))
				{
					fprintf(stderr, "Error: failed to store linker search path\n");
					free(cli_link_args);
					free(cli_link_libraries);
					free(cli_link_search_paths);
					return 1;
				}
				i++;
			}
			else
			{
				fprintf(stderr, "Error: -L/--link-search requires a path argument\n");
				print_usage(argv[0]);
				free(cli_link_args);
				free(cli_link_libraries);
				free(cli_link_search_paths);
				return 1;
			}
		}
		else if (strcmp(argv[i], "--link-arg") == 0)
		{
			if (i + 1 < argc)
			{
				if (!append_space_value(&cli_link_args, argv[i + 1]))
				{
					fprintf(stderr, "Error: failed to store raw linker argument\n");
					free(cli_link_args);
					free(cli_link_libraries);
					free(cli_link_search_paths);
					return 1;
				}
				i++;
			}
			else
			{
				fprintf(stderr, "Error: --link-arg requires an argument\n");
				print_usage(argv[0]);
				free(cli_link_args);
				free(cli_link_libraries);
				free(cli_link_search_paths);
				return 1;
			}
		}
		else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--rawir") == 0)
		{
			stop_at_ir = true;
		}
	}

	if (!file_path)
	{
		fprintf(stderr, "Error: No file specified\n");
		print_usage(argv[0]);
		free(cli_link_args);
		free(cli_link_libraries);
		free(cli_link_search_paths);
		return 1;
	}

	if (!has_valid_extension(file_path))
	{
		fprintf(stderr, "Error: File must have .adn or .adan extension\n");
		free(cli_link_args);
		free(cli_link_libraries);
		free(cli_link_search_paths);
		return 1;
	}

	char* source = read_file(file_path);
	if (!source)
	{
		fprintf(stderr, "Failed to read source file! (Error)\n");
		free(cli_link_args);
		free(cli_link_libraries);
		free(cli_link_search_paths);
		return 2;
	}

	SymbolTableStack* global_stack = sts_init();
	Scanner* scanner = scanner_init(source);
	Parser* parser = parser_init(scanner);
	if (parser)
	{
		parser->allow_undefined_symbols = true;
	}

	ASTNode* ast = parser_parse_program(parser);

	parser_free(parser);
	scanner_free(scanner);

	int exit_code = 0;

	if (ast)
	{
		printf("AST created successfully! (Info)\n");

		SemanticAnalyzer* analyzer = semantic_init(ast, global_stack);
		if (analyzer)
		{
			if (!semantic_analyze(analyzer))
			{
				fprintf(stderr,
				        "Semantic analysis failed with %d error(s). (Error)\n",
				        analyzer->error_count);
				exit_code = 3;
			}
			else
			{
				printf("Semantic analysis completed successfully! (Info)\n");

				char* ll_path = NULL;
				if (file_path)
				{
					size_t flen = strlen(file_path);
					const char* dot = strrchr(file_path, '.');
					size_t base_len = dot ? (size_t)(dot - file_path) : flen;
					ll_path = (char*)malloc(base_len + 4 + 1);
					if (ll_path)
					{
						memcpy(ll_path, file_path, base_len);
						strcpy(ll_path + base_len, ".ll");
					}
				}

				int emit_res = -1;
				if (ll_path)
				{
					emit_res = backend_compile_ast_to_llvm_file(ast, ll_path);
					if (emit_res == 0)
					{
						printf("LLVM IR emitted to %s\n", ll_path);
					}
					else
					{
						fprintf(stderr, "Failed to emit LLVM IR to %s\n",
						        ll_path);
						exit_code = 4;
					}
				}

				if (ll_path && emit_res == 0 && !stop_at_ir)
				{
					char* merged_link_libraries = NULL;
					char* merged_link_search_paths = NULL;
					char default_out[PATH_MAX];
					const char* final_out = out_path;

					if (!final_out)
					{
						size_t flen = strlen(file_path);
						const char* dot = strrchr(file_path, '.');
						size_t base_len =
						    dot ? (size_t)(dot - file_path) : flen;
						memcpy(default_out, file_path, base_len);
						default_out[base_len] = '\0';
						final_out = default_out;
					}

					char resolved_out[PATH_MAX];
					struct stat st;
					bool is_dir = false;
					size_t op_len = strlen(final_out);
					if (final_out[op_len - 1] == '/')
					{
						is_dir = true;
					}
					else if (stat(final_out, &st) == 0 && S_ISDIR(st.st_mode))
					{
						is_dir = true;
					}

					if (is_dir)
					{
						const char* slash = strrchr(file_path, '/');
						const char* base = slash ? slash + 1 : file_path;
						const char* dot = strrchr(base, '.');
						size_t stem_len =
						    dot ? (size_t)(dot - base) : strlen(base);
						size_t dir_len = op_len;
						while (dir_len > 0 && final_out[dir_len - 1] == '/')
						{
							dir_len--;
						}
						snprintf(resolved_out, sizeof(resolved_out),
						         "%.*s/%.*s", (int)dir_len, final_out,
						         (int)stem_len, base);
					}
					else
					{
						snprintf(resolved_out, sizeof(resolved_out), "%s",
						         final_out);
					}

					const char* outp = resolved_out;
					int lres;
					const char* bundle_paths = semantic_get_bundle_paths();
					const char* embedded_modules = semantic_get_embedded_modules();
					const char* native_libraries = semantic_get_native_libraries();
					const char* native_search_paths = semantic_get_native_search_paths();
					char combined_bundle_paths[MAX_BUNDLE_PATH_SIZE];
					bool paths_ok = true;
					if (!append_csv_values(&merged_link_libraries, cli_link_libraries) ||
					    !append_csv_values(&merged_link_libraries, native_libraries) ||
					    !append_csv_values(&merged_link_search_paths,
					                      cli_link_search_paths) ||
					    !append_csv_values(&merged_link_search_paths,
					                      native_search_paths))
					{
						fprintf(stderr,
						        "Error: failed to build native linker configuration.\n");
						exit_code = 5;
						paths_ok = false;
					}
					if (paths_ok &&
					    !build_combined_bundle_paths(bundle_paths,
					                              combined_bundle_paths,
					                              sizeof(combined_bundle_paths)))
					{
						fprintf(stderr,
						        "Error: bundle paths string is too long.\n");
						exit_code = 5;
						paths_ok = false;
					}
					if (paths_ok)
					{
						LinkerConfig link_config = {0};
						link_config.raw_link_args = cli_link_args;
						link_config.bundle_csv = combined_bundle_paths;
						link_config.embedded_modules_csv = embedded_modules;
						link_config.native_libraries_csv = merged_link_libraries;
						link_config.native_search_paths_csv =
						    merged_link_search_paths;

						lres = linker_link(ll_path, outp, &link_config);

						if (lres == 0)
						{
							printf("Linked executable: %s\n", outp);
							remove(ll_path);
						}
						else
						{
							fprintf(stderr,
							        "Linking failed (code=%d)\n", lres);
							exit_code = 5;
						}
					}

					free(merged_link_libraries);
					free(merged_link_search_paths);
				}

				if (ll_path)
				{
					free(ll_path);
				}
			}
			semantic_free(analyzer);
		}
		else
		{
			fprintf(stderr, "Failed to initialize semantic analyzer! (Error)\n");
			exit_code = 3;
		}

		ast_free(ast);
	}
	sts_free(global_stack);
	free(source);
	free(cli_link_args);
	free(cli_link_libraries);
	free(cli_link_search_paths);

	return exit_code;
}