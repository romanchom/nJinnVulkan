#pragma once

#include <unordered_set>
#include <Eigen/Dense>
#include "Component.hpp"
#include "TransientUniformAllocator.h"
#include "CommandBuffer.hpp"
#include "GBuffer.hpp"

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
		GBuffer mGBuffer;
		DescriptorSet mGeometryDescriptorSet;
		DescriptorSet mLightingDescriptorSet;


		void draw(const std::unordered_set<class Renderer * > deferredObjects, const std::unordered_set<class LightSource *> lights);
		//void setRenderTarget();
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

	inline Camera & Camera::verticalFieldOfView(double value) noexcept {
		mVerticalFieldOfView = value;
		mProjectionDirty = true;
		return *this;
	}

	inline double Camera::verticalFieldOfView() const noexcept {
		return mVerticalFieldOfView;
	}


	inline Camera & Camera::horizontalFieldOfView(double value) noexcept {
		mHorizontalFieldOfView = value;
		mProjectionDirty = true;
		return *this;
	}

	inline double Camera::horizontalFieldOfView() const noexcept	{
		return mHorizontalFieldOfView;
	}


	inline Camera & Camera::nearClippingPlane(double value) noexcept	{
		mNearClippingPlane = value;
		mProjectionDirty = true;
		return *this;
	}

	inline double Camera::nearClippingPlane() const noexcept	{
		return mNearClippingPlane;
	}


	inline Camera & Camera::farClippingPlane(double value) noexcept	{
		mFarClippingPlane = value;
		mProjectionDirty = true;
		return *this;
	}

	inline double Camera::farClippingPlane() const noexcept	{
		return mFarClippingPlane;
	}
}

