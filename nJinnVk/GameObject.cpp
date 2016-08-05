#include "stdafx.hpp"
#include "GameObject.hpp"


namespace nJinn {
	GameObject::set_type GameObject::sGameObjects;


	GameObject::GameObject() :
		mParent(nullptr),
		mTransformChanged(true)
	{
		mTransform.setIdentity();
		mRotation.setIdentity();
		mPosition.setZero();
		mScale.setOnes();
	}

	GameObject::~GameObject()
	{
		for (Component * c : mComponents) delete c;
	}

	GameObject * GameObject::create()
	{
		auto it = sGameObjects.emplace();
		auto ret = &*(it.first);
		// unordered_set contains immutable objects, 
		// but since we know that hash depends on object location in memory
		// we can safely cast constness away
		return const_cast<GameObject *>(ret);
	}

	GameObject * GameObject::addChild()
	{
		GameObject * child = create();
		child->parent(this);
		return child;
	}

	void GameObject::parent(GameObject * value)
	{
		if (mParent) {
			auto & vec = mParent->mChildren;
			auto it = std::find(vec.begin(), vec.end(), this);
			assert(it != vec.end());
			size_t index = it - vec.begin();
			if (index > vec.size() - 1) {
				vec[index] = vec.back();
			}
			vec.pop_back();
		}
		mParent = value;
		markTransformChanged();
		if (mParent) {
			mParent->mChildren.push_back(this);
		}
	}

	void GameObject::destroyNow()
	{
		for (auto child : mChildren) {
			child->destroyNow();
		}
		sGameObjects.erase(*this);
	}

	const Eigen::Matrix4d & GameObject::transform()
	{
		if (mTransformChanged) {
			Eigen::Affine3d t;
			t.setIdentity();
			t.translate(mPosition);
			t.rotate(mRotation);
			t.scale(mScale);
			mTransform = t.matrix();
			if (mParent) mTransform *= mParent->transform();
			mTransformChanged = false;
		}
		return mTransform;
	}

	void GameObject::clearScene()
	{
		sGameObjects.clear();
	}
}
