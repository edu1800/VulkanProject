#include "../include/ResourceManager.h"
#include "../include/Resource.h"
#include "../include/CommonUtil.h"

namespace Vulkan
{
	ResourceManager::ResourceManager()
	{
	}

	ResourceManager::~ResourceManager()
	{
		RemoveAll();
	}

	Resource * ResourceManager::Create(const std::string & name)
	{
		if (IsContain(name))
		{
			return NULL;
		}

		Resource* pResource = createImpl(name);
		if (pResource != NULL)
		{
			add(pResource);
		}
		return pResource;
	}

	Resource * ResourceManager::Create(const std::string & name, const std::string & fileName)
	{
		if (IsContain(name))
		{
			return NULL;
		}

		Resource* pResource = createImpl(name, fileName);
		if (pResource != NULL)
		{
			add(pResource);
		}
		return pResource;
	}

	void ResourceManager::Remove(const std::string& name)
	{
		if (!IsContain(name))
		{
			return;
		}

		ResourceMap::iterator resourceItr = m_mapResource.find(name);
		Resource* pResource = resourceItr->second;
		m_mapResource.erase(resourceItr);
		SAFE_DELETE(pResource);
	}

	void ResourceManager::RemoveAll()
	{
		ResourceMap::iterator itr = m_mapResource.begin();
		ResourceMap::iterator itrEnd = m_mapResource.end();

		for (; itr != itrEnd; itr++)
		{
			SAFE_DELETE(itr->second);
		}

		m_mapResource.clear();
	}

	bool ResourceManager::IsContain(const std::string & name)
	{
		return m_mapResource.find(name) != m_mapResource.end();
	}
	Resource * ResourceManager::GetResource(const std::string & name)
	{
		if (IsContain(name))
		{
			return m_mapResource[name];
		}

		return NULL;
	}

	const ResourceManager::ResourceMap & ResourceManager::GetAllResources()
	{
		return m_mapResource;
	}

	size_t ResourceManager::GetResourceCount()
	{
		return m_mapResource.size();
	}

	void ResourceManager::add(Resource * pResource)
	{
		if (pResource == NULL)
		{
			return;
		}

		m_mapResource.insert(std::make_pair(pResource->GetName(), pResource));
	}

	Resource* ResourceManager::createImpl(const std::string & name, const std::string & filename)
	{
		return createImpl(name);
	}
}
