#ifndef _GAMEOBJECT_MANAGER_H_
#define _GAMEOBJECT_MANAGER_H_

#pragma once

#include <vector>
#include "ResourceManager.h"
#include "Singleton.h"
#include "../include/CommonUtil.h"
#include "../../ThirdParty/glm/vec3.hpp"
#include "../../ThirdParty/glm/fwd.hpp"

namespace Vulkan
{
	class GameObject;
	class Camera;
	class Light;

	typedef std::vector<Camera*> CameraList;
	typedef std::vector<Light*> LightList;
	class GameObjectManager : public ResourceManager, public Singleton<GameObjectManager>
	{
	public:
		GameObjectManager();
		virtual ~GameObjectManager();

		static GameObject* CreateGameObject(const std::string & name);
		static GameObject* CreateGameObject(const std::string& name, const glm::vec3 & pos, const glm::quat & rot, const glm::vec3 scale);

		const CameraList& GetCameraList() const { return m_listCamera; }
		const LightList& GetLightList() const { return m_listLight; }

		void AddCamera(Camera* pCamera);
		void RemoveCamera(Camera* pCamera);
		void SortCamera();
		void AddLight(Light* pLight);
		void RemoveLight(Light* pLight);

		SINGLETON_DECLARE(GameObjectManager)
	protected:
		virtual Resource* createImpl(const std::string& name);

		CameraList m_listCamera;
		LightList m_listLight;
	};

}
#endif

