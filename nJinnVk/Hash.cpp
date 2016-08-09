#include "stdafx.hpp"

#include "Hash.hpp"

uint64_t nJinn::detail::iterativeHash(const char * string, size_t length)
{
	uint64_t hash = FNV_offset_basis;
	for (int i = 0; i < length; ++i) {
		hash ^= ((uint64_t)string[i]);
		hash *= FNV_prime;
	}
	return hash;
}
