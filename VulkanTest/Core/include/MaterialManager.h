#ifndef _MATERIAL_MANAGER_H_
#define _MATERIAL_MANAGER_H_

#pragma once

#include "ResourceManager.h"
#include "Singleton.h"
#include "CommonUtil.h"

namespace Vulkan
{
	class Material;
	class MaterialManager : public ResourceManager, public Singleton<MaterialManager>
	{
	public:
		MaterialManager();
		virtual ~MaterialManager();

		Material* GetDefaultMaterial() const { return m_pDefaultMaterial; }

		SINGLETON_DECLARE(MaterialManager)
	protected:
		virtual Resource* createImpl(const std::string& name);
		Material* m_pDefaultMaterial;
	};
}

#endif
