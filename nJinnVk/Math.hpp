#pragma once

#include <cmath>

namespace nJinn {
	namespace detail {
		template<typename T>
		constexpr T radToDeg() {
			return static_cast<T>(180.0 / M_PI);
		}

		template<typename T>
		constexpr T degToRad() {
			return static_cast<T>(M_PI / 180.0);
		}
	}

	template<typename T>
	constexpr T radiansToDegrees(T input) {
		return input * detail::radToDeg<T>();
	}

	template<typename T>
	constexpr T degreesToRadians(T input) {
		return input * detail::degToRad<T>();
	}


	namespace literals {
		constexpr double operator ""_deg(long double input) {
			return static_cast<double>(degreesToRadians(input));
		}

		constexpr double operator ""_rad(long double input) {
			return static_cast<double>(input);
		}
	}


	template<typename T>
	T nextMultipleOf(T value, T multipleOf) {
		value += multipleOf - 1;
		value /= multipleOf;
		value *= multipleOf;
		return value;
	}
}