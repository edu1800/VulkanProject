#ifndef _RESOURCE_H_
#define _RESOURCE_H_

#pragma once

#include <string>
#include "CommonUtil.h"

namespace Vulkan
{
	enum ResourceType
	{
		NONE,
		GAMEOBJECT,
		MESH,
		MATERIAL,
		TEXTURE,
		SHADER,
		COMPOSITOR
	};

	class ResourceManager;
	class Resource
	{
	public:
		Resource(ResourceManager* pResourceMgr, const std::string& name);
		virtual ~Resource();

		std::string GetName() const { return m_Name; }

		virtual void Load() = 0;
		virtual void UnLoad() = 0;

		ResourceType GetResourceType() const { return m_eType; }

		GET_SET_DECLARE(Dirty, bool, m_bIsDirty)

	protected:
		std::string m_Name;
		ResourceManager* m_pResourceMgr;
		ResourceType m_eType;
		bool m_bIsDirty;
	};
}
#endif
