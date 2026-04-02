#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

static char* adn_crypto_strdup_empty(void)
{
	char* result = (char*)malloc(1);
	if (result)
	{
		result[0] = '\0';
	}
	return result;
}

static char* adn_crypto_hex_encode(const unsigned char* data, size_t length)
{
	static const char hex[] = "0123456789abcdef";
	char* output;

	if (!data || length == 0)
	{
		return adn_crypto_strdup_empty();
	}

	output = (char*)malloc(length * 2 + 1);
	if (!output)
	{
		return NULL;
	}

	for (size_t i = 0; i < length; i++)
	{
		output[i * 2] = hex[(data[i] >> 4) & 0x0F];
		output[i * 2 + 1] = hex[data[i] & 0x0F];
	}
	output[length * 2] = '\0';
	return output;
}

static char* adn_crypto_digest_hex(const char* text, const EVP_MD* md)
{
	unsigned char digest[EVP_MAX_MD_SIZE];
	unsigned int digest_length = 0;
	EVP_MD_CTX* ctx;
	const unsigned char* bytes = (const unsigned char*)(text ? text : "");
	size_t length = text ? strlen(text) : 0;
	char* result;

	if (!md)
	{
		return adn_crypto_strdup_empty();
	}

	ctx = EVP_MD_CTX_new();
	if (!ctx)
	{
		return adn_crypto_strdup_empty();
	}

	if (EVP_DigestInit_ex(ctx, md, NULL) != 1 ||
	    EVP_DigestUpdate(ctx, bytes, length) != 1 ||
	    EVP_DigestFinal_ex(ctx, digest, &digest_length) != 1)
	{
		EVP_MD_CTX_free(ctx);
		return adn_crypto_strdup_empty();
	}

	EVP_MD_CTX_free(ctx);
	result = adn_crypto_hex_encode(digest, digest_length);
	return result ? result : adn_crypto_strdup_empty();
}

char* __crypto_sha256_hex(const char* value)
{
	return adn_crypto_digest_hex(value, EVP_sha256());
}

char* __crypto_sha512_hex(const char* value)
{
	return adn_crypto_digest_hex(value, EVP_sha512());
}

char* __crypto_random_hex(int32_t byte_count)
{
	unsigned char* buffer;
	char* result;

	if (byte_count <= 0)
	{
		return adn_crypto_strdup_empty();
	}

	buffer = (unsigned char*)malloc((size_t)byte_count);
	if (!buffer)
	{
		return adn_crypto_strdup_empty();
	}

	if (RAND_bytes(buffer, byte_count) != 1)
	{
		free(buffer);
		return adn_crypto_strdup_empty();
	}

	result = adn_crypto_hex_encode(buffer, (size_t)byte_count);
	free(buffer);
	return result ? result : adn_crypto_strdup_empty();
}

int32_t __crypto_secure_equals(const char* left, const char* right)
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

	left_length = strlen(left);
	right_length = strlen(right);
	if (left_length != right_length)
	{
		return 0;
	}

	return CRYPTO_memcmp(left, right, left_length) == 0 ? 1 : 0;
}