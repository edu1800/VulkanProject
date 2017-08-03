#include "../include/Resource.h"

namespace Vulkan
{
	Resource::Resource(ResourceManager* pResourceMgr, const std::string& name)
		:m_pResourceMgr(pResourceMgr)
		,m_Name(name)
		,m_bIsDirty(true)
	{

	}

	Resource::~Resource()
	{

	}
}