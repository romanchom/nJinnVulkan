#include "stdafx.h"
#include "nJinnVkTest.h"

#include <nJinnVk/Math.hpp>
#include <nJinnVk/Application.hpp>
#include <nJinnVk/MaterialFamily.hpp>
#include <nJinnVk/Material.hpp>
#include <nJinnVk/Mesh.hpp>
#include <nJinnVk/MeshRenderer.hpp>
#include <nJinnVk/GameObject.hpp>
#include <nJinnVk/ResourceManager.hpp>
#include <nJinnVk/Clock.hpp>
#include <nJinnVk/Camera.hpp>
#include <nJinnVk/LightSourceDirectional.hpp>


using namespace nJinn;
using namespace nJinn::literals;

class G : public GameBase {
private:
	GameObject * go;

public:
	virtual void onInitialize() override {
		go = nullptr;
	}
	
	virtual void onUpdate() override {
		if (nJinn::clock->frame() == 100) {

			GameObject * camera = GameObject::create();
			camera->position(0, -2, 0);
			auto cam = camera->addComponent<Camera>();
			(*cam)
				.nearClippingPlane(0.01)
				.farClippingPlane(1000)
				.horizontalFieldOfView(60.0_deg)
				.verticalFieldOfView(40.0_deg);

			GameObject * light = GameObject::create();
			light->addComponent<LightSourceDirectional>();

			MaterialFamily::handle matFam = resourceManager->get<MaterialFamily>("materialFamily.yml", ResourceLoadPolicy::Immediate);

			go = GameObject::create();
			go->position(0, 0, 0);
			go->rotation(Eigen::Quaterniond(Eigen::AngleAxisd(0.1, Eigen::Vector3d::UnitZ())));
			go->scale(0.1, 0.1, 0.1);

			MeshRenderer * mr = go->addComponent<MeshRenderer>();
			mr->mesh(resourceManager->get<Mesh>("asteroid.vbm", ResourceLoadPolicy::Immediate));
			mr->materialFamily(matFam);
		}
		if (nullptr != go) {
			go->rotation(Eigen::Quaterniond(Eigen::AngleAxisd(nJinn::clock->time(), Eigen::Vector3d::UnitZ())));
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