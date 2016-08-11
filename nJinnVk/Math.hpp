#pragma once

namespace nJinn {
	template<typename T>
	T nextMultipleOf(T value, T multipleOf) {
		value += multipleOf - 1;
		value /= multipleOf;
		value *= multipleOf;
		return value;
	}
}