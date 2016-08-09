#include "stdafx.hpp"
#include "Renderer.hpp"

#include "RendererSystem.hpp"
#include "ResourceManager.hpp"

namespace nJinn {
	Renderer::Renderer(){}

	Renderer::~Renderer()
	{
		rendererSystem->unregisterRenderer(this);
	}

	void Renderer::materialFamily(const MaterialFamily::handle & material)
	{
		mMaterialFamily = material;
		resourceManager->onResourceLoaded(material, [=] { return validate(); });
	}

	bool Renderer::validate()
	{	
		if (isValid()) {
			//mForwardMaterial = static_cast<std::unique_ptr<Material>>(mMaterialFamily->instantiate());
			rendererSystem->registerRenderer(this);
			return true;
		} else {
			return false;
		}
	}
}
