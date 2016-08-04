#pragma once

#include <string>
#include <cstdint>

namespace nJinn {
	namespace detail {
		const uint64_t FNV_offset_basis = 14695981039346656037ULL;
		const uint64_t FNV_prime = 1099511628211ULL;

		constexpr uint64_t recursiveHash(const char * string, size_t length, uint64_t hash = FNV_offset_basis) {
			return (length == 0) ? hash : recursiveHash(string + 1, length - 1, (hash ^ uint64_t(*string)) * FNV_prime);
		}

		uint64_t iterativeHash(const char * string, size_t length) {
			uint64_t hash = FNV_offset_basis;
			for (int i = 0; i < length; ++i) {
				hash ^= ((uint64_t)string[i]);
				hash *= FNV_prime;
			}
			return hash;
		}

		constexpr size_t strlen(const char * string) {
			return (*string == 0) ? 0 : (1 + strlen(string + 1));
		}
	}

	inline namespace literals {
		constexpr uint64_t operator""_hash(const char * string, size_t length) {
			return detail::recursiveHash(string, length);
		}
	}

	uint64_t hash(const std::string & string) {
		return detail::iterativeHash(string.data(), string.size());
	}
}