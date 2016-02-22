#pragma once

#include <vector>
#include <limits>
#include <cassert>
#include <half.hpp>

#include "AttributeType.hpp"

namespace meshLoader {
	class TypeConverter {
	public:
		virtual size_t convert(char * dst, const float * data, size_t count) const = 0;
		virtual attributeType type() const = 0;
		virtual size_t size() const = 0;
		static TypeConverter * newConverter(const std::string & name);
	};

	template<typename Dst>
	class TypeConverterD : public TypeConverter {
	public:
		virtual size_t size() const;
	};

	template<typename Dst>
	size_t TypeConverterD<Dst>::size() const {
		return sizeof(Dst);
	}

	template <typename Dst>
	class RegularConverter : public TypeConverterD<Dst> {
	public:
		virtual size_t convert(char * dst, const float * data, size_t count) const;
		virtual attributeType type() const;
	};

	template <typename Dst>
	size_t RegularConverter<Dst>::convert(char * dst, const float * data, size_t count) const {
		Dst * casted = reinterpret_cast<Dst *>(dst);
		for (size_t i = 0; i < count; ++i) {
			casted[i] = static_cast<Dst>(data[i]);
		}
		return sizeof(Dst) * count;
	}

	template <typename Dst>
	class NormalizingConverter : public TypeConverterD<Dst> {
		virtual size_t convert(char * dst, const float * data, size_t count) const;
		virtual attributeType type() const;
	};


	template <typename Dst>
	size_t NormalizingConverter<Dst>::convert(char * dst, const float * data, size_t count) const {
		Dst * casted = (Dst *)dst;
		for (uint32_t i = 0; i < count; ++i) {
			casted[i] = (Dst)(data[i] * (float)std::numeric_limits<Dst>::max());
		}
		return sizeof(Dst) * count;
	}
}
