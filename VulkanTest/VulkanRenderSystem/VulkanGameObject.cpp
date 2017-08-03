#include "VulkanGameObject.h"
#include "../Core/include/CommonUtil.h"
#include "VulkanData.h"

namespace Vulkan
{
	VulkanGameObject::VulkanGameObject(ResourceManager * pResourceMgr, const std::string & name)
		: GameObject(pResourceMgr, name)
	{
	}

	VulkanGameObject::~VulkanGameObject()
	{
		ShaderResourceMap::iterator itr = m_mapVertexUniformBuffer.begin();
		ShaderResourceMap::iterator itrEnd = m_mapVertexUniformBuffer.end();
		for (; itr != itrEnd; ++itr)
		{
			SAFE_DELETE(itr->second);
		}
		m_mapVertexUniformBuffer.clear();

		itr = m_mapFragmentUniformBuffer.begin();
		itrEnd = m_mapFragmentUniformBuffer.end();
		for (; itr != itrEnd; ++itr)
		{
			SAFE_DELETE(itr->second);
		}
		m_mapFragmentUniformBuffer.clear();
	}

	void VulkanGameObject::SetVertexUniformBuffer(Shader * pShader, ResourceBuffer * pBuffer)
	{
		m_mapVertexUniformBuffer[pShader] = pBuffer;
	}

	ResourceBuffer * VulkanGameObject::GetVertexUniformBuffer(Shader* pShader)
	{
		if (m_mapVertexUniformBuffer.find(pShader) == m_mapVertexUniformBuffer.end())
		{
			return NULL;
		}

		return m_mapVertexUniformBuffer[pShader];
	}

	void VulkanGameObject::SetFragmentUniformBuffer(Shader * pShader, ResourceBuffer * pBuffer)
	{
		m_mapFragmentUniformBuffer[pShader] = pBuffer;
	}

	ResourceBuffer * VulkanGameObject::GetFragmentUniformBuffer(Shader* pShader)
	{
		if (m_mapFragmentUniformBuffer.find(pShader) == m_mapFragmentUniformBuffer.end())
		{
			return NULL;
		}

		return m_mapFragmentUniformBuffer[pShader];
	}
}