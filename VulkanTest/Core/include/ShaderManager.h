#ifndef _SHADER_MANAGER_H_
#define _SHADER_MANAGER_H_

#pragma once

#include "ResourceManager.h"
#include "CommonUtil.h"
#include "Singleton.h"

namespace Vulkan
{
	class ShaderManager : public ResourceManager, public Singleton<ShaderManager>
	{
	public:
		ShaderManager();
		virtual ~ShaderManager();

		SINGLETON_DECLARE(ShaderManager)
	protected:
		virtual Resource* createImpl(const std::string& name);
		virtual Resource* createImpl(const std::string& name, const std::string& filename);
	};
}
#endif
