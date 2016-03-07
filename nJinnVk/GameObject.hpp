#pragma once

#include <unordered_set>
#include <Eigen/Dense>
#include "Component.hpp"


namespace nJinn {
	class GameObject
	{
	private:
		Eigen::Matrix4d mTransform;
		Eigen::Quaterniond mRotation;

		Eigen::Vector3d mPosition;
		Eigen::Vector3d mScale;

		bool mTransformChanged;
		void markTransformChanged() {
			if (!mTransformChanged) {
				mTransformChanged = true;
				for (auto child : mChildren)
					child->markTransformChanged();
			}
		}

		GameObject* mParent;
		std::vector<GameObject *> mChildren;

		std::vector<ComponentBase *> mComponents;
	public:
		GameObject();
		~GameObject();

		static GameObject * create();

		template<typename Component_t>
		Component_t * addComponent();

		template<typename Component_t>
		Component_t * getComponent();

		GameObject * addChild();

		void parent(GameObject * value);
		GameObject * parent() const { return mParent; }

		void destroyNow();

		const Eigen::Vector3d position() const { return mPosition; }
		void position(const Eigen::Vector3d & value) { mPosition = value; markTransformChanged(); }
		void position(double x, double y, double z) { mPosition << x, y, z; markTransformChanged(); }

		const Eigen::Vector3d scale() const { return mScale; }
		void scale(const Eigen::Vector3d & value) { mScale = value; markTransformChanged(); }
		void scale(double x, double y, double z) { mScale << x, y, z; markTransformChanged(); }

		const Eigen::Quaterniond rotation() const { return mRotation; }
		void rotation(const Eigen::Quaterniond & value) { mRotation = value; mRotation.normalize(); markTransformChanged(); }

		const Eigen::Matrix4d & transform();

		bool operator==(const GameObject & other) const {
			return this == &other;
		}
		static void clearScene();
	private:
		struct hash {
			typedef nJinn::GameObject argument_type;
			typedef std::size_t result_type;
			result_type operator()(const nJinn::GameObject & t) const { return (size_t)&t; }
		};

		typedef std::unordered_set<GameObject, hash, std::equal_to<GameObject>, Eigen::aligned_allocator<GameObject>> set_type;
		static set_type sGameObjects;
	};


	template<typename Component_t>
	inline Component_t * GameObject::addComponent()
	{
		Component_t * comp = new Component_t();
		comp->setOwner(this);
		mComponents.push_back(comp);
		return comp;
	}

	template<typename Component_t>
	inline Component_t * GameObject::getComponent()
	{
		for (Component * c : mComponents) {
			Component_t * ret = dynamic_cast<Component_t>(c);
			if (c) return ret;
		}
		return nullptr;
	}

}
