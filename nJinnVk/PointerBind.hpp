#pragma once

namespace nJinn {
	template<typename T>
	class PointerBind {
	public:
		PointerBind(T * value, T ** target) {
			*target = value;
		}
	};
}