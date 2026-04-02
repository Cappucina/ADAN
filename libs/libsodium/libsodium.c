#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int sodium_init(void);

int sodium_memcmp(const void* left, const void* right, size_t length);

void randombytes_buf(void* buffer, size_t size);

int crypto_hash_sha256(unsigned char* out, const unsigned char* in, unsigned long long inlen);

int crypto_hash_sha512(unsigned char* out, const unsigned char* in, unsigned long long inlen);

#define ADN_LIBSODIUM_SHA256_BYTES 32
#define ADN_LIBSODIUM_SHA512_BYTES 64

static char* adn_libsodium_strdup_empty(void)
{
	char* result = (char*)malloc(1);
	if (result)
	{
		result[0] = '\0';
	}
	return result;
}

static int adn_libsodium_ready(void)
{
	static int initialized = 0;
	static int init_result = -1;

	if (!initialized)
	{
		init_result = sodium_init();
		initialized = 1;
	}

	return init_result >= 0;
}

static char* adn_libsodium_hex_encode(const unsigned char* data, size_t length)
{
	static const char hex[] = "0123456789abcdef";
	char* output;

	if (!data || length == 0)
	{
		return adn_libsodium_strdup_empty();
	}

	output = (char*)malloc(length * 2 + 1);
	if (!output)
	{
		return NULL;
	}

	for (size_t i = 0; i < length; ++i)
	{
		output[i * 2] = hex[(data[i] >> 4) & 0x0F];
		output[i * 2 + 1] = hex[data[i] & 0x0F];
	}
	output[length * 2] = '\0';
	return output;
}

static char* adn_libsodium_digest_hex(const char* text,
	                                 int (*digest_fn)(unsigned char*, const unsigned char*, unsigned long long),
	                                 size_t digest_size)
{
	unsigned char digest[ADN_LIBSODIUM_SHA512_BYTES];
	const unsigned char* bytes = (const unsigned char*)(text ? text : "");
	unsigned long long length = (unsigned long long)(text ? strlen(text) : 0);
	char* result;

	if (!digest_fn || digest_size > sizeof(digest) || !adn_libsodium_ready())
	{
		return adn_libsodium_strdup_empty();
	}

	if (digest_fn(digest, bytes, length) != 0)
	{
		return adn_libsodium_strdup_empty();
	}

	result = adn_libsodium_hex_encode(digest, digest_size);
	return result ? result : adn_libsodium_strdup_empty();
}

char* __libsodium_sha256_hex(const char* value)
{
	return adn_libsodium_digest_hex(value, crypto_hash_sha256,
	                               ADN_LIBSODIUM_SHA256_BYTES);
}

char* __libsodium_sha512_hex(const char* value)
{
	return adn_libsodium_digest_hex(value, crypto_hash_sha512,
	                               ADN_LIBSODIUM_SHA512_BYTES);
}

char* __libsodium_random_hex(int32_t byte_count)
{
	unsigned char* buffer;
	char* result;

	if (byte_count <= 0 || !adn_libsodium_ready())
	{
		return adn_libsodium_strdup_empty();
	}

	buffer = (unsigned char*)malloc((size_t)byte_count);
	if (!buffer)
	{
		return adn_libsodium_strdup_empty();
	}

	randombytes_buf(buffer, (size_t)byte_count);
	result = adn_libsodium_hex_encode(buffer, (size_t)byte_count);
	free(buffer);
	return result ? result : adn_libsodium_strdup_empty();
}

int32_t __libsodium_secure_equals(const char* left, const char* right)
{
	size_t left_length;
	size_t right_length;

	if (!left)
	{
		left = "";
	}
	if (!right)
	{
		right = "";
	}
	if (!adn_libsodium_ready())
	{
		return 0;
	}

	left_length = strlen(left);
	right_length = strlen(right);
	if (left_length != right_length)
	{
		return 0;
	}

	return sodium_memcmp(left, right, left_length) == 0 ? 1 : 0;
}