#include "../include/Camera.h"
#include "../../ThirdParty/glm/gtc/matrix_transform.hpp"
#include "../include/Transform.h"
#include "../include/GameObject.h"
#include "../include/GameObjectManager.h"
#include "../include/Texture.h"
#include "../include/MeshRenderer.h"

namespace Vulkan
{
	Camera::Camera()
		:m_FOV(0.785f) //45«×
		,m_Aspect(1.0f)
		,m_NearPlane(0.1f)
		,m_FarPlane(100.0f)
		,m_eMode(PERSPECTIVE)
		,m_lRenderLayer(0xFFFFFFFFFFFFFFFF)
		,m_iDepth(0)
		,m_pRenderTexture(NULL)
		,m_pReplaceMaterial(NULL)
	{
		m_FOV = glm::radians(45.0f);
		GameObjectManager::GetSingleton().AddCamera(this);
		SetClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		SetClearDepthAndStencil(1.0f, 0);
	}

	Camera::~Camera()
	{
		GameObjectManager::GetSingleton().RemoveCamera(this);
	}

	const glm::mat4& Camera::GetViewMatrix()
	{
		return m_ViewMatrix;
	}

	const glm::mat4& Camera::GetProjectionMatrix()
	{
		return m_ProjectionMatrix;
	}

	void Camera::SetClearColor(float r, float g, float b, float a)
	{
		CameraClearList::iterator itr = m_vecCameraClear.begin();
		CameraClearList::iterator itrEnd = m_vecCameraClear.end();
		for (; itr != itrEnd; ++itr)
		{
			if (itr->isColor)
			{
				itr->clearData.color[0] = r;
				itr->clearData.color[1] = g;
				itr->clearData.color[2] = b;
				itr->clearData.color[3] = a;

				return;
			}
		}

		m_vecCameraClear.push_back(CameraClear(r, g, b, a));
	}

	void Camera::SetClearDepthAndStencil(float depth, unsigned int stencil)
	{
		CameraClearList::iterator itr = m_vecCameraClear.begin();
		CameraClearList::iterator itrEnd = m_vecCameraClear.end();
		for (; itr != itrEnd; ++itr)
		{
			if (!itr->isColor)
			{
				itr->clearData.depthStencil.depth = depth;
				itr->clearData.depthStencil.stencil = stencil;

				return;
			}
		}
		m_vecCameraClear.push_back(CameraClear(depth, stencil));
	}
	void Camera::UpdateTransform()
	{
		UpdateViewMatrix();
		UpdateProjectionMatrix();
	}

	bool Camera::UpdateViewMatrix()
	{
		Transform* pTransform = GetGameObject()->GetTransform();
		if (pTransform->GetIsUpdateTransform())
		{
			m_ViewMatrix = glm::lookAt(pTransform->GetPosition(), pTransform->GetPosition() + pTransform->GetForward(), pTransform->GetUp());
			pTransform->SetIsUpdateTransform(false);
			return true;
		}
		return false;
	}

	void Camera::UpdateProjectionMatrix()
	{
		glm::mat4 clip = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, -1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.5f, 0.0f,
			0.0f, 0.0f, 0.5f, 1.0f);

		if (m_eMode == PERSPECTIVE)
		{
			m_ProjectionMatrix = clip * glm::perspective(m_FOV, m_Aspect, m_NearPlane, m_FarPlane);
		}
	}

	void Camera::SetRenderLayer(unsigned long long layer)
	{
		m_lRenderLayer = layer;
	}

	void Camera::AddRenderLayer(unsigned long long layer)
	{
		m_lRenderLayer |= layer;
	}

	void Camera::RemoveRenderLayer(unsigned long long layer)
	{
		m_lRenderLayer = m_lRenderLayer & (~layer);
	}

	void Camera::SetRenderTexture(Texture * pTexture)
	{
		if (pTexture != NULL && pTexture->IsRenderTarget())
		{
			m_pRenderTexture = pTexture;
		}
	}

	bool Camera::IsRender(MeshRenderer * pRenderer)
	{
		if (pRenderer == NULL)
		{
			return false;
		}

		return (m_lRenderLayer & pRenderer->GetGameObject()->GetLayer()) > 0;
	}
}
