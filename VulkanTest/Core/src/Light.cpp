#include "../include/Light.h"
#include "../include/GameObject.h"
#include "../include/Transform.h"
#include "../include/GameObjectManager.h"

namespace Vulkan
{
	Light::Light()
		:m_eLightType(LT_DIRECTION)
		,m_DiffuseColor(glm::vec3(1.0f, 1.0f, 1.0f))
		,m_SpecularColor(glm::vec3(1.0f, 1.0f, 1.0f))
		,m_fIntensity(1.0f)
	{
		GameObjectManager::GetSingleton().AddLight(this);
	}

	Light::~Light()
	{
		GameObjectManager::GetSingleton().RemoveLight(this);
	}

	glm::vec3 Light::GetDirection()
	{
		if (m_pGameObject == NULL)
		{
			return glm::vec3(0.0f, 0.0f, 1.0f);
		}

		return m_pGameObject->GetTransform()->GetForward();
	}
}