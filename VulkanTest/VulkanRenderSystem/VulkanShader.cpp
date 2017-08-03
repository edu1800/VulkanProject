#include "VulkanShader.h"
#include "../Core/include/CommonUtil.h"
namespace Vulkan
{
	VulkanShader::VulkanShader(ResourceManager * pResourceMgr, const std::string & name)
		:Shader(pResourceMgr, name)
		,m_pShaderModule(NULL)
	{
	}

	VulkanShader::~VulkanShader()
	{
		SAFE_DELETE(m_pShaderModule);
	}
}