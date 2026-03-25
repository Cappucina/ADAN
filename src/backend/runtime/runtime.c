#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "runtime.h"

// base^exp for integer exponentiation
int64_t adn_powi(int64_t base, int64_t exp)
{
	if (exp < 0)
	{
		// negative exponents not supported for integer power
		return 0;
	}
	if (exp == 0)
	{
		return 1;
	}

	int64_t result = 1;
	int64_t current_base = base;
	int64_t current_exp = exp;

	while (current_exp > 0)
	{
		if (current_exp & 1)
		{
			result *= current_base;
		}
		current_base *= current_base;
		current_exp >>= 1;
	}

	return result;
}

char* adn_strconcat(const char* s1, const char* s2)
{
	if (!s1 || !s2)
	{
		return NULL;
	}

	size_t len1 = strlen(s1);
	size_t len2 = strlen(s2);
	char* result = (char*)malloc(len1 + len2 + 1);

	if (!result)
	{
		return NULL;
	}

	strcpy(result, s1);
	strcat(result, s2);

	return result;
}

char* adn_i32_to_string(int64_t val)
{
	char buf[32];
	int len = snprintf(buf, sizeof(buf), "%lld", (long long)val);
	if (len < 0)
	{
		return NULL;
	}
	char* result = (char*)malloc((size_t)len + 1);
	if (!result)
	{
		return NULL;
	}
	memcpy(result, buf, (size_t)len + 1);
	return result;
}

int64_t adn_string_to_i32(const char* s)
{
	if (!s)
	{
		return 0;
	}
	return (int64_t)strtoll(s, NULL, 10);
}

char* adn_f64_to_string(double val)
{
	char buf[64];
	int len = snprintf(buf, sizeof(buf), "%g", val);
	if (len < 0)
	{
		return NULL;
	}
	char* result = (char*)malloc((size_t)len + 1);
	if (!result)
	{
		return NULL;
	}
	memcpy(result, buf, (size_t)len + 1);
	return result;
}

double adn_string_to_f64(const char* s)
{
	if (!s)
	{
		return 0.0;
	}
	return strtod(s, NULL);
}