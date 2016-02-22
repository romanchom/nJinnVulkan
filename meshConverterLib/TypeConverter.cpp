#include "TypeConverter.hpp"

namespace meshLoader {
	TypeConverter * TypeConverter::newConverter(const std::string & name) {
		TypeConverter * t;
		if (name == "float") {
			t = new RegularConverter<float>();
		} else if (name == "half") {
			t = new RegularConverter<half_float::half>();
		} else if (name == "byte") {
			t = new RegularConverter<int8_t>();
		} else if (name == "byten") {
			t = new NormalizingConverter<int8_t>();
		} else if (name == "ubyte") {
			t = new RegularConverter<uint8_t>();
		} else if (name == "ubyten") {
			t = new NormalizingConverter<uint8_t>();
		} else if (name == "short") {
			t = new RegularConverter<int16_t>();
		} else if (name == "shortn") {
			t = new NormalizingConverter<int16_t>();
		} else if (name == "ushort") {
			t = new RegularConverter<uint16_t>();
		} else if (name == "ushortn") {
			t = new NormalizingConverter<uint16_t>();
		} else throw std::exception("Unrecognized data name");
		return t;
	}


	template<>
	attributeType RegularConverter<uint8_t>::type() const { return attributeType::e8uscaled; }
	template<>
	attributeType RegularConverter<int8_t>::type() const { return attributeType::e8sscaled; }

	template<>
	attributeType RegularConverter<uint16_t>::type() const { return attributeType::e16uscaled; }
	template<>
	attributeType RegularConverter<int16_t>::type() const { return attributeType::e16sscaled; }
	template<>
	attributeType RegularConverter<half_float::half>::type() const { return attributeType::e16float; }


	template<>
	attributeType NormalizingConverter<uint8_t>::type() const { return attributeType::e8unorm; }
	template<>
	attributeType NormalizingConverter<int8_t>::type() const { return attributeType::e8snorm; }

	template<>
	attributeType NormalizingConverter<uint16_t>::type() const { return attributeType::e16unorm; }
	template<>
	attributeType NormalizingConverter<int16_t>::type() const { return attributeType::e16snorm; }

	template<>
	attributeType RegularConverter<float>::type() const { return attributeType::e32float; }

}
