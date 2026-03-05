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
