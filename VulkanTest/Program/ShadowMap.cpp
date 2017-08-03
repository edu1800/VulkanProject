#include "ShadowMap.h"
#include "ShadowCamera.h"
#include "../Core/include/GameObject.h"
#include "../Core/include/GameObjectManager.h"
#include "../Core/include/Light.h"

namespace Vulkan
{
	ShadowMap::ShadowMap(int width, int height, Light* pLight)
		:m_iWidth(width)
		,m_iHeight(height)
		,m_pLight(pLight)
	{
		GameObject* cameraGo = static_cast<GameObject*>(GameObjectManager::GetSingleton().Create("ShadowCamera"));
		m_pShadowCamera = cameraGo->AddComponent<ShadowCamera>();
		m_pShadowCamera->Init(width, height, pLight);
		m_pShadowCamera->SetDepth(-1);
	}

	ShadowMap::~ShadowMap()
	{
	}
}