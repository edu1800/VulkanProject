#include "../include/Renderer.h"
#include "../include/Camera.h"
#include "../include/CommonUtil.h"
#include "../include/MeshManager.h"
#include "../include/Mesh.h"
#include "../include/GameObject.h"
#include "../include/GameObjectManager.h"
#include "../include/Transform.h"
#include "../include/Light.h"

namespace Vulkan
{
	Renderer::Renderer()
		:m_pCamera(NULL)
		,m_pCameraGo(NULL)
	{
	
	}

	Renderer::~Renderer()
	{
		
	}

	void Renderer::Initialize(const char * windowName, int width, int height)
	{
		m_pCameraGo = GameObjectManager::GetSingleton().CreateGameObject("Main Camera");
		m_pCamera = m_pCameraGo->AddComponent<Camera>();
		m_pCameraGo->GetTransform()->SetPosition(glm::vec3(-5.0f, 3.0f, -10.0f));
		m_pCamera->SetAspect((float)width / height);

		GameObject* pLightGo = GameObjectManager::GetSingleton().CreateGameObject("Directional Light");
		Light* pLight = pLightGo->AddComponent<Light>();
		pLightGo->GetTransform()->SetPosition(glm::vec3(0.0f, 10.0f, 0.0f));
	}

	bool Renderer::Render()
	{
		return false;
	}
}
