#pragma once

#include <iostream>

namespace nJinn {
	template<typename Elem, typename Traits = std::char_traits<Elem>>
	class BasicTeeStream// : std::basic_ostream<Elem, Traits>
	{
	private:
		std::ostream * o1;
		std::ostream * o2;
	public:
		typedef std::basic_ostream<Elem, Traits> SuperType;

		BasicTeeStream() : 
			o1(nullptr),
			o2(nullptr)
		{

		}

		BasicTeeStream(std::ostream& o1, std::ostream& o2)
			: //SuperType(o1.rdbuf()),
			o1(&o1),
			o2(&o2)
		{}

		BasicTeeStream & operator << (SuperType & (__cdecl *manip)(SuperType &)) {
			(*o2) << manip;
			(*o1) << manip;
			return *this;
		}

		template<typename T>
		BasicTeeStream & operator << (const T & t) {
			(*o1) << t;
			(*o2) << t;
			return *this;
		}
	};

	typedef BasicTeeStream<char> TeeStream;
}