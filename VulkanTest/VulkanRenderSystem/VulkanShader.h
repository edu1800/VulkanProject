#ifndef _VULAKN_SHADER_H_
#define _VULKAN_SHADER_H_

#pragma once

#include "../Core/include/Shader.h"

namespace Vulkan
{
	struct ShaderModule;
	class VulkanShader : public Shader
	{
	public:
		VulkanShader(ResourceManager* pResourceMgr, const std::string& name);
		virtual ~VulkanShader();

		void SetShaderModule(ShaderModule* pShaderModule) { m_pShaderModule = pShaderModule; }
		ShaderModule* GetShaderModule() const { return m_pShaderModule; }

	protected:
		ShaderModule* m_pShaderModule;
	};
}

#endif
