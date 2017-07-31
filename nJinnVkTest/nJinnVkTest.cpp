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
	std::vector<GameObject *> asteroids;
	bool ok;
public:
	virtual void onInitialize() override {
		ok = false;
	}
	
	virtual void onUpdate() override {
		if (nJinn::clock->frame() == 100) {

			auto camera = GameObject::create();
			camera->position(0, -4, 1);
			auto cam = camera->addComponent<Camera>();
			(*cam)
				.nearClippingPlane(0.01)
				.farClippingPlane(1000)
				.horizontalFieldOfView(80.0_deg)
				.verticalFieldOfView(60.0_deg);

			auto light = GameObject::create();
			light->addComponent<LightSourceDirectional>();

			auto matFam = resourceManager->get<MaterialFamily>("materialFamily.yml", ResourceLoadPolicy::Immediate);
			auto mesh = resourceManager->get<Mesh>("asteroid.vbm", ResourceLoadPolicy::Immediate);

			for (int x = -1; x < 0; ++x) {
				for (int y = -3; y < 3; ++y) {
					auto go = GameObject::create();
					go->position(x + 0.5, y + 0.5, 0);
					go->scale(0.1, 0.1, 0.1);

					auto mr = go->addComponent<MeshRenderer>();
					mr->mesh(mesh);
					mr->materialFamily(matFam);
					asteroids.push_back(go);
				}
			}
			ok = true;
		}
		if (ok) {
			for (auto && go : asteroids) {
				go->rotation(Eigen::Quaterniond(Eigen::AngleAxisd(nJinn::clock->time(), Eigen::Vector3d::UnitZ())));
			}
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