#include "../include/GameObject.h"
#include "../include/Transform.h"
#include "../include/CommonUtil.h"

namespace Vulkan
{
	GameObject::GameObject(ResourceManager* pResourceMgr, const std::string& name)
		:Resource(pResourceMgr, name)
		,m_pParentGo(NULL)
		,m_bIsStatic(false)
		,m_Radius(3.0f)
		,m_lLayer(0)
	{
		m_eType = GAMEOBJECT;
		m_pTransform = AddComponent<Transform>();
	}

	GameObject::~GameObject()
	{
		VEC_DELETE_POINTER(m_listComponent);
	}

	void GameObject::Load()
	{
	}

	void GameObject::UnLoad()
	{
	}

	void GameObject::AddChild(GameObject * pGo)
	{
		m_listChildGo.push_back(pGo);
		if (pGo->GetParent() != NULL)
		{
			pGo->GetParent()->RemoveChild(pGo);
		}
		pGo->m_pParentGo = this;
	}

	void GameObject::SetParent(GameObject * pParent)
	{
		if (GetParent() != NULL)
		{
			GetParent()->RemoveChild(this);
		}

		m_pParentGo = pParent;
		pParent->m_listChildGo.push_back(this);
	}

	void GameObject::RemoveChild(GameObject * pGo)
	{
		GameObjectList::iterator itr = std::find(m_listChildGo.begin(), m_listChildGo.end(), pGo);
		if (itr != m_listChildGo.end())
		{
			m_listChildGo.erase(itr);
		}
	}

	GameObject * GameObject::FindChild(const std::string & name)
	{
		GameObjectList::iterator itr = m_listChildGo.begin();
		GameObjectList::iterator itrEnd = m_listChildGo.end();
		for (; itr != itrEnd; ++itr)
		{
			if ((*itr)->GetName() == name)
			{
				return *itr;
			}
		}

		return NULL;
	}

	int GameObject::GetChildCount()
	{
		return m_listChildGo.size();
	}

	GameObject * GameObject::GetChild(int index)
	{
		if (index < 0 || index >= GetChildCount())
		{
			return NULL;
		}

		return m_listChildGo[index];
	}
}