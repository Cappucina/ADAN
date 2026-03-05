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