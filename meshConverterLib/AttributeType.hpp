#pragma once

#include <cstdint>

enum class attributeType : uint32_t {
	e8unorm = 0,
	e8snorm,
	e8uscaled,
	e8sscaled,
	e8uint,
	e8sint,
	e16unorm,
	e16snorm,
	e16uscaled,
	e16sscaled,
	e16int,
	e16uint,
	e16float,
	e32int,
	e32uint,
	e32float,
	e64int,
	e64uint,
	e64float,
};