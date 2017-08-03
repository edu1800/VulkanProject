#ifndef _CAMERA_H_
#define _CAMERA_H_

#pragma once

#include "../../ThirdParty/glm/matrix.hpp"
#include "Component.h"
#include "CommonUtil.h"
#include <vector>

namespace Vulkan
{
	enum Projection_Mode
	{
		PERSPECTIVE,
		ORTHOGONAL
	};

	struct DepthStencilClearData
	{
		float depth;
		unsigned int stencil;
	};

	union ClearData
	{
		float color[4];
		DepthStencilClearData depthStencil;
	};

	struct CameraClear
	{
		bool isColor;
		ClearData clearData;

		CameraClear(float r, float g, float b, float a)
		{
			isColor = true;
			clearData.color[0] = r;
			clearData.color[1] = g;
			clearData.color[2] = b;
			clearData.color[3] = a;
		}

		CameraClear(float depth, unsigned int stencil)
		{
			isColor = false;
			clearData.depthStencil.depth = depth;
			clearData.depthStencil.stencil = stencil;
		}
	};

	class Texture;
	class GameObject;
	class MeshRenderer;
	class Material;
	class Camera : public Component
	{
	public:
		typedef std::vector<CameraClear> CameraClearList;
		Camera();
		virtual ~Camera();

		void SetFOV(float fov) { m_FOV = fov; UpdateProjectionMatrix(); }
		float GetFOV() const { return m_FOV; }

		void SetNearPlane(float nearPlane) { m_NearPlane = nearPlane; UpdateProjectionMatrix(); }
		float GetNearPlane() const { return m_NearPlane; }

		void SetFarPlane(float farPlane) { m_FarPlane = farPlane; UpdateProjectionMatrix(); }
		float GetFarPlane() const { return m_FarPlane; }

		void SetAspect(float aspect) { m_Aspect = aspect; UpdateProjectionMatrix(); }
		float GetAspect() const { return m_Aspect; }

		void SetProjectionMode(Projection_Mode mode) { m_eMode = mode; }
		Projection_Mode GetProjectionMode() const { return m_eMode; }

		const glm::mat4& GetViewMatrix();
		const glm::mat4& GetProjectionMatrix();

		void SetClearColor(float r, float g, float b, float a);
		void SetClearDepthAndStencil(float depth, unsigned int stencil);

		void UpdateTransform();
		virtual bool UpdateViewMatrix();
		virtual void UpdateProjectionMatrix();

		const CameraClearList& GetClearCameraList() const { return m_vecCameraClear; }

		void SetRenderLayer(unsigned long long layer);
		void AddRenderLayer(unsigned long long layer);
		void RemoveRenderLayer(unsigned long long layer);
		unsigned long long GetRenderLayer() const { return m_lRenderLayer; }

		void SetRenderTexture(Texture* pTexture);
		Texture* GetRenderTexture() const { return m_pRenderTexture; }

		virtual bool IsRender(MeshRenderer* pRenderer);

		GET_SET_DECLARE(Depth, int, m_iDepth)
		GET_SET_DECLARE(ReplaceMaterial, Material*, m_pReplaceMaterial)

	protected:
		float m_FOV;
		float m_Aspect;
		float m_NearPlane;
		float m_FarPlane;
		glm::mat4 m_ViewMatrix;
		glm::mat4 m_ProjectionMatrix;
		Projection_Mode m_eMode;
		unsigned long long m_lRenderLayer;
		int m_iDepth;
		Texture* m_pRenderTexture;
		Material* m_pReplaceMaterial;

		CameraClearList m_vecCameraClear;
	};
}
#endif
