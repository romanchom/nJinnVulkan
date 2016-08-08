#include "stdafx.h"
#include "nJinnVkTest.h"

#include <nJinnVk/Application.hpp>
#include <nJinnVk/MaterialFamily.hpp>
#include <nJinnVk/Material.hpp>
#include <nJinnVk/Mesh.hpp>
#include <nJinnVk/MeshRenderer.hpp>
#include <nJinnVk/GameObject.hpp>
#include <nJinnVk/ResourceManager.hpp>
#include <nJinnVk/Clock.hpp>

using namespace nJinn;

class G : public GameBase {
public:
	virtual void onInitialize() override {

	}
	
	virtual void onUpdate() override {
		if (nJinn::clock->frame() == 100) {
			MaterialFamily::handle matFam = resourceManager->get<MaterialFamily>("materialFamily.yml");

			GameObject * go = GameObject::create();

			MeshRenderer * mr = go->addComponent<MeshRenderer>();
			mr->mesh(resourceManager->get<Mesh>("asteroid.vbm"));
			mr->materialFamily(matFam);
		}
	
	}
	virtual void onPreRender() override {}
	virtual void onRendered() override {}
	virtual void onExit() override {}
};

#include <nJinnVk/Debug.hpp>

MAIN_FUNCTION{
	Application app(APPLICATION_PARAMS_VAL);
	return app.run<G>();
}