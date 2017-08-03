#ifndef _COMPONENT_H_
#define _COMPONENT_H_

#pragma once
#include <vector>

namespace Vulkan
{
	class GameObject;
	class Component
	{
	public:
		Component();
		virtual ~Component();

		void SetGameObject(GameObject* pGo) { m_pGameObject = pGo; }
		GameObject* GetGameObject() const { return m_pGameObject; }

		unsigned GetHashCode();

	protected:
		GameObject* m_pGameObject;

	private:
		unsigned m_HashCode;
	};
}

#endif
