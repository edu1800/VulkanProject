#ifndef _GAMEOBJECT_H_
#define _GAMEOBJECT_H_

#pragma once
#include "Resource.h"
#include <vector>
#include <string>
#include "CommonUtil.h"
#include "../../ThirdParty/glm/glm.hpp"

namespace Vulkan
{
	class Transform;
	class Component;
	class GameObject;

	typedef std::vector<Component*> ComponentList;
	typedef std::vector<GameObject*> GameObjectList;

	class GameObject : public Resource
	{
	public:
		
		GameObject(ResourceManager* pResourceMgr, const std::string& name);
		virtual ~GameObject();

		Transform* GetTransform() { return m_pTransform; }

		virtual void Load();
		virtual void UnLoad();

		void AddChild(GameObject* pGo);
		void SetParent(GameObject* pParent);
		void RemoveChild(GameObject* pGo);
		GameObject* GetParent() const { return m_pParentGo; }
		GameObject* FindChild(const std::string& name);
		int GetChildCount();
		GameObject* GetChild(int index);

		GET_SET_DECLARE(Static, bool, m_bIsStatic)
		GET_SET_DECLARE(Radius, float, m_Radius)
		GET_SET_DECLARE(Center, glm::vec3, m_Center)

		template<typename T>
		T* AddComponent();
		template<typename T>
		void RemoveComponent(T* component);
		template<typename T>
		T* GetComponent();

		GET_SET_DECLARE(Layer, unsigned long long, m_lLayer)

	private:
		Transform* m_pTransform;

		GameObject* m_pParentGo;
		GameObjectList m_listChildGo;

		ComponentList m_listComponent;

		bool m_bIsStatic;
		glm::vec3 m_Center;
		float m_Radius;
		unsigned long long m_lLayer;
	};

	template<typename T>
	inline T * GameObject::AddComponent()
	{
		T* t = new T();
		t->SetGameObject(this);
		m_listComponent.push_back(t);
		return t;
	}

	template<typename T>
	inline void GameObject::RemoveComponent(T * component)
	{
		ComponentList::iterator itr = std::find(m_listComponent.begin(), m_listComponent.end(), component);
		if (itr != m_listComponent.end())
		{
			m_listComponent.erase(itr);
		}
	}

	template<typename T>
	inline T * GameObject::GetComponent()
	{
		unsigned hashcode = CommonUtil::GetHashCode(typeid(T).name());
		ComponentList::iterator itr = m_listComponent.begin();
		ComponentList::iterator itrEnd = m_listComponent.end();
		for (; itr != itrEnd; ++itr)
		{
			if (hashcode == (*itr)->GetHashCode())
			{
				return static_cast<T*>(*itr);
			}
		}

		return NULL;
	}
}


#endif
