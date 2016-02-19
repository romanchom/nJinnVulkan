#include "stdafx.h"
#include "nJinnVkTest.h"


#include <nJinnVk/Application.hpp>

using namespace nJinn;

class G : public GameBase {};

MAIN_FUNCTION{
	return Application::initialize<G>(APPLICATION_PARAMS_VAL);
}