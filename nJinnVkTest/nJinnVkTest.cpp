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
private:
	GameObject * go;
public:
	virtual void onInitialize() override {
		go = nullptr;
	}
	
	virtual void onUpdate() override {
		if (nJinn::clock->frame() == 100) {
			MaterialFamily::handle matFam = resourceManager->get<MaterialFamily>("materialFamily.yml", true);

			go = GameObject::create();
			go->position(0.5, 0.1, 0.1);
			go->rotation(Eigen::Quaterniond(Eigen::AngleAxisd(0.1, Eigen::Vector3d::UnitY())));
			go->scale(0.1, 0.1, 0.1);

			MeshRenderer * mr = go->addComponent<MeshRenderer>();
			mr->mesh(resourceManager->get<Mesh>("asteroid.vbm", true));
			mr->materialFamily(matFam);
		}
		if (nullptr != go) {
			go->rotation(Eigen::Quaterniond(Eigen::AngleAxisd(nJinn::clock->time(), Eigen::Vector3d::UnitY())));
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