#ifndef _RESOURCE_MANAGER_H_
#define _RESOURCE_MANAGER_H_

#pragma once

#include <map>
#include <string>

namespace Vulkan
{
	class Resource;
	class ResourceManager
	{
	public:
		typedef std::map<std::string, Resource*> ResourceMap;

		ResourceManager();
		virtual ~ResourceManager();

		virtual Resource* Create(const std::string& name);
		virtual Resource* Create(const std::string& name, const std::string& fileName);
		virtual void Remove(const std::string& name);
		virtual void RemoveAll();
		virtual bool IsContain(const std::string& name);

		Resource* GetResource(const std::string& name);
		const ResourceMap& GetAllResources();

		size_t GetResourceCount();

	protected:
		virtual void add(Resource* pResource);
		virtual Resource* createImpl(const std::string& name) = 0;
		virtual Resource* createImpl(const std::string& name, const std::string& filename);

		ResourceMap m_mapResource;
	};
}


#endif
