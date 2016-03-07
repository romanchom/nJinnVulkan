#include "stdafx.h"
#include "nJinnVkTest.h"


#include <nJinnVk/Application.hpp>
#include <nJinnVk/MaterialFamily.hpp>
#include <nJinnVk/Material.hpp>
#include <nJinnVk/Mesh.hpp>
#include <nJinnVk/MeshRenderer.hpp>
#include <nJinnVk/GameObject.hpp>

using namespace nJinn;

class G : public GameBase {
public:
	virtual void onInitialize() override {
		MaterialFamily::p matFam = MaterialFamily::load("ASD");

		GameObject * go = GameObject::create();

		MeshRenderer * mr = go->addComponent<MeshRenderer>();
		mr->mMesh = Mesh::load("asteroid.vbm");
		mr->mForwardMaterial = MaterialFamily::load("")->instantiate();

	}
	
	virtual void onUpdate() override {
	
	}
	
	virtual void onPreRender() override {}
	virtual void onRendered() override {}
	virtual void onExit() override {
	
	}
};

MAIN_FUNCTION{
	return Application::initialize<G>(APPLICATION_PARAMS_VAL);
}