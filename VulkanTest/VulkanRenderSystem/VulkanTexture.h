#ifndef _VULKAN_TEXTURE_H_
#define _VULKAN_TEXTURE_H_

#pragma once

#include "../Core/include/Texture.h"
#include "../ThirdParty/vulkan/include/vulkan.h"

namespace Vulkan
{
	struct TextureImage;
	struct RenderPass;
	class VulkanTexture : public Texture
	{
	public:
		VulkanTexture(const std::string& fileName, ResourceManager* pResourceMgr, const std::string& name);
		VulkanTexture(int width, int height, bool isFrameBuffer, ResourceManager * pResourceMgr, const std::string & name);
		virtual ~VulkanTexture();

		virtual void Load();
		virtual void UnLoad();

		void CreateFrameBuffer(RenderPass* pRenderPass);
		TextureImage* GetTextureImage() const { return m_pTextureImage; }

		void SetCommandBufferIndex(int commandBufferIndex) { m_iCommandBufferIndex = commandBufferIndex; }

	protected:
		VkImageType getImageType(TextureType type);
		VkImageViewType getImageViewType(TextureType type);
		VkFilter getSamplerFilter(TextureFilter filter);
		VkSamplerAddressMode getAddressMode(TextureAddressMode mode);

		void createTexture();
		void createRenderTexture();

		int m_iCommandBufferIndex;
		TextureImage* m_pTextureImage;
	};
}

#endif
