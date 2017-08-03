#include "VulkanTexture.h"
#include "../Core/include/Root.h"
#include "VulkanRenderer.h"
#include "VulkanData.h"
#include "../Core/include/CommonUtil.h"
#include "VulkanUtil.h"

namespace Vulkan
{
	VulkanTexture::VulkanTexture(const std::string & fileName, ResourceManager * pResourceMgr, const std::string & name)
		:Texture(fileName, pResourceMgr, name)
		,m_iCommandBufferIndex(0)
		,m_pTextureImage(NULL)
	{
	}

	VulkanTexture::VulkanTexture(int width, int height, bool isFrameBuffer, ResourceManager * pResourceMgr, const std::string & name)
		:Texture(width, height, isFrameBuffer, pResourceMgr, name)
		,m_iCommandBufferIndex(0)
		,m_pTextureImage(NULL)
	{
	}

	VulkanTexture::~VulkanTexture()
	{
		
	}

	void VulkanTexture::Load()
	{
		if (!m_bIsLoaded)
		{
			Texture::Load();

			if (IsFrameBuffer())
			{
				return;
			}

			if (!IsRenderTarget())
			{
				createTexture();
			}
			else
			{
				createRenderTexture();
			}
			
			m_bIsLoaded = true;

			Texture::UnLoad();
		}
	}

	void VulkanTexture::UnLoad()
	{
		if (m_bIsLoaded)
		{
			Texture::UnLoad();
			SAFE_DELETE(m_pTextureImage);
			m_bIsLoaded = false;
		}
	}
	void VulkanTexture::CreateFrameBuffer(RenderPass * pRenderPass)
	{
		if (!m_bIsLoaded || !m_bIsRenderTarget || m_pTextureImage->pFrameBuffer != NULL)
		{
			return;
		}

		VulkanRenderer* pRenderer = static_cast<VulkanRenderer*>(Root::GetSingleton().GetRenderer());
		VulkanData* pVulkanData = pRenderer->GetVulkanData();

		VkImageView attachments[2];
		attachments[0] = m_pTextureImage->pImageView;
		attachments[1] = m_pTextureImage->pStageImageView;

		VkFramebufferCreateInfo fb_info = {};
		fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fb_info.pNext = NULL;
		fb_info.renderPass = pRenderPass->pRenderPass;
		fb_info.attachmentCount = 2;
		fb_info.pAttachments = attachments;
		fb_info.width = m_iTextureWidth;
		fb_info.height = m_iTextureHeight;
		fb_info.layers = 1;

		m_pTextureImage->pFrameBuffer = ((VkFramebuffer*)malloc(sizeof(VkFramebuffer)))[0];
		vkCreateFramebuffer(pVulkanData->pDevice->handle, &fb_info, NULL, &m_pTextureImage->pFrameBuffer);
	}

	VkImageType VulkanTexture::getImageType(TextureType type)
	{
		switch (type)
		{
		case Vulkan::TEX_TYPE_1D:
			return VK_IMAGE_TYPE_1D;
		case Vulkan::TEX_TYPE_2D:
			return VK_IMAGE_TYPE_2D;
		case Vulkan::TEX_TYPE_3D:
			return VK_IMAGE_TYPE_3D;
		case Vulkan::TEX_TYPE_CUBE_MAP:
			break;
		case Vulkan::TEX_TYPE_2D_ARRAY:
			break;
		default:
			break;
		}

		return VK_IMAGE_TYPE_2D;
	}
	VkImageViewType VulkanTexture::getImageViewType(TextureType type)
	{
		switch (type)
		{
		case Vulkan::TEX_TYPE_1D:
			return VK_IMAGE_VIEW_TYPE_1D;
		case Vulkan::TEX_TYPE_2D:
			return VK_IMAGE_VIEW_TYPE_2D;
		case Vulkan::TEX_TYPE_3D:
			return VK_IMAGE_VIEW_TYPE_3D;
		case Vulkan::TEX_TYPE_CUBE_MAP:
			return VK_IMAGE_VIEW_TYPE_CUBE;
		case Vulkan::TEX_TYPE_2D_ARRAY:
			return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		default:
			break;
		}

		return VK_IMAGE_VIEW_TYPE_2D;
	}
	VkFilter VulkanTexture::getSamplerFilter(TextureFilter filter)
	{
		switch (filter)
		{
		case Vulkan::TF_NEAREST:
			return VK_FILTER_NEAREST;
		case Vulkan::TF_LINEAR:
			return VK_FILTER_LINEAR;
		case Vulkan::TF_CUBIC:
			return VK_FILTER_CUBIC_IMG;
		default:
			break;
		}

		return VK_FILTER_NEAREST;
	}

