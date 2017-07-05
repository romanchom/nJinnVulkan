#pragma once

#include <unordered_set>
#include <Eigen/Dense>
#include "Component.hpp"
#include "TransientUniformAllocator.h"
#include "CommandBuffer.hpp"

namespace nJinn {
	class Camera : public Component
	{
	private:
		TransientUniformAllocator mAllocator;
		CommandBuffer mCommandBuffer;

		double mVerticalFieldOfView;
		double mHorizontalFieldOfView;
		double mNearClippingPlane;
		double mFarClippingPlane;

		Eigen::Matrix4d mProjection;
		bool mProjectionDirty;

		void draw(const std::unordered_set<class Renderer * > deferredObjects, const std::unordered_set<class LightSource *> lights);
		void setRenderTarget();
	public:
		Camera();
		virtual ~Camera();

		inline Camera & verticalFieldOfView(double value) noexcept;
		inline double verticalFieldOfView() const noexcept;

		inline Camera & horizontalFieldOfView(double value) noexcept;
		inline double horizontalFieldOfView() const noexcept;

		inline Camera & nearClippingPlane(double value) noexcept;
		inline double nearClippingPlane() const noexcept;

		inline Camera & farClippingPlane(double value) noexcept;
		inline double farClippingPlane() const noexcept;

		friend class RendererSystem;
	};

	inline Camera & nJinn::Camera::verticalFieldOfView(double value) noexcept {
		mVerticalFieldOfView = value;
		mProjectionDirty = true;
		return *this;
	}

	inline double nJinn::Camera::verticalFieldOfView() const noexcept {
		return mVerticalFieldOfView;
	}


	inline Camera & nJinn::Camera::horizontalFieldOfView(double value) noexcept {
		mHorizontalFieldOfView = value;
		mProjectionDirty = true;
		return *this;
	}

	inline double nJinn::Camera::horizontalFieldOfView() const noexcept	{
		return mHorizontalFieldOfView;
	}


	inline Camera & nJinn::Camera::nearClippingPlane(double value) noexcept	{
		mNearClippingPlane = value;
		mProjectionDirty = true;
		return *this;
	}

	inline double nJinn::Camera::nearClippingPlane() const noexcept	{
		return mNearClippingPlane;
	}


	inline Camera & nJinn::Camera::farClippingPlane(double value) noexcept	{
		mFarClippingPlane = value;
		mProjectionDirty = true;
		return *this;
	}

	inline double nJinn::Camera::farClippingPlane() const noexcept	{
		return mFarClippingPlane;
	}
}

