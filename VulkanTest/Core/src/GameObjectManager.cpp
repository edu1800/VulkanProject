#include "../include/GameObjectManager.h"
#include "../include/GameObject.h"
#include "../include/Renderer.h"
#include "../include/Root.h"
#include "../include/Transform.h"
#include "../include/Camera.h"
#include <algorithm>

namespace Vulkan
{
	SINGLETON_IMPL(GameObjectManager)

	GameObjectManager::GameObjectManager()
		:ResourceManager()
		,Singleton<GameObjectManager>()
	{
	}

	GameObjectManager::~GameObjectManager()
	{

	}

	GameObject * GameObjectManager::CreateGameObject(const std::string & name)
	{
		return CreateGameObject(name, glm::vec3(0.0f), glm::quat(), glm::vec3(1.0f));
	}

	GameObject * GameObjectManager::CreateGameObject(const std::string & name, const glm::vec3 & pos, const glm::quat & rot, const glm::vec3 scale)
	{
		GameObject* go = static_cast<GameObject*>(GameObjectManager::GetSingleton().Create(name));
		Transform* pNode = go->GetTransform();
		pNode->SetPosition(pos);
		pNode->SetRotation(rot);
		pNode->SetScale(scale);
		return go;
	}

	void GameObjectManager::AddCamera(Camera * pCamera)
	{
		m_listCamera.push_back(pCamera);
	}

	void GameObjectManager::RemoveCamera(Camera * pCamera)
	{
		CameraList::iterator itr = std::find(m_listCamera.begin(), m_listCamera.end(), pCamera);
		if (itr != m_listCamera.end())
		{
			m_listCamera.erase(itr);
		}
	}

	bool cameraDepthSorting(Camera* pLeft, Camera* pRight)
	{
		return pLeft->GetDepth() < pRight->GetDepth();
	}

	void GameObjectManager::SortCamera()
	{
		std::sort(m_listCamera.begin(), m_listCamera.end(), cameraDepthSorting);
	}

	void GameObjectManager::AddLight(Light * pLight)
	{
		m_listLight.push_back(pLight);
	}

	void GameObjectManager::RemoveLight(Light * pLight)
	{
		LightList::iterator itr = std::find(m_listLight.begin(), m_listLight.end(), pLight);
		if (itr != m_listLight.end())
		{
			m_listLight.erase(itr);
		}
	}

	Resource * Vulkan::GameObjectManager::createImpl(const std::string & name)
	{
		return Root::GetSingleton().GetRenderer()->CreateGameObject(this, name);
	}

}
