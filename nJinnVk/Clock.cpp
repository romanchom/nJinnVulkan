#include "stdafx.hpp"
#include "Clock.hpp"

namespace nJinn {
	Clock * clock;

	Clock::Clock() :
		mLastFrame(clock_t::now()),
		mCurrentFrame(clock_t::now()),
		mDelta(0),
		mTimeSinceStart(0),
		mDeltaDouble(0.0),
		mTimeDouble(0.0),
		mFrameCount(0)
	{}


	Clock::~Clock()
	{}

	void Clock::update()
	{
		++mFrameCount;
		mLastFrame = mCurrentFrame;
		mCurrentFrame = clock_t::now();
		mDelta = mCurrentFrame - mLastFrame;
		mTimeSinceStart += mDelta;
		mDeltaDouble = static_cast<std::chrono::duration<double>>(mDelta).count();
		mTimeDouble = static_cast<std::chrono::duration<double>>(mTimeSinceStart).count();
	}
}
