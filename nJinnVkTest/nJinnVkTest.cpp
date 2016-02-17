#include "stdafx.h"
#include "nJinnVkTest.h"


#include <nJinnVk/application.hpp>
#include <nJinnVk/context.hpp>
#include <nJinnVk/window.hpp>

using namespace nJinn;

class G : public gameBase {};

MAIN_FUNCTION{
	return application::initialize<G>(APPLICATION_PARAMS_VAL);
}