#include "ShadowCamera.h"
#include "../Core/include/MeshRenderer.h"
#include "../Core/include/Shader.h"
#include "../Core/include/ShaderManager.h"
#include "../Core/include/Material.h"
#include "../Core/include/MaterialManager.h"
#include "../Core/include/Texture.h"
#include "../Core/include/TextureManager.h"
#include "../Core/include/GameObject.h"
#include "../Core/include/GameObjectManager.h"
#include "../Core/include/Transform.h"
#include "../Core/include/Light.h"
#include "../../ThirdParty/glm/gtc/matrix_transform.hpp"

namespace Vulkan
{
	ShadowCamera::ShadowCamera()
		:m_pLight(NULL)
	{
	}

	ShadowCamera::~ShadowCamera()
	{
		MaterialManager::GetSingleton().Remove("ShadowMapMaterial");
		ShaderManager::GetSingleton().Remove("ShadowVertexShader");
		ShaderManager::GetSingleton().Remove("ShadowFragmentShader");
		TextureManager::GetSingleton().Remove("ShadowMap");
	}

	void ShadowCamera::Init(int width, int height, Light* pLight)
	{
		Texture* pShadowMap = static_cast<Texture*>(TextureManager::GetSingleton().CreateRenderTexture("ShadowMap", width, height, false));
		Shader* pShadowVertexShader = static_cast<Shader*>(ShaderManager::GetSingleton().Create("ShadowVertexShader", "../Resource/shadowMap.vert"));
		pShadowVertexShader->SetEntryName("main");
		pShadowVertexShader->SetShaderType(VERTEX);
		pShadowVertexShader->GetShaderParameters()->RemoveAll();
		Shader* pShadowFragmentShader = static_cast<Shader*>(ShaderManager::GetSingleton().Create("ShadowFragmentShader", "../Resource/shadowMap.frag"));
		pShadowFragmentShader->SetEntryName("main");
		pShadowFragmentShader->SetShaderType(FRAGMENT);
		pShadowFragmentShader->GetShaderParameters()->AddParamter("CameraClip", GCT_FLOAT2, 1);
		Material* pShadowMaterial = static_cast<Material*>(MaterialManager::GetSingleton().Create("ShadowMapMaterial"));
		pShadowMaterial->AddShader(pShadowVertexShader);
		pShadowMaterial->AddShader(pShadowFragmentShader);
		pShadowMaterial->SetCullMode(CM_FLAG_FRONT);
		SetReplaceMaterial(pShadowMaterial);
		SetRenderTexture(pShadowMap);
		m_pLight = pLight;
		SetAspect(width / height);
		SetClearColor(1, 1, 1, 1);
	}

	bool ShadowCamera::IsRender(MeshRenderer * pRenderer)
	{
		if (!Camera::IsRender(pRenderer))
		{
			return false;
		}

		return pRenderer->GetCastShadow();
	}

	bool ShadowCamera::UpdateViewMatrix()
	{
		Transform* pTransform = m_pLight->GetGameObject()->GetTransform();
		if (pTransform->GetIsUpdateTransform())
		{
			if (m_pLight->GetLightType() == Light::LT_DIRECTION)
			{
				m_ViewMatrix = glm::lookAt(-m_pLight->GetDirection(), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
			}
			else if (m_pLight->GetLightType() == Light::LT_SPOTLIGHT)
			{
				m_ViewMatrix = glm::lookAt(pTransform->GetPosition(), pTransform->GetPosition() - pTransform->GetForward(), glm::vec3(0, 1, 0));
			}
			else
			{

			}
			pTransform->SetIsUpdateTransform(false);
			return true;
		}
		return false;
	}

	void ShadowCamera::UpdateProjectionMatrix()
	{
		if (m_pLight->GetLightType() == Light::LT_DIRECTION)
		{
			m_ProjectionMatrix =  glm::ortho(m_NearPlane, m_FarPlane, m_NearPlane, m_FarPlane, m_NearPlane, m_FarPlane);
		}
		else if (m_pLight->GetLightType() == Light::LT_SPOTLIGHT)
		{
			m_ProjectionMatrix = glm::perspective(m_FOV, m_Aspect, m_NearPlane, m_FarPlane);
		}
		else
		{

		}
	}

	void ShadowCamera::UpdateCameraInfoToShader()
	{
		Shader* pShader = static_cast<Shader*>(ShaderManager::GetSingleton().GetResource("ShadowFragmentShader"));
		glm::vec2 c(GetNearPlane(), GetFarPlane());
		pShader->GetShaderParameters()->SetParameter("CameraClip", &c[0], 2);
	}
}

