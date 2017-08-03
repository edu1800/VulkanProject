#ifndef _VULKAN_GAMEOBJECT_H_
#define _VULKAN_GAMEOBJECT_H_

#pragma once

#include "../Core/include/GameObject.h"
#include <map>

namespace Vulkan
{
	struct ResourceBuffer;
	struct PipelineLayout;
	struct GraphicsPipeline;
	class Shader;

	typedef std::map<Shader*, ResourceBuffer*> ShaderResourceMap;

	class VulkanGameObject : public GameObject
	{
	public:
		VulkanGameObject(ResourceManager* pResourceMgr, const std::string& name);
		virtual ~VulkanGameObject();

		void SetVertexUniformBuffer(Shader* pShader, ResourceBuffer* pBuffer);
		ResourceBuffer* GetVertexUniformBuffer(Shader* pShader);

		void SetFragmentUniformBuffer(Shader* pShader, ResourceBuffer* pBuffer);
		ResourceBuffer* GetFragmentUniformBuffer(Shader* pShader);

	protected:
		ShaderResourceMap m_mapVertexUniformBuffer;
		ShaderResourceMap m_mapFragmentUniformBuffer;
	};
}

#endif