	VkSamplerAddressMode VulkanTexture::getAddressMode(TextureAddressMode mode)
	{
		switch (mode)
		{
		case Vulkan::TAM_WRAP:
			return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		case Vulkan::TAM_MIRROR:
			return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		case Vulkan::TAM_CLAMP:
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		case Vulkan::TAM_BORDER:
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		}

		return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	}
	void VulkanTexture::createTexture()
	{
		VulkanRenderer* pRenderer = static_cast<VulkanRenderer*>(Root::GetSingleton().GetRenderer());
		VulkanData* pVulkanData = pRenderer->GetVulkanData();

		if (pVulkanData->pDevice == NULL)
		{
			return;
		}

		PhysicalDevice* pPhysicalDevice = pVulkanData->vecPhysicalDevices[pVulkanData->pDevice->physicalDeviceIndex];
		VkDevice vkDevice = pVulkanData->pDevice->handle;

		m_pTextureImage = new TextureImage();
		m_pTextureImage->pDevice = pVulkanData->pDevice;

		VkFormatProperties formatProps;
		vkGetPhysicalDeviceFormatProperties(pPhysicalDevice->handle, VK_FORMAT_R8G8B8A8_UNORM, &formatProps);

		/* See if we can use a linear tiled image for a texture, if not, we will
		* need a staging image for the texture data */
		VkImageUsageFlags extraUsages = 0;
		VkFormatFeatureFlags extraFeatures = 0;

		VkFormatFeatureFlags allFeatures =
			(VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | extraFeatures);
		bool needStaging =
			((formatProps.linearTilingFeatures & allFeatures) != allFeatures)
			? true
			: false;

		VkImageCreateInfo imageCreateInfo = {};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.pNext = NULL;
		imageCreateInfo.imageType = getImageType(GetTextureType());
		imageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
		imageCreateInfo.extent.width = m_iTextureWidth;
		imageCreateInfo.extent.height = m_iTextureHeight;
		imageCreateInfo.extent.depth = m_iTextureDepth;
		imageCreateInfo.mipLevels = m_iMipmap;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.usage = needStaging ? (VK_IMAGE_USAGE_TRANSFER_SRC_BIT | extraUsages) : (VK_IMAGE_USAGE_SAMPLED_BIT | extraUsages);
		imageCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
		imageCreateInfo.queueFamilyIndexCount = 0;
		imageCreateInfo.pQueueFamilyIndices = NULL;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.flags = 0;

		VkMemoryAllocateInfo mem_alloc = {};
		mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		mem_alloc.pNext = NULL;
		mem_alloc.allocationSize = 0;
		mem_alloc.memoryTypeIndex = 0;

		VkMemoryRequirements memReqs;
		VkImage mappableImage;
		VkDeviceMemory mappableMemory;

		vkCreateImage(vkDevice, &imageCreateInfo, NULL, &mappableImage);
		vkGetImageMemoryRequirements(vkDevice, mappableImage, &memReqs);
		mem_alloc.allocationSize = memReqs.size;

		VulkanUtil::GetMemoryTypeFromProperties(pPhysicalDevice, memReqs.memoryTypeBits,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&mem_alloc.memoryTypeIndex);

		vkAllocateMemory(vkDevice, &mem_alloc, NULL, &mappableMemory);
		vkBindImageMemory(vkDevice, mappableImage, mappableMemory, 0);


		/*we can't just copy the pixel bytes directly into the image memory with memcpy and
		assume that this works correctly. The problem is that there may be padding bytes between rows of pixels.
		In other words, the graphics card may assume that one row of pixels is not texWidth * 4 bytes wide,
		but rather texWidth * 4 + paddingBytes.
		To handle this correctly, we need to query how bytes are arranged in our staging image using vkGetImageSubresourceLayout:*/
		VkImageSubresource subres = {};
		subres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subres.mipLevel = 0;
		subres.arrayLayer = 0;
		VkSubresourceLayout subResourceLayout;
		vkGetImageSubresourceLayout(vkDevice, mappableImage, &subres, &subResourceLayout);

		if (!m_bIsRenderTarget)
		{
			void* data = NULL;
			vkMapMemory(vkDevice, mappableMemory, 0, memReqs.size, 0, &data);
			copyTexture(subResourceLayout.rowPitch, (unsigned char*)data);
			vkUnmapMemory(vkDevice, mappableMemory);
		}

		if (!needStaging)
		{
			/* If we can use the linear tiled image as a texture, just do it */
			m_pTextureImage->pImage = mappableImage;
			m_pTextureImage->pMem = mappableMemory;
			m_pTextureImage->pStagingImage = NULL;
			m_pTextureImage->pStagingMem = NULL;
			VulkanUtil::SetImageLayout(*pVulkanData, m_pTextureImage->pImage,
				VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_iCommandBufferIndex);
		}
		else
		{
			/* The mappable image cannot be our texture, so create an optimally
			* tiled image and blit to it */
			imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

			vkCreateImage(vkDevice, &imageCreateInfo, NULL, &m_pTextureImage->pImage);
			vkGetImageMemoryRequirements(vkDevice, m_pTextureImage->pImage, &memReqs);
			mem_alloc.allocationSize = memReqs.size;

			VulkanUtil::GetMemoryTypeFromProperties(pPhysicalDevice, memReqs.memoryTypeBits, 0, &mem_alloc.memoryTypeIndex);
			vkAllocateMemory(vkDevice, &mem_alloc, NULL, &m_pTextureImage->pMem);
			vkBindImageMemory(vkDevice, m_pTextureImage->pImage, m_pTextureImage->pMem, 0);

			/* Since we're going to blit from the mappable image, set its layout to
			* SOURCE_OPTIMAL */
			/* Side effect is that this will create info.cmd */
			VulkanUtil::SetImageLayout(*pVulkanData, mappableImage, VK_IMAGE_ASPECT_COLOR_BIT,
				VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_iCommandBufferIndex);

			/* Since we're going to blit to the texture image, set its layout to
			* DESTINATION_OPTIMAL */
			VulkanUtil::SetImageLayout(*pVulkanData, m_pTextureImage->pImage, VK_IMAGE_ASPECT_COLOR_BIT,
				VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_iCommandBufferIndex);

			if (!m_bIsRenderTarget)
			{
				VkImageCopy copyRegion;
				copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				copyRegion.srcSubresource.mipLevel = 0;
				copyRegion.srcSubresource.baseArrayLayer = 0;
				copyRegion.srcSubresource.layerCount = 1;
				copyRegion.srcOffset.x = 0;
				copyRegion.srcOffset.y = 0;
				copyRegion.srcOffset.z = 0;
				copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				copyRegion.dstSubresource.mipLevel = 0;
				copyRegion.dstSubresource.baseArrayLayer = 0;
				copyRegion.dstSubresource.layerCount = 1;
				copyRegion.dstOffset.x = 0;
				copyRegion.dstOffset.y = 0;
				copyRegion.dstOffset.z = 0;
				copyRegion.extent.width = m_iTextureWidth;
				copyRegion.extent.height = m_iTextureHeight;
				copyRegion.extent.depth = m_iTextureDepth;

				vkCmdCopyImage(pVulkanData->pDevice->vecCommandBuffer[m_iCommandBufferIndex], mappableImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					m_pTextureImage->pImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1, &copyRegion);
			}

			VulkanUtil::SetImageLayout(*pVulkanData, m_pTextureImage->pImage, VK_IMAGE_ASPECT_COLOR_BIT,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_iCommandBufferIndex);

			m_pTextureImage->pStagingImage = mappableImage;
			m_pTextureImage->pStagingMem = mappableMemory;
		}

		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.pNext = NULL;
		viewInfo.image = m_pTextureImage->pImage;
		viewInfo.viewType = getImageViewType(m_eTextureType);
		viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
		viewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
		viewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
		viewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
		viewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		vkCreateImageView(vkDevice, &viewInfo, NULL, &m_pTextureImage->pImageView);

		VkSamplerCreateInfo samplerCreateInfo = {};
		samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.magFilter = getSamplerFilter(GetMagFilter());
		samplerCreateInfo.minFilter = getSamplerFilter(GetMinFilter());
		samplerCreateInfo.addressModeU = getAddressMode(GetAddressModeU());
		samplerCreateInfo.addressModeV = getAddressMode(GetAddressModeV());
		samplerCreateInfo.addressModeW = getAddressMode(GetAddressModeW());
		samplerCreateInfo.mipLodBias = 0.0f;
		samplerCreateInfo.anisotropyEnable = IsEnableAnisotropy() ? VK_TRUE : VK_FALSE;
		samplerCreateInfo.maxAnisotropy = GetmaxAnisotropy();
		samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
		samplerCreateInfo.minLod = 0.0f;
		samplerCreateInfo.maxLod = 0.0f;
		samplerCreateInfo.compareEnable = VK_FALSE;
		samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

		vkCreateSampler(vkDevice, &samplerCreateInfo, NULL, &m_pTextureImage->pSampler);

		m_pTextureImage->stImageInfo.imageView = m_pTextureImage->pImageView;
		m_pTextureImage->stImageInfo.sampler = m_pTextureImage->pSampler;
		m_pTextureImage->stImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	}
	void VulkanTexture::createRenderTexture()
	{
		VulkanRenderer* pRenderer = static_cast<VulkanRenderer*>(Root::GetSingleton().GetRenderer());
		VulkanData* pVulkanData = pRenderer->GetVulkanData();

		if (pVulkanData->pDevice == NULL)
		{
			return;
		}

		VkDevice vkDevice = pVulkanData->pDevice->handle;

		m_pTextureImage = new TextureImage();
		m_pTextureImage->pDevice = pVulkanData->pDevice;

		// We will sample directly from the color attachment
		m_pTextureImage->pImage = VulkanUtil::CreateVkImage(vkDevice, VK_IMAGE_TYPE_2D, pVulkanData->pWindow->format,
			m_iTextureWidth, m_iTextureHeight, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_IMAGE_TILING_OPTIMAL);

		VkMemoryRequirements memReqs;
		vkGetImageMemoryRequirements(vkDevice, m_pTextureImage->pImage, &memReqs);

		VkMemoryAllocateInfo mem_alloc = {};
		mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		mem_alloc.pNext = NULL;
		mem_alloc.allocationSize = memReqs.size;
		VulkanUtil::GetMemoryTypeFromProperties(pVulkanData->vecPhysicalDevices[pVulkanData->pDevice->physicalDeviceIndex], memReqs.memoryTypeBits, 0, &mem_alloc.memoryTypeIndex);

		VkResult result = vkAllocateMemory(vkDevice, &mem_alloc, NULL, &m_pTextureImage->pMem);
		result = vkBindImageMemory(vkDevice, m_pTextureImage->pImage, m_pTextureImage->pMem, 0);

		m_pTextureImage->pImageView = VulkanUtil::CreateVkImageView(vkDevice, VK_IMAGE_VIEW_TYPE_2D, pVulkanData->pWindow->format,
			VK_IMAGE_ASPECT_COLOR_BIT, m_pTextureImage->pImage);

		m_pTextureImage->pStagingImage = VulkanUtil::CreateVkImage(vkDevice, VK_IMAGE_TYPE_2D, VK_FORMAT_D16_UNORM, m_iTextureWidth, m_iTextureHeight,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);

		vkGetImageMemoryRequirements(vkDevice, m_pTextureImage->pStagingImage, &memReqs);
		mem_alloc.allocationSize = memReqs.size;
		VulkanUtil::GetMemoryTypeFromProperties(pVulkanData->vecPhysicalDevices[pVulkanData->pDevice->physicalDeviceIndex], memReqs.memoryTypeBits, 0, &mem_alloc.memoryTypeIndex);
		result = vkAllocateMemory(vkDevice, &mem_alloc, NULL, &m_pTextureImage->pStagingMem);
		result = vkBindImageMemory(vkDevice, m_pTextureImage->pStagingImage, m_pTextureImage->pStagingMem, 0);

		m_pTextureImage->pStageImageView = VulkanUtil::CreateVkImageView(vkDevice, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_D16_UNORM,
			VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, m_pTextureImage->pStagingImage);

		VkSamplerCreateInfo samplerCreateInfo = {};
		samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.magFilter = getSamplerFilter(GetMagFilter());
		samplerCreateInfo.minFilter = getSamplerFilter(GetMinFilter());
		samplerCreateInfo.addressModeU = getAddressMode(GetAddressModeU());
		samplerCreateInfo.addressModeV = getAddressMode(GetAddressModeV());
		samplerCreateInfo.addressModeW = getAddressMode(GetAddressModeW());
		samplerCreateInfo.mipLodBias = 0.0f;
		samplerCreateInfo.anisotropyEnable = IsEnableAnisotropy() ? VK_TRUE : VK_FALSE;
		samplerCreateInfo.maxAnisotropy = GetmaxAnisotropy();
		samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
		samplerCreateInfo.minLod = 0.0f;
		samplerCreateInfo.maxLod = 0.0f;
		samplerCreateInfo.compareEnable = VK_FALSE;
		samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

		vkCreateSampler(vkDevice, &samplerCreateInfo, NULL, &m_pTextureImage->pSampler);

		m_pTextureImage->stImageInfo.imageView = m_pTextureImage->pImageView;
		m_pTextureImage->stImageInfo.sampler = m_pTextureImage->pSampler;
		m_pTextureImage->stImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}
}