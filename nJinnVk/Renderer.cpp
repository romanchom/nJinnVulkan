#include "stdafx.hpp"
#include "Renderer.hpp"



namespace nJinn {
	std::unordered_set<Renderer *> Renderer::sRenderers;


	Renderer::Renderer()
	{
		sRenderers.insert(this);
	}

	Renderer::~Renderer()
	{
		sRenderers.erase(this);
		if (mForwardMaterial) delete mForwardMaterial;
	}


}
