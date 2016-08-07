#pragma once

#include <chrono>

namespace nJinn {
	class Clock
	{
	private:
		typedef std::chrono::high_resolution_clock clock_t;
		clock_t::time_point mLastFrame;
		clock_t::time_point mCurrentFrame;
		clock_t::duration mDelta;
		clock_t::duration mTimeSinceStart;
		double mDeltaDouble;
		double mTimeDouble;
		uint64_t mFrameCount;
	public:
		Clock();
		~Clock();
		void update();

		uint64_t frame() const { return mFrameCount; }
		double time() const { return mTimeDouble; }
		double delta() const { return mDeltaDouble; }
	};

	extern Clock * clock;
}
