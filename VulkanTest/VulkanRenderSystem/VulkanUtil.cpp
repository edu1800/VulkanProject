#include "VulkanUtil.h"
#include "../Core/include/Transform.h"
#include "../Core/include/Camera.h"
#include "../Core/include/GameObject.h"
#include "../Core/include/GameObjectManager.h"
#include "../Core/include/MeshManager.h"
#include "../Core/include/Mesh.h"
#include "../Core/include/Material.h"
#include "../Core/include/Texture.h"
#include "../Core/include/ShaderParameter.h"
#include "../Core/include/MeshRenderer.h"
#include "../Core/include/Light.h"
#include "../Core/include/Compositor.h"
#include "../Core/include/CompositorManager.h"
#include "../ThirdParty/glm/vec3.hpp"
#include "../ThirdParty/glm/mat4x4.hpp"
#include "../ThirdParty/SPIRV/GlslangToSpv.h"
#include "VulkanVertexBuffer.h"
#include "../Core/include/VertexIndexData.h"
#include "VulkanGameObject.h"
#include "VulkanShader.h"
#include "VulkanTexture.h"
#include "VulkanIndexBuffer.h"
#include "../ThirdParty/glfw/include/GLFW/glfw3.h"

#ifdef Windows
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include "../ThirdParty/glfw/include/GLFW/glfw3native.h"

namespace Vulkan
{
	VkResult VulkanUtil::CreateVkInstance(struct VulkanData& data, const char* appName, int appVersion, const char* engineName, int engineVersion)
	{
		VkApplicationInfo app_info = {};
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pNext = NULL;
		app_info.pApplicationName = appName;
		app_info.applicationVersion = appVersion;
		app_info.pEngineName = engineName;
		app_info.engineVersion = engineVersion;
		app_info.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo inst_info = {};
		inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		inst_info.pNext = NULL;
		inst_info.flags = 0;
		inst_info.pApplicationInfo = &app_info;
		inst_info.enabledLayerCount = data.vecInstanceLayerName.size();
		inst_info.ppEnabledLayerNames = inst_info.enabledLayerCount > 0 ? data.vecInstanceLayerName.data() : NULL;
		inst_info.enabledExtensionCount = data.vecInstanceExtensionName.size();
		inst_info.ppEnabledExtensionNames = inst_info.enabledExtensionCount > 0 ? data.vecInstanceExtensionName.data() : NULL;

		return vkCreateInstance(&inst_info, NULL, &data.pInstance);
	}

	void VulkanUtil::AddInstanceExtensionName(VulkanData & data)
	{
		unsigned int glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		for (size_t i = 0; i < glfwExtensionCount; i++)
		{
			data.vecInstanceExtensionName.push_back(glfwExtensions[i]);
		}
	}

	void VulkanUtil::AddDeviceExtensionName(VulkanData & data)
	{
		data.vecDeviceExtensionName.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	}

	VkResult VulkanUtil::GetPhysicalDevices(struct VulkanData& data)
	{
		uint32_t deviceCount = 0;
		VkResult res = vkEnumeratePhysicalDevices(data.pInstance, &deviceCount, NULL);

		std::vector<VkPhysicalDevice> devices;
		devices.resize(deviceCount);

		res = vkEnumeratePhysicalDevices(data.pInstance, &deviceCount, devices.data());
		data.vecPhysicalDevices.clear();

		if (res == VK_SUCCESS)
		{
			for (size_t i = 0; i < deviceCount; i++)
			{
				PhysicalDevice* device = new PhysicalDevice();
				device->handle = devices[i];
				vkGetPhysicalDeviceQueueFamilyProperties(device->handle, &device->queueFamilyPropertyCount, NULL);
				device->vecQueueFamilyProps.resize(device->queueFamilyPropertyCount);
				vkGetPhysicalDeviceQueueFamilyProperties(device->handle, &device->queueFamilyPropertyCount, device->vecQueueFamilyProps.data());
				vkGetPhysicalDeviceMemoryProperties(device->handle, &device->memProps);
				vkGetPhysicalDeviceProperties(device->handle, &device->props);
				data.vecPhysicalDevices.push_back(device);
			}
		}

		return res;
	}

	VkResult VulkanUtil::CreateDevice(VulkanData & data, int physicalDeviceIndex)
	{
		if (data.u32graphicsQueueFamilyIndex != UINT32_MAX)
		{
			return createDevice(data, VK_QUEUE_GRAPHICS_BIT, physicalDeviceIndex, data.u32graphicsQueueFamilyIndex);
		}

		return CreateDevice(data, VK_QUEUE_GRAPHICS_BIT, physicalDeviceIndex);
	}

	VkResult VulkanUtil::CreateDevice(struct VulkanData& data, VkQueueFlagBits flag, int physicalDeviceIndex)
	{
		if (physicalDeviceIndex < 0 || physicalDeviceIndex >= data.vecPhysicalDevices.size())
		{
			return VK_NOT_READY;
		}

		Device* device = NULL;
		PhysicalDevice* physicalDevice = data.vecPhysicalDevices[physicalDeviceIndex];
		uint32_t queueFamilyIndex = UINT32_MAX;

		for (size_t i = 0; i < physicalDevice->queueFamilyPropertyCount; i++)
		{
			if (physicalDevice->vecQueueFamilyProps[i].queueFlags & flag)
			{
				queueFamilyIndex = i;
				break;
			}
		}

		if (queueFamilyIndex != UINT32_MAX)
		{
			return createDevice(data, flag, physicalDeviceIndex, queueFamilyIndex);
		}

		return VK_NOT_READY;
	}

	VkResult VulkanUtil::SubmitQueue(VulkanData & data, Semaphore* waitSemaphore,  int commandBufferIndex)
	{
		if (data.pDevice == NULL || waitSemaphore == NULL || data.pFence == NULL)
		{
			return VK_NOT_READY;
		}

		VkPipelineStageFlags pipe_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkSubmitInfo submit_info[1] = {};
		submit_info[0].pNext = NULL;
		submit_info[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info[0].waitSemaphoreCount = 1;
		submit_info[0].pWaitSemaphores = &waitSemaphore->pSemaphore;
		submit_info[0].pWaitDstStageMask = &pipe_stage_flags;
		submit_info[0].commandBufferCount = 1;
		submit_info[0].pCommandBuffers = &data.pDevice->vecCommandBuffer[commandBufferIndex];
		submit_info[0].signalSemaphoreCount = 0;
		submit_info[0].pSignalSemaphores = NULL;

		VkResult result = vkQueueSubmit(data.pDevice->pGraphicsQueue, 1, submit_info, data.pFence->pFence);
		result = vkQueueWaitIdle(data.pDevice->pGraphicsQueue);
		return result;
	}

	VkResult VulkanUtil::SubmitQueue(VulkanData & data, Semaphore * waitSemaphore, Semaphore * signalSemphore, int commandBufferIndex)
	{
		if (data.pDevice == NULL || waitSemaphore == NULL || data.pFence == NULL)
		{
			return VK_NOT_READY;
		}

	
		VkPipelineStageFlags pipe_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkSubmitInfo submit_info[1] = {};
		submit_info[0].pNext = NULL;
		submit_info[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info[0].waitSemaphoreCount = waitSemaphore == NULL ? 0 : 1;
		submit_info[0].pWaitSemaphores = waitSemaphore == NULL ? NULL : &waitSemaphore->pSemaphore;
		submit_info[0].pWaitDstStageMask = &pipe_stage_flags;
		submit_info[0].commandBufferCount = 1;
		submit_info[0].pCommandBuffers = &data.pDevice->vecCommandBuffer[commandBufferIndex];
		submit_info[0].signalSemaphoreCount = signalSemphore == NULL ? 0 : 1;
		submit_info[0].pSignalSemaphores = signalSemphore == NULL ? NULL :  &signalSemphore->pSemaphore;

		VkResult result = vkQueueSubmit(data.pDevice->pGraphicsQueue, 1, submit_info, NULL);
		result = vkQueueWaitIdle(data.pDevice->pGraphicsQueue);
		return result;
	}

	bool VulkanUtil::IsFinishSubmitQueue(VulkanData & data)
	{
		if (data.pDevice == NULL || data.pFence == NULL)
		{
			return true;
		}

		return vkWaitForFences(data.pDevice->handle, 1, &data.pFence->pFence, VK_TRUE, 100000000) == VK_SUCCESS;
	}

	void VulkanUtil::SetImageLayout(VulkanData & data, VkImage image, VkImageAspectFlags aspectMask, 
									VkImageLayout oldImageLayout, VkImageLayout newImageLayout, int commandBufferIndex)
	{
		if (data.pDevice == NULL || data.pDevice->vecCommandBuffer.size() == 0)
		{
			return;
		}

		VkImageMemoryBarrier imageMemoryBarrier = {};
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.pNext = NULL;
		imageMemoryBarrier.srcAccessMask = 0;
		imageMemoryBarrier.dstAccessMask = 0;
		imageMemoryBarrier.oldLayout = oldImageLayout;
		imageMemoryBarrier.newLayout = newImageLayout;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.image = image;
		imageMemoryBarrier.subresourceRange.aspectMask = aspectMask;
		imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
		imageMemoryBarrier.subresourceRange.levelCount = 1;
		imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
		imageMemoryBarrier.subresourceRange.layerCount = 1;

		if (oldImageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) 
		{
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		}

		if (newImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		}

		if (newImageLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
		{
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		}

		if (oldImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) 
		{
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		}

		if (oldImageLayout == VK_IMAGE_LAYOUT_PREINITIALIZED) 
		{
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		}

		if (newImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		}

		if (newImageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) 
		{
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		}

		if (newImageLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) 
		{
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		}

		VkPipelineStageFlags src_stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		VkPipelineStageFlags dest_stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

		vkCmdPipelineBarrier(data.pDevice->vecCommandBuffer[commandBufferIndex], src_stages, dest_stages, 0, 0, NULL, 0, NULL, 1, &imageMemoryBarrier);
	}

	ResourceBuffer * VulkanUtil::CreateResourceBuffer(VulkanData & data, unsigned int bufferSize, VkBufferUsageFlagBits bufferUsage, VkFlags requirementsMask)
	{
		if (data.pDevice == NULL)
		{
			return NULL;
		}

		ResourceBuffer* pResourceBuffer = new ResourceBuffer();
		pResourceBuffer->pDevice = data.pDevice;

		VkBufferCreateInfo buf_info = {};
		buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buf_info.pNext = NULL;
		buf_info.usage = bufferUsage;
		buf_info.size = bufferSize;
		buf_info.queueFamilyIndexCount = 0;
		buf_info.pQueueFamilyIndices = NULL;
		buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		buf_info.flags = 0;

		VkResult result = vkCreateBuffer(data.pDevice->handle, &buf_info, NULL, &pResourceBuffer->pBuffer);

		VkMemoryRequirements memReqs;
		vkGetBufferMemoryRequirements(pResourceBuffer->pDevice->handle, pResourceBuffer->pBuffer, &memReqs);

		VkMemoryAllocateInfo alloc_info = {};
		alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		alloc_info.pNext = NULL;
		alloc_info.memoryTypeIndex = 0;
		alloc_info.allocationSize = memReqs.size;

		VulkanUtil::GetMemoryTypeFromProperties(data.vecPhysicalDevices[pResourceBuffer->pDevice->physicalDeviceIndex], memReqs.memoryTypeBits,
			requirementsMask, &alloc_info.memoryTypeIndex);


		result = vkAllocateMemory(pResourceBuffer->pDevice->handle, &alloc_info, NULL, &pResourceBuffer->pMem);
		pResourceBuffer->BufferInfo.range = memReqs.size;
		pResourceBuffer->BufferInfo.offset = 0;
		pResourceBuffer->BufferInfo.buffer = pResourceBuffer->pBuffer;

		return pResourceBuffer;
	}

	#define CREATE_SHADER(dev, shader)			\
		if (shader != NULL)						\
		{										\
			if (!createShader(dev, shader))		\
				return false;					\
		}								

	bool VulkanUtil::CreateShader(VulkanData & data, Material * pMaterial, GraphicsPipelineData* pPipelineData, int commandBufferIndex)
	{
		if (data.pDevice == NULL || pMaterial == NULL)
		{
			return false;
		}

		if (pMaterial->GetVertexShader() == NULL && pMaterial->GetFragmentShader() == NULL)
		{
			return false;
		}

		glslang::InitializeProcess();
		
		CREATE_SHADER(data.pDevice, pMaterial->GetVertexShader())
		CREATE_SHADER(data.pDevice, pMaterial->GetFragmentShader())

		updateShaderParameter(pMaterial, data.pDevice, pPipelineData->m_pGraphicsPipeline, commandBufferIndex);

		glslang::FinalizeProcess();
		return true;
	}

	void VulkanUtil::LoadTexture(VulkanData & data, Material * pMaterial, int commandBufferIndex)
	{
		if (data.pDevice == NULL || pMaterial == NULL || pMaterial->GetTextureCount() == 0)
		{
			return;
		}

		const TextureList& textures = pMaterial->GetTextureList();
		TextureList::const_iterator itr = textures.begin();
		TextureList::const_iterator itrEnd = textures.end();

		for (; itr != itrEnd; ++itr)
		{
			Texture* pTexture = *itr;
			if (pTexture != NULL)
			{
				VulkanTexture* pVulkanTexture = static_cast<VulkanTexture*>(pTexture);
				pVulkanTexture->SetCommandBufferIndex(commandBufferIndex);
				pVulkanTexture->Load();
			}
		}
	}

	void VulkanUtil::LoadTextureAndFrameBuffer(VulkanData & data, Texture * pTexture, RenderPass * pRenderPass, int commandBufferIndex)
	{
		if (data.pDevice == NULL || pTexture == NULL)
		{
			return;
		}

		VulkanTexture* pVulkanTexture = static_cast<VulkanTexture*>(pTexture);
		pVulkanTexture->SetCommandBufferIndex(commandBufferIndex);
		pVulkanTexture->Load();
		if (pTexture->IsRenderTarget())
		{
			pVulkanTexture->CreateFrameBuffer(pRenderPass);
		}
	}

	VkResult VulkanUtil::CreateFrameBuffer(VulkanData & data)
	{
		if (data.pDevice == NULL || data.pRenderPass == NULL || data.pSwapchain == NULL || data.pDepthBuffer == NULL || data.pWindow == NULL)
		{
			return VK_NOT_READY;
		}

		if (data.pFrameBuffer != NULL)
		{
			return VK_SUCCESS;
		}

		VkImageView attachments[2];
		attachments[1] = data.pDepthBuffer->pBuffer.pView;

		VkFramebufferCreateInfo fb_info = {};
		fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fb_info.pNext = NULL;
		fb_info.renderPass = data.pRenderPass->pRenderPass;
		fb_info.attachmentCount = 2;
		fb_info.pAttachments = attachments;
		fb_info.width = data.pWindow->Width;
		fb_info.height = data.pWindow->Height;
		fb_info.layers = 1;

		data.pFrameBuffer = new FrameBuffer();
		data.pFrameBuffer->pDevice = data.pDevice;
		data.pFrameBuffer->u32FrameBufferCount = data.pSwapchain->u32SwapchainImageCount;
		data.pFrameBuffer->pFrameBuffer = (VkFramebuffer*)malloc(data.pSwapchain->u32SwapchainImageCount * sizeof(VkFramebuffer));

		VkResult result = VK_SUCCESS;
		for (size_t i = 0; i < data.pFrameBuffer->u32FrameBufferCount; i++)
		{
			attachments[0] = data.pSwapchain->vecBuffers[i].pView;
			result = vkCreateFramebuffer(data.pDevice->handle, &fb_info, NULL, &data.pFrameBuffer->pFrameBuffer[i]);
		}

		return result;
	}

	void VulkanUtil::BindDataVertexBuffer(Mesh * pMesh)
	{
		if (pMesh == NULL)
		{
			return;
		}

		void* pData = pMesh->GetVertexBufferData();
		if (pData == NULL)
		{
			return;
		}

		if (!pMesh->GetDirty())
		{
			return;
		}

		VertexBufferBinding* pBinding = pMesh->GetVertexData()->GetVertexBinding();
		const VertexBufferBinding::VertexBufferBindingMap& bindings = pBinding->GetBindings();
		VertexBufferBinding::VertexBufferBindingMap::const_iterator itr = bindings.begin();
		VertexBufferBinding::VertexBufferBindingMap::const_iterator itrEnd = bindings.end();

		for (; itr != itrEnd; ++itr)
		{
			VertexBuffer* pBuffer = itr->second;
			void* pDest = pBuffer->Lock();
			memcpy(pDest, pData, pBuffer->GetSizeInBytes());
			pBuffer->UnLock();
		}

		pMesh->SetDirty(true);
	}

	void VulkanUtil::BindDataIndexBuffer(SubMesh * pMesh)
	{
		if (pMesh == NULL)
		{
			return;
		}

		IndexData* pIndexData = pMesh->m_pIndexData;
		if (pIndexData == NULL || pIndexData->GetIndexCount() == 0)
		{
			return;
		}

		void* pData = pMesh->m_pIndexBufferData;
		if (pData == NULL)
		{
			return;
		}

		IndexBuffer* pBuffer = pIndexData->GetIndexBuffer();
		void* pDest = pBuffer->Lock();
		memcpy(pDest, pData, pBuffer->GetSizeInBytes());
		pBuffer->UnLock();
	}

	void VulkanUtil::CreateDynamicState(VulkanData & data)
	{
		if (data.dynamicState.dynamicStateCount != 0)
		{
			return;
		}

		memset(data.dynamicStateEnables, 0, sizeof(data.dynamicStateEnables));
		data.dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		data.dynamicState.pNext = NULL;
		data.dynamicState.pDynamicStates = data.dynamicStateEnables;
		data.dynamicState.dynamicStateCount = 0;

		data.vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		data.vp.pNext = NULL;
		data.vp.flags = 0;
		data.vp.viewportCount = 1;
		data.dynamicStateEnables[data.dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
		data.vp.scissorCount = data.vp.viewportCount;
		data.dynamicStateEnables[data.dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;
		data.vp.pViewports = NULL;

		data.ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		data.ms.pNext = NULL;
		data.ms.flags = 0;
		data.ms.pSampleMask = NULL;
		data.ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		data.ms.sampleShadingEnable = VK_FALSE;
		data.ms.alphaToCoverageEnable = VK_FALSE;
		data.ms.alphaToOneEnable = VK_FALSE;
		data.ms.minSampleShading = 0.0;
	}

	VkResult VulkanUtil::CreatePipeline(struct VulkanData& data, Camera* pCamera, Material* pMaterial, SubMesh* pSubMesh, GraphicsPipelineData* pPipelineData, RenderPass* pRenderPass)
	{
		if (pMaterial == NULL || data.pDevice == NULL)
		{
			return VK_NOT_READY;
		}

		RenderPass* pUsedRenderPass = pRenderPass;
		if (pUsedRenderPass == NULL)
		{
			if (pCamera->GetRenderTexture() == NULL)
			{
				pUsedRenderPass = data.pRenderPass;
			}
			else
			{
				pUsedRenderPass = data.mapRenderTexturePass[pCamera];
			}
		}

		if (pUsedRenderPass == NULL)
		{
			return VK_NOT_READY;
		}

		if (pPipelineData->m_pGraphicsPipeline->pPipeline != NULL)
		{
			return VK_SUCCESS;
		}

		VkPipelineVertexInputStateCreateInfo vi;
		memset(&vi, 0, sizeof(vi));
		vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vi.pNext = NULL;
		vi.flags = 0;

		InputAttributeList inputAttrList;
		InputBindingList inputBidingList;

		if (pSubMesh != NULL)
		{
			Mesh* pMesh = pSubMesh->pMesh;

			VertexData* pVertexData = pMesh->GetVertexData();
			if (pVertexData != NULL)
			{
				VertexBufferBinding* pBinding = pVertexData->GetVertexBinding();
				VertexDeclaration* pDecl = pVertexData->GetVertexDeclaration();

				buildInputAttributeDescription(pDecl, inputAttrList, pDecl->GetBindingIndex());
				buildInputBinding(pBinding, inputBidingList);

				vi.vertexBindingDescriptionCount = pBinding->GetBufferCount();
				vi.pVertexBindingDescriptions = &inputBidingList[0];
				vi.vertexAttributeDescriptionCount = pDecl->GetElementCount();
				vi.pVertexAttributeDescriptions = &inputAttrList[0];
			}
		}

		VkPipelineInputAssemblyStateCreateInfo ia;
		ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		ia.pNext = NULL;
		ia.flags = 0;
		ia.primitiveRestartEnable = VK_FALSE;
		ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		VkPipelineRasterizationStateCreateInfo rs;
		rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rs.pNext = NULL;
		rs.flags = 0;
		rs.polygonMode = VK_POLYGON_MODE_FILL;
		rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rs.depthClampEnable = true;
		rs.rasterizerDiscardEnable = VK_FALSE;
		rs.lineWidth = 1.0f;
		rs.cullMode = GetCullModeFlagBits(pMaterial->GetCullMode());

		if (pMaterial->IsEnableDepthBias())
		{
			rs.depthBiasEnable = VK_TRUE;
			rs.depthBiasConstantFactor = pMaterial->GetDepthBiasValue();
			rs.depthBiasClamp = pMaterial->GetDepthBiasClamp();
			rs.depthBiasSlopeFactor = pMaterial->GetDepthBiasSlope();
		}
		else
		{
			rs.depthBiasEnable = VK_FALSE;
			rs.depthBiasConstantFactor = 0;
			rs.depthBiasClamp = 0;
			rs.depthBiasSlopeFactor = 0;
		}

		VkPipelineColorBlendStateCreateInfo cb;
		cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		cb.flags = 0;
		cb.pNext = NULL;
		VkPipelineColorBlendAttachmentState att_state[1];
		att_state[0].colorWriteMask = pMaterial->GetColorMask();
		att_state[0].blendEnable = VK_TRUE;
		att_state[0].colorBlendOp = GetBlendingOperation(pMaterial->GetBlendOperation());
		att_state[0].alphaBlendOp = GetBlendingOperation(pMaterial->GetAlphaBlendOperation());
		att_state[0].srcColorBlendFactor = GetBlendingFactor(pMaterial->GetSourceBlendFactor());
		att_state[0].dstColorBlendFactor = GetBlendingFactor(pMaterial->GetDestBlendFactor());
		att_state[0].srcAlphaBlendFactor = GetBlendingFactor(pMaterial->GetSourceBlendFactorAlpha());
		att_state[0].dstAlphaBlendFactor = GetBlendingFactor(pMaterial->GetDestBlendFactorAlpha());
		cb.attachmentCount = 1;
		cb.pAttachments = att_state;
		cb.logicOpEnable = VK_FALSE;
		cb.logicOp = VK_LOGIC_OP_COPY;
		cb.blendConstants[0] = 0.0f;
		cb.blendConstants[1] = 0.0f;
		cb.blendConstants[2] = 0.0f;
		cb.blendConstants[3] = 0.0f;

		VkPipelineDepthStencilStateCreateInfo ds;
		ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		ds.pNext = NULL;
		ds.flags = 0;
		ds.depthTestEnable = pMaterial->IsEnableDepthTest();
		ds.depthWriteEnable = pMaterial->IsEnableDepthWrite();
		ds.depthCompareOp = GetCompareOperation(pMaterial->GetDepthCompareFunction());
		ds.depthBoundsTestEnable = VK_FALSE;
		ds.stencilTestEnable = VK_FALSE;
		ds.back.failOp = VK_STENCIL_OP_KEEP;
		ds.back.passOp = VK_STENCIL_OP_KEEP;
		ds.back.compareOp = VK_COMPARE_OP_ALWAYS;
		ds.back.compareMask = 0;
		ds.back.reference = 0;
		ds.back.depthFailOp = VK_STENCIL_OP_KEEP;
		ds.back.writeMask = 0;
		ds.minDepthBounds = 0;
		ds.maxDepthBounds = 0;
		ds.front = ds.back;

		VkGraphicsPipelineCreateInfo pipeline;
		pipeline.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline.pNext = NULL;
		pipeline.layout = pPipelineData->m_pGraphicsPipeline->pPipelineLayout;
		pipeline.basePipelineHandle = 0;
		pipeline.basePipelineIndex = 0;
		pipeline.flags = 0;
		pipeline.pVertexInputState = &vi;
		pipeline.pInputAssemblyState = &ia;
		pipeline.pRasterizationState = &rs;
		pipeline.pColorBlendState = &cb;
		pipeline.pTessellationState = NULL;
		pipeline.pMultisampleState = &data.ms;
		pipeline.pDynamicState = &data.dynamicState;
		pipeline.pViewportState = &data.vp;
		pipeline.pDepthStencilState = &ds;
		pipeline.stageCount = 0;
		VkPipelineShaderStageCreateInfo stages[2];

		VkSpecializationInfo vertexSpecInfo = {};
		VkSpecializationInfo fragmentSpecInfo = {};
		std::vector<VkSpecializationMapEntry> vertexSpecMapEntryList;
		std::vector<VkSpecializationMapEntry> fragmentSpecMapEntryList;

		if (pMaterial->GetVertexShader() != NULL)
		{
			VulkanShader* pShader = static_cast<VulkanShader*>(pMaterial->GetVertexShader());
			stages[pipeline.stageCount] = pShader->GetShaderModule()->stShaderState;
			ShaderParametersInt* constantParameters = pShader->GetConstantParameters();
			if (constantParameters->GetParameterCount() > 0)
			{
				
				createShaderConstantParameterInfo(pShader, vertexSpecInfo, vertexSpecMapEntryList);
				stages[pipeline.stageCount].pSpecializationInfo = &vertexSpecInfo;
			}
			pipeline.stageCount++;
		}

		if (pMaterial->GetFragmentShader() != NULL)
		{
			VulkanShader* pShader = static_cast<VulkanShader*>(pMaterial->GetFragmentShader());
			stages[pipeline.stageCount] = pShader->GetShaderModule()->stShaderState;
			ShaderParametersInt* constantParameters = pShader->GetConstantParameters();
			if (constantParameters->GetParameterCount() > 0)
			{
				createShaderConstantParameterInfo(pShader, fragmentSpecInfo, fragmentSpecMapEntryList);
				stages[pipeline.stageCount].pSpecializationInfo = &fragmentSpecInfo;
			}
			pipeline.stageCount++;
		}

		if (pipeline.stageCount == 0)
		{
			pipeline.pStages = NULL;
		}
		else
		{
			pipeline.pStages = stages;
		}
		
		pipeline.renderPass = pUsedRenderPass->pRenderPass;
		pipeline.subpass = 0;

		VkResult result = vkCreateGraphicsPipelines(data.pDevice->handle, NULL, 1, &pipeline, NULL, &pPipelineData->m_pGraphicsPipeline->pPipeline);
		return result;
	}

	VkResult VulkanUtil::CreateSemphore(VulkanData & data, int count)
	{
		if (data.pDevice == NULL)
		{
			return VK_NOT_READY;
		}

		if (data.pSemaphore.size() > 0)
		{
			return VK_SUCCESS;
		}


		for (size_t i = 0; i < count; i++)
		{
			VkSemaphoreCreateInfo imageAcquiredSemaphoreCreateInfo;
			imageAcquiredSemaphoreCreateInfo.sType =
				VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			imageAcquiredSemaphoreCreateInfo.pNext = NULL;
			imageAcquiredSemaphoreCreateInfo.flags = 0;

			VkSemaphore semaphore;
			VkResult result = vkCreateSemaphore(data.pDevice->handle, &imageAcquiredSemaphoreCreateInfo, NULL, &semaphore);
			if (result != VK_SUCCESS)
			{
				return result;
			}

			Semaphore* pSemaphore = new Semaphore();
			pSemaphore->pDevice = data.pDevice;
			pSemaphore->pSemaphore = semaphore;
			data.pSemaphore.push_back(pSemaphore);
		}

		return VK_SUCCESS;
	}

	VkResult VulkanUtil::CreateFence(VulkanData & data)
	{
		if (data.pDevice == NULL)
		{
			return VK_NOT_READY;
		}

		if (data.pFence != NULL)
		{
			return VK_SUCCESS;
		}

		data.pFence = new Fence();
		data.pFence->pDevice = data.pDevice;

		VkFenceCreateInfo fenceInfo;
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.pNext = NULL;
		fenceInfo.flags = 0;
		return vkCreateFence(data.pDevice->handle, &fenceInfo, NULL, &data.pFence->pFence);
	}

	void VulkanUtil::CreateCompositors(VulkanData & data)
	{
		if (data.pWindow == NULL || data.pDevice == NULL)
		{
			return;
		}

		if (CompositorManager::GetSingleton().GetResourceCount() == 0)
		{
			return;
		}

		if (data.pCompositor != NULL)
		{
			return;
		}

		// Create a separate render pass for the offscreen rendering as it may differ from the one used for scene rendering
		VkAttachmentDescription attachment[2];
		attachment[0].format = data.pWindow->format;
		attachment[0].samples = VK_SAMPLE_COUNT_1_BIT;
		attachment[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachment[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachment[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachment[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		attachment[1].format = VK_FORMAT_D16_UNORM;
		attachment[1].samples = VK_SAMPLE_COUNT_1_BIT;
		attachment[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachment[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachment[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachment[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachment[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorReference = {};
		colorReference.attachment = 0;
		colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthReference = {};
		depthReference.attachment = 1;
		depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.flags = 0;
		subpass.inputAttachmentCount = 0;
		subpass.pInputAttachments = NULL;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorReference;
		subpass.pResolveAttachments = NULL;
		subpass.pDepthStencilAttachment = &depthReference;
		subpass.preserveAttachmentCount = 0;
		subpass.pPreserveAttachments = NULL;

		// Use subpass dependencies for layout transitions
		VkSubpassDependency dependencies[2];

		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		//Create Render Pass
		data.pCompositor = new CompositorData();
		data.pCompositor->pDevice = data.pDevice;
		data.pCompositor->pRenderPass = new RenderPass();
		data.pCompositor->pRenderPass->pDevice = data.pDevice;

		VkRenderPassCreateInfo rp_info = {};
		rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		rp_info.pNext = NULL;
		rp_info.attachmentCount = 2;
		rp_info.pAttachments = attachment;
		rp_info.subpassCount = 1;
		rp_info.pSubpasses = &subpass;
		rp_info.dependencyCount = 2;
		rp_info.pDependencies = dependencies;

		VkResult result = vkCreateRenderPass(data.pDevice->handle, &rp_info, NULL, &data.pCompositor->pRenderPass->pRenderPass);
		if (result != VK_SUCCESS)
		{
			printf("Create render pass fail in the createCompositors");
			return;
		}

		VkSamplerCreateInfo sampler_info = {};
		sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler_info.pNext = NULL;
		sampler_info.magFilter = VK_FILTER_LINEAR;
		sampler_info.minFilter = VK_FILTER_LINEAR;
		sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler_info.mipLodBias = 0.0f;
		sampler_info.maxAnisotropy = 0;
		sampler_info.minLod = 0.0f;
		sampler_info.maxLod = 1.0f;
		sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		result = vkCreateSampler(data.pDevice->handle, &sampler_info, NULL, &data.pCompositor->pSampler);
		if (result != VK_SUCCESS)
		{
			printf("Craete sampler fail in the createCompositors");
			return;
		}

		data.pCompositor->pSemaphore = new Semaphore();
		data.pCompositor->pSemaphore->pDevice = data.pDevice;
		VkSemaphoreCreateInfo imageAcquiredSemaphoreCreateInfo;
		imageAcquiredSemaphoreCreateInfo.sType =
			VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		imageAcquiredSemaphoreCreateInfo.pNext = NULL;
		imageAcquiredSemaphoreCreateInfo.flags = 0;

		vkCreateSemaphore(data.pDevice->handle, &imageAcquiredSemaphoreCreateInfo, NULL, &data.pCompositor->pSemaphore->pSemaphore);
		bool isFirst = true;

		const ResourceManager::ResourceMap resources = CompositorManager::GetSingleton().GetAllResources();
		ResourceManager::ResourceMap::const_iterator rItr = resources.begin();
		ResourceManager::ResourceMap::const_iterator rItrEnd = resources.end();
		for (; rItr != rItrEnd; ++rItr)
		{
			Compositor* pCompositor = static_cast<Compositor*>(rItr->second);
			const CompositorPassList& listPass = pCompositor->GetPassList();

			if (listPass.size() == 0)
			{
				continue;
			}

			if (pCompositor->GetCamera() == NULL)
			{
				continue;
			}

			if (data.pCompositor->listPassData.find(pCompositor->GetCamera()) == data.pCompositor->listPassData.end())
			{
				data.pCompositor->listPassData.insert(std::pair<Camera*, std::vector<CompositorPassData*>>(pCompositor->GetCamera(), std::vector<CompositorPassData*>()));
			}

			CompositorPassData* pCompositorPassData = new CompositorPassData();
			data.pCompositor->listPassData[pCompositor->GetCamera()].push_back(pCompositorPassData);

			CompositorPassList::const_iterator pItr = listPass.begin();
			CompositorPassList::const_iterator pItrEnd = listPass.end();
			for (; pItr != pItrEnd; ++pItr)
			{
				CompositorFrameBuffer* pFrameBuffer = new CompositorFrameBuffer();
				pFrameBuffer->pDevice = data.pDevice;
				pFrameBuffer->u32FrameBufferCount = 1;
				pFrameBuffer->iWidth = pItr->iWidth;
				pFrameBuffer->iHeight = pItr->iHeight;
				pCompositorPassData->listFrameBuffer.push_back(pFrameBuffer);

				if (pItr->eMode == CTM_TEXTURE)
				{
					LoadTextureAndFrameBuffer(data, pItr->pTexture, data.pCompositor->pRenderPass, 0);
					VulkanTexture* pVulkanTexture = static_cast<VulkanTexture*>(pItr->pTexture);
					pFrameBuffer->pFrameBuffer = &pVulkanTexture->GetTextureImage()->pFrameBuffer;
					pFrameBuffer->iTextureWidth = pVulkanTexture->GetWidth();
					pFrameBuffer->iTextureHeight = pVulkanTexture->GetHeight();
				}

				Material* pMaterial = pItr->pMaterial;
				pFrameBuffer->pMaterial = pMaterial;
				pFrameBuffer->pGraphicsPipeline = new GraphicsPipelineData();
				CreatePipelineLayout(data, pMaterial, pFrameBuffer->pGraphicsPipeline);

				if (isFirst)
				{
					pFrameBuffer->pSemaphore = data.pCompositor->pSemaphore;
					isFirst = false;
				}
				else
				{
					pFrameBuffer->pSemaphore = new Semaphore();
					pFrameBuffer->pSemaphore->pDevice = data.pDevice;
					vkCreateSemaphore(data.pDevice->handle, &imageAcquiredSemaphoreCreateInfo, NULL, &pFrameBuffer->pSemaphore->pSemaphore);
				}
			}
		}
	}

	void VulkanUtil::CreateCompositorShaders(VulkanData & data, Camera* pCamera, int commandBufferIndex)
	{
		if (data.pCompositor == NULL)
		{
			return;
		}

		if (data.pCompositor->listPassData.find(pCamera) != data.pCompositor->listPassData.end())
		{
			std::vector<CompositorPassData*>::iterator cItr = data.pCompositor->listPassData[pCamera].begin();
			std::vector<CompositorPassData*>::iterator cItrEnd = data.pCompositor->listPassData[pCamera].end();

			for (; cItr != cItrEnd; ++cItr)
			{
				CompositorPassData* pCompositor = *cItr;
				if (!pCompositor->isInit)
				{
					std::vector<CompositorFrameBuffer*>::iterator fItr = pCompositor->listFrameBuffer.begin();
					std::vector<CompositorFrameBuffer*>::iterator fItrEnd = pCompositor->listFrameBuffer.end();
					for (; fItr != fItrEnd; ++fItr)
					{
						CompositorFrameBuffer* pFrameBuffer = *fItr;
						Material* pMaterial = pFrameBuffer->pMaterial;
						CreateShader(data, pMaterial, pFrameBuffer->pGraphicsPipeline, commandBufferIndex);
						LoadTexture(data, pMaterial, commandBufferIndex);
						CreatePipeline(data, pCamera, pMaterial, NULL, pFrameBuffer->pGraphicsPipeline, data.pCompositor->pRenderPass);

						pFrameBuffer->pPilelineLayout = new PipelineLayout();
						CreateDescriptPool(data, pFrameBuffer->pPilelineLayout, pMaterial);

						Shader* pVertexShader = pMaterial->GetVertexShader();
						if (pVertexShader->GetConstantParameters()->GetParameterCount() > 0)
						{
							pFrameBuffer->pVertexUniformBuffer = new ResourceBuffer();
							pFrameBuffer->pVertexUniformBuffer->pDevice = data.pDevice;
							updateUniformBuffer(data, NULL, NULL, pFrameBuffer->pVertexUniformBuffer, pVertexShader->GetUniformShaderParameters(), false);
						}

						Shader* pFragmentShader = pMaterial->GetFragmentShader();
						if (pFragmentShader->GetConstantParameters()->GetParameterCount() > 0)
						{
							pFrameBuffer->pFragmentUniformBuffer = new ResourceBuffer();
							pFrameBuffer->pFragmentUniformBuffer->pDevice = data.pDevice;
							updateUniformBuffer(data, NULL, NULL, pFrameBuffer->pFragmentUniformBuffer, pFragmentShader->GetUniformShaderParameters(), false);
						}
					}
					pCompositor->isInit = true;
				}
			}
		}
	}

	void VulkanUtil::DrawCompositors(VulkanData & data, Camera* pCamera, int commandBufferIndex)
	{
		if (data.pCompositor == NULL)
		{
			return;
		}

		if (data.pCompositor->listPassData.find(pCamera) == data.pCompositor->listPassData.end())
		{
			return;
		}
	
		VkRenderPassBeginInfo rpBeginInfo = {};
		rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		rpBeginInfo.pNext = NULL;
		rpBeginInfo.renderPass = data.pCompositor->pRenderPass->pRenderPass;
		rpBeginInfo.framebuffer = data.pFrameBuffer->pFrameBuffer[data.pSwapchain->u32CurrentBuffer];
		rpBeginInfo.renderArea.extent.width = data.pWindow->Width;
		rpBeginInfo.renderArea.extent.height = data.pWindow->Height;
		rpBeginInfo.clearValueCount = pCamera->GetClearCameraList().size();
		rpBeginInfo.pClearValues = pCamera->GetRenderTexture() == NULL ? data.pRenderPass->clearValue : data.mapRenderTexturePass[pCamera]->clearValue;

		VkCommandBuffer commandBuffer = data.pDevice->vecCommandBuffer[commandBufferIndex];
		//vkCmdBeginRenderPass(commandBuffer, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		std::vector<CompositorPassData*>::iterator cItr = data.pCompositor->listPassData[pCamera].begin();
		std::vector<CompositorPassData*>::iterator cItrEnd = data.pCompositor->listPassData[pCamera].end();
		ResourceBuffer* pBuffers[2];
		Semaphore* pLastSemphore = NULL;
		bool isFirst = true;

		for (; cItr != cItrEnd; ++cItr)
		{
			CompositorPassData* pCompositor = *cItr;
			std::vector<CompositorFrameBuffer*>::iterator fItr = pCompositor->listFrameBuffer.begin();
			std::vector<CompositorFrameBuffer*>::iterator fItrEnd = pCompositor->listFrameBuffer.end();
			for (; fItr != fItrEnd; ++fItr)
			{
				CompositorFrameBuffer* pFrameBuffer = *fItr;
				
				if (!isFirst)
				{
					vkCmdEndRenderPass(commandBuffer);
					VulkanUtil::EndCommandBuffer(data, commandBufferIndex);
					VulkanUtil::SubmitQueue(data, pLastSemphore, pFrameBuffer->pSemaphore, commandBufferIndex);
				}

				pBuffers[0] = pFrameBuffer->pVertexUniformBuffer;
				pBuffers[1] = pFrameBuffer->pFragmentUniformBuffer;
				UpdateDescriptSet(data, pBuffers, pFrameBuffer->pPilelineLayout, pFrameBuffer->pMaterial, pFrameBuffer->pGraphicsPipeline);
				
				if (pFrameBuffer->pFrameBuffer == NULL)
				{
					//rpBeginInfo.renderPass = data.pCompositor->pRenderPass->pRenderPass;
					rpBeginInfo.framebuffer = data.pFrameBuffer->pFrameBuffer[data.pSwapchain->u32CurrentBuffer];
					rpBeginInfo.renderArea.extent.width = data.pWindow->Width;
					rpBeginInfo.renderArea.extent.height = data.pWindow->Height;
				}
				else
				{
					//rpBeginInfo.renderPass = data.pRenderPass->pRenderPass;
					rpBeginInfo.framebuffer = pFrameBuffer->pFrameBuffer[0];
					rpBeginInfo.renderArea.extent.width = pFrameBuffer->iTextureWidth;
					rpBeginInfo.renderArea.extent.height = pFrameBuffer->iTextureHeight;
				}
				pLastSemphore = pFrameBuffer->pSemaphore;
				VulkanUtil::BeingCommandBuffer(data, commandBufferIndex);
				vkCmdBeginRenderPass(commandBuffer, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
				BindViewport(data, pFrameBuffer->iWidth, pFrameBuffer->iHeight, commandBufferIndex);

				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pFrameBuffer->pGraphicsPipeline->m_pGraphicsPipeline->pPipelineLayout, 
										0, pFrameBuffer->pPilelineLayout->vecDescriptorSet.size(), pFrameBuffer->pPilelineLayout->vecDescriptorSet.data(), 0, NULL);
				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pFrameBuffer->pGraphicsPipeline->m_pGraphicsPipeline->pPipeline);
				vkCmdDraw(commandBuffer, 3, 1, 0, 0);
				//vkCmdEndRenderPass(commandBuffer);

				isFirst = false;
			}
		}

		vkCmdEndRenderPass(commandBuffer);
		VulkanUtil::EndCommandBuffer(data, commandBufferIndex);
		VulkanUtil::SubmitQueue(data, pLastSemphore, NULL, commandBufferIndex);
	}

	bool VulkanUtil::glslToSPV(const VkShaderStageFlagBits shaderType, const char * pShader, std::vector<unsigned int>& spirv)
	{
#ifndef ANDROID
		EShLanguage stage = FindLanguage(shaderType);
		glslang::TShader shader(stage);
		glslang::TProgram program;
		const char* shaderStrings[1];
		TBuiltInResource resources;
		initShaderResource(resources);

		//Enable SPIR-V and Vulkan rules when parsing GLSL
		EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);
		shaderStrings[0] = pShader;
		shader.setStrings(shaderStrings, 1);

		if (!shader.parse(&resources, 100, false, messages))
		{
			puts(shader.getInfoLog());
			puts(shader.getInfoDebugLog());
			return false;
		}

		program.addShader(&shader);

		if (!program.link(messages))
		{
			puts(shader.getInfoLog());
			puts(shader.getInfoDebugLog());
			return false;
		}

		glslang::GlslangToSpv(*program.getIntermediate(stage), spirv);
#else
#endif

		return true;
	}

	VkResult VulkanUtil::createDevice(struct VulkanData& data, VkQueueFlagBits flag, int physicalDeviceIndex, uint32_t queueFamilyIndex)
	{
		if (physicalDeviceIndex < 0 || physicalDeviceIndex >= data.vecPhysicalDevices.size())
		{
			return VK_NOT_READY;
		}

		PhysicalDevice* physicalDevice = data.vecPhysicalDevices[physicalDeviceIndex];

		Device* device = new Device();
		device->physicalDeviceIndex = physicalDeviceIndex;
		device->eQueueFlagBits = flag;
		device->u32QueueFamilyIndex = queueFamilyIndex;

		float queue_priorities[1] = { 0.0 };
		VkDeviceQueueCreateInfo queue_info = {};
		queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		queue_info.pNext = 0;
		queue_info.queueCount = 1;
		queue_info.pQueuePriorities = queue_priorities;
		queue_info.queueFamilyIndex = device->u32QueueFamilyIndex;

		VkDeviceCreateInfo device_info = {};
		device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		device_info.pNext = NULL;
		device_info.queueCreateInfoCount = 1;
		device_info.pQueueCreateInfos = &queue_info;
		device_info.enabledExtensionCount = data.vecDeviceExtensionName.size();
		device_info.ppEnabledExtensionNames = device_info.enabledExtensionCount > 0 ? data.vecDeviceExtensionName.data() : NULL;
		device_info.enabledLayerCount = data.vecDeviceLayerName.size();
		device_info.ppEnabledLayerNames = device_info.enabledLayerCount > 0 ? data.vecDeviceLayerName.data() : NULL;
		device_info.pEnabledFeatures = NULL;

		VkResult result = vkCreateDevice(physicalDevice->handle, &device_info, NULL, &device->handle);
		if (result == VK_SUCCESS)
		{
			data.pDevice = device;
			vkGetDeviceQueue(device->handle, queueFamilyIndex, 0, &device->pGraphicsQueue);
		}

		return result;
	}

	bool VulkanUtil::GetMemoryTypeFromProperties(PhysicalDevice * pPhysicaldevice, uint32_t typeBits, VkFlags requirementsMask, uint32_t * typeIndex)
	{
		for (size_t i = 0; i < pPhysicaldevice->memProps.memoryTypeCount; i++)
		{
			if ((typeBits & 1) == 1)
			{
				if ((pPhysicaldevice->memProps.memoryTypes[i].propertyFlags & requirementsMask) == requirementsMask)
				{
					*typeIndex = i;
					return true;
				}
			}

			typeBits >>= 1;
		}

		return false;
	}

	int VulkanUtil::getDescriptorSetLayoutBindingCount(Material * pMaterial)
	{
		if (pMaterial == NULL)
		{
			return 0;
		}

		int bindingCount = 0;
		bindingCount = pMaterial->GetVertexShader()->GetUniformShaderParameters()->GetParameterCount() > 0 ? bindingCount + 1 : bindingCount;
		bindingCount = pMaterial->GetFragmentShader()->GetUniformShaderParameters()->GetParameterCount() > 0 ? bindingCount + 1 : bindingCount;
		bindingCount += pMaterial->GetTextureCount();

		return bindingCount;
	}

	EShLanguage VulkanUtil::FindLanguage(const VkShaderStageFlagBits shaderType)
	{
		switch (shaderType) 
		{
		case VK_SHADER_STAGE_VERTEX_BIT:
			return EShLangVertex;

		case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
			return EShLangTessControl;

		case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
			return EShLangTessEvaluation;

		case VK_SHADER_STAGE_GEOMETRY_BIT:
			return EShLangGeometry;

		case VK_SHADER_STAGE_FRAGMENT_BIT:
			return EShLangFragment;

		case VK_SHADER_STAGE_COMPUTE_BIT:
			return EShLangCompute;

		default:
			return EShLangVertex;
		}
	}

	void VulkanUtil::initShaderResource(TBuiltInResource& resources)
	{
		resources.maxLights = 32;
		resources.maxClipPlanes = 6;
		resources.maxTextureUnits = 32;
		resources.maxTextureCoords = 32;
		resources.maxVertexAttribs = 64;
		resources.maxVertexUniformComponents = 4096;
		resources.maxVaryingFloats = 64;
		resources.maxVertexTextureImageUnits = 32;
		resources.maxCombinedTextureImageUnits = 80;
		resources.maxTextureImageUnits = 32;
		resources.maxFragmentUniformComponents = 4096;
		resources.maxDrawBuffers = 32;
		resources.maxVertexUniformVectors = 128;
		resources.maxVaryingVectors = 8;
		resources.maxFragmentUniformVectors = 16;
		resources.maxVertexOutputVectors = 16;
		resources.maxFragmentInputVectors = 15;
		resources.minProgramTexelOffset = -8;
		resources.maxProgramTexelOffset = 7;
		resources.maxClipDistances = 8;
		resources.maxComputeWorkGroupCountX = 65535;
		resources.maxComputeWorkGroupCountY = 65535;
		resources.maxComputeWorkGroupCountZ = 65535;
		resources.maxComputeWorkGroupSizeX = 1024;
		resources.maxComputeWorkGroupSizeY = 1024;
		resources.maxComputeWorkGroupSizeZ = 64;
		resources.maxComputeUniformComponents = 1024;
		resources.maxComputeTextureImageUnits = 16;
		resources.maxComputeImageUniforms = 8;
		resources.maxComputeAtomicCounters = 8;
		resources.maxComputeAtomicCounterBuffers = 1;
		resources.maxVaryingComponents = 60;
		resources.maxVertexOutputComponents = 64;
		resources.maxGeometryInputComponents = 64;
		resources.maxGeometryOutputComponents = 128;
		resources.maxFragmentInputComponents = 128;
		resources.maxImageUnits = 8;
		resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
		resources.maxCombinedShaderOutputResources = 8;
		resources.maxImageSamples = 0;
		resources.maxVertexImageUniforms = 0;
		resources.maxTessControlImageUniforms = 0;
		resources.maxTessEvaluationImageUniforms = 0;
		resources.maxGeometryImageUniforms = 0;
		resources.maxFragmentImageUniforms = 8;
		resources.maxCombinedImageUniforms = 8;
		resources.maxGeometryTextureImageUnits = 16;
		resources.maxGeometryOutputVertices = 256;
		resources.maxGeometryTotalOutputComponents = 1024;
		resources.maxGeometryUniformComponents = 1024;
		resources.maxGeometryVaryingComponents = 64;
		resources.maxTessControlInputComponents = 128;
		resources.maxTessControlOutputComponents = 128;
		resources.maxTessControlTextureImageUnits = 16;
		resources.maxTessControlUniformComponents = 1024;
		resources.maxTessControlTotalOutputComponents = 4096;
		resources.maxTessEvaluationInputComponents = 128;
		resources.maxTessEvaluationOutputComponents = 128;
		resources.maxTessEvaluationTextureImageUnits = 16;
		resources.maxTessEvaluationUniformComponents = 1024;
		resources.maxTessPatchComponents = 120;
		resources.maxPatchVertices = 32;
		resources.maxTessGenLevel = 64;
		resources.maxViewports = 16;
		resources.maxVertexAtomicCounters = 0;
		resources.maxTessControlAtomicCounters = 0;
		resources.maxTessEvaluationAtomicCounters = 0;
		resources.maxGeometryAtomicCounters = 0;
		resources.maxFragmentAtomicCounters = 8;
		resources.maxCombinedAtomicCounters = 8;
		resources.maxAtomicCounterBindings = 1;
		resources.maxVertexAtomicCounterBuffers = 0;
		resources.maxTessControlAtomicCounterBuffers = 0;
		resources.maxTessEvaluationAtomicCounterBuffers = 0;
		resources.maxGeometryAtomicCounterBuffers = 0;
		resources.maxFragmentAtomicCounterBuffers = 1;
		resources.maxCombinedAtomicCounterBuffers = 1;
		resources.maxAtomicCounterBufferSize = 16384;
		resources.maxTransformFeedbackBuffers = 4;
		resources.maxTransformFeedbackInterleavedComponents = 64;
		resources.maxCullDistances = 8;
		resources.maxCombinedClipAndCullDistances = 8;
		resources.maxSamples = 4;
		resources.limits.nonInductiveForLoops = 1;
		resources.limits.whileLoops = 1;
		resources.limits.doWhileLoops = 1;
		resources.limits.generalUniformIndexing = 1;
		resources.limits.generalAttributeMatrixVectorIndexing = 1;
		resources.limits.generalVaryingIndexing = 1;
		resources.limits.generalSamplerIndexing = 1;
		resources.limits.generalVariableIndexing = 1;
		resources.limits.generalConstantMatrixVectorIndexing = 1;
	}

	bool VulkanUtil::createShader(Device * pDevice, Shader * pShader)
	{
		VulkanShader* pVulkanShader = static_cast<VulkanShader*>(pShader);
		if (pVulkanShader->GetShaderModule() != NULL)
		{
			return true;
		}

		ShaderModule* pModule = new ShaderModule();
		pVulkanShader->SetShaderModule(pModule);
		pModule->pDevice = pDevice;

		std::vector<unsigned int> vtxSPV;

		pModule->stShaderState.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		pModule->stShaderState.pNext = NULL;
		pModule->stShaderState.pSpecializationInfo = NULL;
		pModule->stShaderState.flags = 0;
		pModule->stShaderState.stage = getShaderStageFlagBits(pShader->GetShaderType());
		pModule->stShaderState.pName = pShader->GetEntryName().c_str();

		if (!glslToSPV(pModule->stShaderState.stage, pShader->GetShaderText().c_str(), vtxSPV))
		{
			return false;
		}

		VkShaderModuleCreateInfo module_info;
		module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		module_info.pNext = NULL;
		module_info.flags = 0;
		module_info.codeSize = vtxSPV.size() * sizeof(unsigned int);
		module_info.pCode = vtxSPV.data();

		if (vkCreateShaderModule(pDevice->handle, &module_info, NULL, &pModule->stShaderState.module) != VK_SUCCESS)
		{
			return false;
		}

		return true;
	}

	void VulkanUtil::createShaderParameter(Material* pMaterial, std::vector<VkPushConstantRange>& constants)
	{
		constants.clear();
		if (pMaterial == NULL)
		{
			return;
		}

		Shader* pVertexShader = pMaterial->GetVertexShader();
		uint32_t offset = 0;
		if (pVertexShader != NULL)
		{
			ShaderParameters* vertexParam = pVertexShader->GetShaderParameters();
			float paramSize = vertexParam->GetShaderConstants().size() * sizeof(float);
			if (paramSize > 0)
			{
				VkPushConstantRange constRange;
				constRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
				constRange.offset = 0;
				constRange.size = paramSize;
				constants.push_back(constRange);
				offset += paramSize;
			}
		}

		Shader* pFragmentShader = pMaterial->GetFragmentShader();
		if (pFragmentShader != NULL)
		{
			ShaderParameters* fragParam = pFragmentShader->GetShaderParameters();
			float paramSize = fragParam->GetShaderConstants().size() * sizeof(float);
			if (paramSize > 0)
			{
				VkPushConstantRange constRange;
				constRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
				constRange.offset = offset;
				constRange.size = paramSize;
				constants.push_back(constRange);
			}
		}
	}

	void VulkanUtil::updateShaderParameter(Material * pMaterial, Device * pDevice, GraphicsPipeline * pPipeline, int commandBufferIndex)
	{
		Shader* pVertexShader = pMaterial->GetVertexShader();
		uint32_t offset = 0;
		ShaderParameters* vertexParams = pVertexShader->GetShaderParameters();
		float vertexParamSize = sizeof(float) * vertexParams->GetShaderConstants().size();
		
		const LightList& lights = GameObjectManager::GetSingleton().GetLightList();
		if (lights.size() > 0)
		{
			Light* pLight = lights[0];
			vertexParams->SetParameter("LightPosition", pLight->GetGameObject()->GetTransform()->GetPosition());
			vertexParams->SetParameter("LightDirection", pLight->GetDirection());
			vertexParams->SetParameter("LightColor", pLight->GetDiffuseColor());
			vertexParams->SetParameter("LightIntensity", pLight->GetIntensity());
		}

		if (vertexParamSize > 0)
		{
			vkCmdPushConstants(pDevice->vecCommandBuffer[commandBufferIndex], pPipeline->pPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT,
				0, vertexParamSize, vertexParams->GetShaderConstants().data());
			offset += vertexParamSize;
		}

		Shader* pFragShader = pMaterial->GetFragmentShader();
		ShaderParameters* fragParams = pFragShader->GetShaderParameters();
		float fragParamSize = sizeof(float) * fragParams->GetShaderConstants().size();
		if (fragParamSize > 0)
		{
			vkCmdPushConstants(pDevice->vecCommandBuffer[commandBufferIndex], pPipeline->pPipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT,
				offset, fragParamSize, fragParams->GetShaderConstants().data());
		}
	}

	VkShaderStageFlagBits VulkanUtil::getShaderStageFlagBits(ShaderType type)
	{
		switch (type)
		{
		case Vulkan::VERTEX:
			return VK_SHADER_STAGE_VERTEX_BIT;
		case Vulkan::FRAGMENT:
			return VK_SHADER_STAGE_FRAGMENT_BIT;
		default:
			return VK_SHADER_STAGE_VERTEX_BIT;
		}
	}

	VkCullModeFlagBits VulkanUtil::GetCullModeFlagBits(CullMode mode)
	{
		switch (mode)
		{
		case Vulkan::CM_FLAG_NONE:
			return VK_CULL_MODE_NONE;
		case Vulkan::CM_FLAG_FRONT:
			return VK_CULL_MODE_FRONT_BIT;
		case Vulkan::CM_FLAG_BACK:
			return VK_CULL_MODE_BACK_BIT;
		case Vulkan::CM_FLAG_FRONT_AND_BACK:
			return VK_CULL_MODE_FRONT_AND_BACK;
		}

		return VK_CULL_MODE_BACK_BIT;
	}

	VkBlendOp VulkanUtil::GetBlendingOperation(SceneBlendOperation operation)
	{
		switch (operation)
		{
		case Vulkan::SBO_ADD:
			return VK_BLEND_OP_ADD;
		case Vulkan::SBO_SUBTRACT:
			return VK_BLEND_OP_SUBTRACT;
		case Vulkan::SBO_REVERSE_SUBTRACT:
			return VK_BLEND_OP_REVERSE_SUBTRACT;
		case Vulkan::SBO_MIN:
			return VK_BLEND_OP_MIN;
		case Vulkan::SBO_MAX:
			return VK_BLEND_OP_MAX;
		}

		return VK_BLEND_OP_ADD;
	}

	VkBlendFactor VulkanUtil::GetBlendingFactor(SceneBlendFactor factor)
	{
		switch (factor)
		{
		case Vulkan::SBF_ZERO:
			return VK_BLEND_FACTOR_ZERO;
		case Vulkan::SBF_ONE:
			return VK_BLEND_FACTOR_ONE;
		case Vulkan::SBF_SRC_COLOUR:
			return VK_BLEND_FACTOR_SRC_COLOR;
		case Vulkan::SBF_DST_COLOUR:
			return VK_BLEND_FACTOR_DST_COLOR;
		case Vulkan::SBF_ONE_MINUS_SRC_COLOUR:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
		case Vulkan::SBF_ONE_MINUS_DST_COLOUR:
			return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
		case Vulkan::SBF_SRC_ALPHA:
			return VK_BLEND_FACTOR_SRC_ALPHA;
		case Vulkan::SBF_DST_ALPHA:
			return VK_BLEND_FACTOR_DST_ALPHA;
		case Vulkan::SBF_ONE_MINUS_SRC_ALPHA:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		case Vulkan::SBF_ONE_MINUS_DST_ALPHA:
			return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
		}

		return VK_BLEND_FACTOR_ZERO;
	}

	VkCompareOp VulkanUtil::GetCompareOperation(DepthCompareFunction func)
	{
		switch (func)
		{
		case Vulkan::DCF_ALWAYS_FAIL:
			return VK_COMPARE_OP_NEVER;
		case Vulkan::DCF_ALWAYS_PASS:
			return VK_COMPARE_OP_ALWAYS;
		case Vulkan::DCF_LESS:
			return VK_COMPARE_OP_LESS;
		case Vulkan::DCF_LESS_EQUAL:
			return VK_COMPARE_OP_LESS_OR_EQUAL;
		case Vulkan::DCF_EQUAL:
			return VK_COMPARE_OP_EQUAL;
		case Vulkan::DCF_NOT_EQUAL:
			return VK_COMPARE_OP_NOT_EQUAL;
		case Vulkan::DCF_GREATER_EQUAL:
			return VK_COMPARE_OP_GREATER_OR_EQUAL;
		case Vulkan::DCF_GREATER:
			return VK_COMPARE_OP_GREATER;
		}

		return VK_COMPARE_OP_LESS_OR_EQUAL;
	}

	void VulkanUtil::buildInputAttributeDescription(VertexDeclaration * pDecl, InputAttributeList& inputAttrList, unsigned short bindingIndex)
	{
		const VertexDeclaration::VertexElementList elements = pDecl->GetElements();
		int offset = 0;
		int index = 0;
		VertexDeclaration::VertexElementList::const_iterator itr = elements.begin();
		VertexDeclaration::VertexElementList::const_iterator itrEnd = elements.end();

		for (; itr != itrEnd; ++itr, ++index)
		{
			VkVertexInputAttributeDescription desc;
			desc.binding = bindingIndex;
			desc.location = index;
			desc.format = transformFormat(itr->GetVertexElementType());
			desc.offset = offset;
			offset += itr->GetTypeSize(itr->GetVertexElementType());
			inputAttrList.push_back(desc);
		}
	}

	VkFormat VulkanUtil::transformFormat(VertexElementType eType)
	{
		switch (eType)
		{
		case Vulkan::VET_FLOAT1:
		case Vulkan::VET_DOUBLE1:
			return VK_FORMAT_R32_SFLOAT;
		case Vulkan::VET_FLOAT2:
		case Vulkan::VET_DOUBLE2:
			return VK_FORMAT_R32G32_SFLOAT;
		case Vulkan::VET_FLOAT3:
		case Vulkan::VET_DOUBLE3:
			return VK_FORMAT_R32G32B32_SFLOAT;
		case Vulkan::VET_FLOAT4:
		case Vulkan::VET_DOUBLE4:
		case Vulkan::VET_COLOUR:
		case Vulkan::VET_COLOUR_ARGB:
		case Vulkan::VET_COLOUR_ABGR:
			return VK_FORMAT_R32G32B32A32_SFLOAT;
		case Vulkan::VET_SHORT1:
			return VK_FORMAT_R16_SINT;
		case Vulkan::VET_SHORT2:
			return VK_FORMAT_R16G16_SINT;
		case Vulkan::VET_SHORT3:
			return VK_FORMAT_R16G16B16_SINT;
		case Vulkan::VET_SHORT4:
			return VK_FORMAT_R16G16B16A16_SINT;
		case Vulkan::VET_UBYTE4:
			return VK_FORMAT_R8_UINT;
		case Vulkan::VET_USHORT1:
			return VK_FORMAT_R16_UINT;
		case Vulkan::VET_USHORT2:
			return VK_FORMAT_R16G16_UINT;
		case Vulkan::VET_USHORT3:
			return VK_FORMAT_R16G16B16_UINT;
		case Vulkan::VET_USHORT4:
			return VK_FORMAT_R16G16B16A16_UINT;
		case Vulkan::VET_INT1:
			return VK_FORMAT_R32_SINT;
		case Vulkan::VET_INT2:
			return VK_FORMAT_R32G32_SINT;
		case Vulkan::VET_INT3:
			return VK_FORMAT_R32G32B32_SINT;
		case Vulkan::VET_INT4:
			return VK_FORMAT_R32G32B32A32_SINT;
		case Vulkan::VET_UINT1:
			return VK_FORMAT_R32_UINT;
		case Vulkan::VET_UINT2:
			return VK_FORMAT_R32G32_UINT;
		case Vulkan::VET_UINT3:
			return VK_FORMAT_R32G32B32_UINT;
		case Vulkan::VET_UINT4:
			return VK_FORMAT_R32G32B32A32_UINT;
		default:
			break;
		}

		return VK_FORMAT_R32G32B32A32_SFLOAT;
	}

	void VulkanUtil::buildInputBinding(VertexBufferBinding * pBinding, InputBindingList & bindingList)
	{
		const VertexBufferBinding::VertexBufferBindingMap bindings = pBinding->GetBindings();
		VertexBufferBinding::VertexBufferBindingMap::const_iterator itr = bindings.begin();
		VertexBufferBinding::VertexBufferBindingMap::const_iterator itrEnd = bindings.end();
		for (; itr != itrEnd; itr++)
		{
			VkVertexInputBindingDescription desc;
			desc.binding = itr->first;
			desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			desc.stride = itr->second->GetVertexSize();
			bindingList.push_back(desc);
		}
	}

	VkImage VulkanUtil::CreateVkImage(VkDevice device, VkImageType imageType, VkFormat format, uint32_t width, uint32_t height, VkImageUsageFlags usageBits, VkImageTiling tiling)
	{
		VkImageCreateInfo image_info = {};
		image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_info.pNext = NULL;
		image_info.imageType = imageType;
		image_info.format = format;
		image_info.extent.width = width;
		image_info.extent.height = height;
		image_info.extent.depth = 1;
		image_info.mipLevels = 1;
		image_info.arrayLayers = 1;
		image_info.samples = VK_SAMPLE_COUNT_1_BIT;
		image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		image_info.queueFamilyIndexCount = 0;
		image_info.pQueueFamilyIndices = NULL;
		image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		// We will sample directly from the color attachment
		image_info.usage = usageBits;
		image_info.tiling = tiling;
		image_info.flags = 0;

		VkImage image;
		VkResult result = vkCreateImage(device, &image_info, NULL, &image);

		return image;
	}

	VkImageView VulkanUtil::CreateVkImageView(VkDevice device, VkImageViewType viewType, VkFormat format, VkImageAspectFlags aspactMask, VkImage image)
	{
		VkImageViewCreateInfo imageView = {};
		imageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageView.pNext = NULL;
		imageView.format = format;
		imageView.components.r = VK_COMPONENT_SWIZZLE_R;
		imageView.components.g = VK_COMPONENT_SWIZZLE_G;
		imageView.components.b = VK_COMPONENT_SWIZZLE_B;
		imageView.components.a = VK_COMPONENT_SWIZZLE_A;
		imageView.subresourceRange.aspectMask = aspactMask;
		imageView.subresourceRange.baseMipLevel = 0;
		imageView.subresourceRange.levelCount = 1;
		imageView.subresourceRange.baseArrayLayer = 0;
		imageView.subresourceRange.layerCount = 1;
		imageView.viewType = viewType;
		imageView.flags = 0;
		imageView.image = image;

		VkImageView view;
		VkResult result = vkCreateImageView(device, &imageView, NULL, &view);

		return view;
	}

	void VulkanUtil::updateUniformBuffer(struct VulkanData& data, Transform* pTransform, Camera* pCamera, ResourceBuffer* pBuffer, ShaderParameters* pParam, bool isCreated)
	{
		VkResult result = VK_SUCCESS;
		VkMemoryRequirements memReqs;
		if (!isCreated)
		{
			VkBufferCreateInfo buf_info = {};
			buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			buf_info.pNext = NULL;
			buf_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			buf_info.size = pParam->GetShaderConstants().size() * sizeof(float);
			buf_info.queueFamilyIndexCount = 0;
			buf_info.pQueueFamilyIndices = NULL;
			buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			buf_info.flags = 0;
			result = vkCreateBuffer(data.pDevice->handle, &buf_info, NULL, &pBuffer->pBuffer);

			vkGetBufferMemoryRequirements(data.pDevice->handle, pBuffer->pBuffer, &memReqs);

			VkMemoryAllocateInfo alloc_info = {};
			alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			alloc_info.pNext = NULL;
			alloc_info.memoryTypeIndex = 0;
			alloc_info.allocationSize = memReqs.size;

			GetMemoryTypeFromProperties(data.vecPhysicalDevices[data.pDevice->physicalDeviceIndex], memReqs.memoryTypeBits,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				&alloc_info.memoryTypeIndex);

			result = vkAllocateMemory(data.pDevice->handle, &alloc_info, NULL, &pBuffer->pMem);

			pBuffer->BufferInfo.buffer = pBuffer->pBuffer;
			pBuffer->BufferInfo.offset = 0;
			pBuffer->BufferInfo.range = buf_info.size;

			result = vkBindBufferMemory(data.pDevice->handle, pBuffer->pBuffer, pBuffer->pMem, 0);
		}
		else
		{
			vkGetBufferMemoryRequirements(data.pDevice->handle, pBuffer->pBuffer, &memReqs);
		}

		if (pTransform != NULL)
		{
			pParam->SetParameter("localToWorld", pTransform->GetTransformMatrix());
		}
		
		if (pCamera != NULL)
		{
			pParam->SetParameter("viewMatrix", pCamera->GetViewMatrix());
			pParam->SetParameter("projMatrix", pCamera->GetProjectionMatrix());
			pParam->SetParameter("camWorldPos", pCamera->GetGameObject()->GetTransform()->GetPosition());
		}

		void* pData = NULL;
		result = vkMapMemory(data.pDevice->handle, pBuffer->pMem, 0, memReqs.size, 0, &pData);
		memcpy(pData, pParam->GetShaderConstants().data(), pParam->GetShaderConstants().size() * sizeof(float));
		vkUnmapMemory(data.pDevice->handle, pBuffer->pMem);
	}

	void VulkanUtil::createShaderConstantParameterInfo(Shader * pShader, VkSpecializationInfo & specInfo, std::vector<VkSpecializationMapEntry>& specMapEntryList)
	{
		specMapEntryList.clear();
		ShaderParametersInt* constantParameters = pShader->GetConstantParameters();
		if (constantParameters->GetParameterCount() > 0)
		{
			const ShaderParametersInt::ShaderParameterMap& shaderParams = constantParameters->GetAllShaderParameters();
			ShaderParametersInt::ShaderParameterMap::const_iterator itr = shaderParams.begin();
			ShaderParametersInt::ShaderParameterMap::const_iterator itrEnd = shaderParams.end();
			for (; itr != itrEnd; ++itr)
			{
				VkSpecializationMapEntry entry = {};
				entry.constantID = itr->first;
				entry.offset = itr->second.physicalIndex;
				entry.size = itr->second.elementSize;
				specMapEntryList.push_back(entry);
			}

			specInfo.mapEntryCount = specMapEntryList.size();
			specInfo.pMapEntries = specMapEntryList.data();
			specInfo.dataSize = constantParameters->GetShaderConstants().size() * sizeof(float);
			specInfo.pData = constantParameters->GetShaderConstants().data();
		}
	}

	void VulkanUtil::createClearValue(Camera * pCamera, RenderPass * pRenderPass)
	{
		const Camera::CameraClearList& cameraClearList = pCamera->GetClearCameraList();
		pRenderPass->clearValue = new VkClearValue[cameraClearList.size()];
		for (size_t i = 0; i < cameraClearList.size(); i++)
		{
			if (cameraClearList[i].isColor)
			{
				memcpy(pRenderPass->clearValue[i].color.float32, cameraClearList[i].clearData.color, sizeof(float) * 4);
			}
			else
			{
				pRenderPass->clearValue[i].depthStencil.depth = cameraClearList[i].clearData.depthStencil.depth;
				pRenderPass->clearValue[i].depthStencil.stencil = cameraClearList[i].clearData.depthStencil.stencil;
			}
		}
	}

	void VulkanUtil::createRenderPass(struct VulkanData& data, RenderPass * pRenderPass, bool isClear, VkImageLayout finalLayout)
	{
		VkAttachmentDescription attachments[2];
		attachments[0].format = data.pWindow->format;
		attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[0].loadOp = isClear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		attachments[0].finalLayout = finalLayout;
		attachments[0].flags = 0;

		attachments[1].format = VK_FORMAT_D16_UNORM;
		attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[1].loadOp = isClear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachments[1].flags = 0;

		VkAttachmentReference color_reference = {};
		color_reference.attachment = 0;
		color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depth_refernece = {};
		depth_refernece.attachment = 1;
		depth_refernece.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.flags = 0;
		subpass.inputAttachmentCount = 0;
		subpass.pInputAttachments = NULL;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_reference;
		subpass.pResolveAttachments = NULL;
		subpass.pDepthStencilAttachment = &depth_refernece;
		subpass.preserveAttachmentCount = 0;
		subpass.pPreserveAttachments = NULL;

		VkRenderPassCreateInfo rp_info = {};
		rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		rp_info.pNext = NULL;
		rp_info.attachmentCount = 2;
		rp_info.pAttachments = attachments;
		rp_info.subpassCount = 1;
		rp_info.pSubpasses = &subpass;
		rp_info.dependencyCount = 0;
		rp_info.pDependencies = NULL;

		pRenderPass->pDevice = data.pDevice;
		vkCreateRenderPass(data.pDevice->handle, &rp_info, NULL, &pRenderPass->pRenderPass);
	}

	VkResult VulkanUtil::CreateCommandPool(VulkanData & data)
	{
		if (data.pDevice == NULL)
		{
			return VK_NOT_READY;
		}

		Device* device = data.pDevice;
		VkCommandPoolCreateInfo cmd_Pool_info = {};
		cmd_Pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmd_Pool_info.pNext = NULL;
		cmd_Pool_info.queueFamilyIndex = device->u32QueueFamilyIndex;
		cmd_Pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		return vkCreateCommandPool(device->handle, &cmd_Pool_info, NULL, &device->pCommandPool);
	}

	VkResult VulkanUtil::CreateCommandBuffer(VulkanData & data, int commandBufferCount, VkCommandBufferLevel level)
	{
		if (data.pDevice == NULL)
		{
			return VK_NOT_READY;
		}

		Device* device = data.pDevice;

		if (device->pCommandPool == NULL)
		{
			return VK_NOT_READY;
		}

		if (device->vecCommandBuffer.size() > 0)
		{
			return VK_SUCCESS;
		}

		VkCommandBufferAllocateInfo cmd = {};
		cmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmd.pNext = NULL;
		cmd.commandPool = device->pCommandPool;
		cmd.level = level;
		cmd.commandBufferCount = commandBufferCount;

		device->vecCommandBuffer.resize(commandBufferCount);
		return vkAllocateCommandBuffers(device->handle, &cmd, device->vecCommandBuffer.data());
	}

	VkResult VulkanUtil::BeingCommandBuffer(VulkanData & data, int commandBufferIndex)
	{
		if (data.pDevice == NULL)
		{
			return VK_NOT_READY;
		}

		vkResetCommandBuffer(data.pDevice->vecCommandBuffer[commandBufferIndex], 0);

		VkCommandBufferBeginInfo cmd_buf_info = {};
		cmd_buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmd_buf_info.pNext = NULL;
		cmd_buf_info.flags = 0;
		cmd_buf_info.pInheritanceInfo = NULL;

		return vkBeginCommandBuffer(data.pDevice->vecCommandBuffer[commandBufferIndex], &cmd_buf_info);
	}

	VkResult VulkanUtil::EndCommandBuffer(VulkanData & data, int commandBufferIndex)
	{
		if (data.pDevice == NULL)
		{
			return VK_NOT_READY;
		}

		return vkEndCommandBuffer(data.pDevice->vecCommandBuffer[commandBufferIndex]);
	}

	void VulkanUtil::EndSingleTimeCommandBuffer(VulkanData & data, int commandBufferIndex)
	{
		VkCommandBuffer commandBuffer = data.pDevice->vecCommandBuffer[commandBufferIndex];
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(data.pDevice->pGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(data.pDevice->pGraphicsQueue);

		vkResetCommandBuffer(commandBuffer, 0);
		BeingCommandBuffer(data, commandBufferIndex);
	}

	void VulkanUtil::CreateConnection(VulkanData & data)
	{
#ifndef Windows

#endif
	}

#ifdef Windows
	static void run(struct sample_info *info)
	{
		/* Placeholder for samples that want to show dynamic content */
	}

	// MS-Windows event handling function:
	LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		struct sample_info *info = reinterpret_cast<struct sample_info *>(
			GetWindowLongPtr(hWnd, GWLP_USERDATA));

		switch (uMsg) {
		case WM_CLOSE:
			PostQuitMessage(0);
			break;
		case WM_PAINT:
			run(info);
			return 0;
		default:
			break;
		}
		return (DefWindowProc(hWnd, uMsg, wParam, lParam));
	}
#endif

	void VulkanUtil::CreateDisplayWindow(VulkanData & data, const char * windowName, int width, int height)
	{
		if (width <= 0 || height <= 0)
		{
			return;
		}

		if (data.pWindow == NULL)
		{
			data.pWindow = new Window();
			data.pWindow->Width = width;
			data.pWindow->Height = height;

			glfwInit();
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
			data.pWindow->pWindow = glfwCreateWindow(width, height, windowName, nullptr, nullptr);

			data.pWindow->hWnd = glfwGetWin32Window(data.pWindow->pWindow);
			data.pWindow->connection = GetModuleHandle(NULL);
		}
	}

	bool VulkanUtil::OnWindowDrawing(VulkanData & data)
	{
		if (data.pWindow == NULL)
		{
			return false;
		}

		if (!glfwWindowShouldClose(data.pWindow->pWindow))
		{
			glfwPollEvents();
			return true;
		}

		return false;
	}

	VkResult VulkanUtil::CreateSurface(VulkanData & data, int physicalDeviceIndex)
	{
		if (data.pWindow == NULL ||
			physicalDeviceIndex < 0 || physicalDeviceIndex >= data.vecPhysicalDevices.size())
		{
			return VK_NOT_READY;
		}

#ifdef Windows
		VkWin32SurfaceCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		createInfo.pNext = NULL;
		createInfo.hinstance = data.pWindow->connection;
		createInfo.hwnd = data.pWindow->hWnd;
#endif
		VkResult result = vkCreateWin32SurfaceKHR(data.pInstance, &createInfo, NULL, &data.pWindow->pSurface);
		if (result == VK_SUCCESS)
		{
			data.pWindow->pInstance = data.pInstance;
			PhysicalDevice* device = data.vecPhysicalDevices[physicalDeviceIndex];

			VkBool32* pSupportsPresnet = (VkBool32*)malloc(device->queueFamilyPropertyCount * sizeof(VkBool32));
			for (size_t i = 0; i < device->queueFamilyPropertyCount; i++)
			{
				vkGetPhysicalDeviceSurfaceSupportKHR(device->handle, i, data.pWindow->pSurface, &pSupportsPresnet[i]);
			}

			// Search for a graphics and a present queue in the array of queue
			// families, try to find one that supports both
			data.u32graphicsQueueFamilyIndex = UINT32_MAX;
			data.u32PresentQueueFamilyIndex = UINT32_MAX;
			for (size_t i = 0; i < device->queueFamilyPropertyCount; i++)
			{
				if ((device->vecQueueFamilyProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
				{
					if (data.u32graphicsQueueFamilyIndex == UINT32_MAX)
					{
						data.u32graphicsQueueFamilyIndex = i;
					}

					if (pSupportsPresnet[i] == VK_TRUE)
					{
						data.u32graphicsQueueFamilyIndex = i;
						data.u32PresentQueueFamilyIndex = i;
						break;
					}
				}
			}

			if (data.u32PresentQueueFamilyIndex == UINT32_MAX)
			{
				for (size_t i = 0; i < device->queueFamilyPropertyCount; i++)
				{
					// If didn't find a queue that supports both graphics and present, then
					// find a separate present queue.
					if (pSupportsPresnet[i] == VK_TRUE)
					{
						data.u32PresentQueueFamilyIndex = i;
						break;
					}
				}
			}
			free(pSupportsPresnet);

			if (data.u32graphicsQueueFamilyIndex == UINT32_MAX || data.u32PresentQueueFamilyIndex == UINT32_MAX)
			{
				return VK_NOT_READY;
			}

			uint32_t formatCount = 0;
			result = vkGetPhysicalDeviceSurfaceFormatsKHR(device->handle, data.pWindow->pSurface, &formatCount, NULL);
			VkSurfaceFormatKHR* surfaceFormats = (VkSurfaceFormatKHR*)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
			result = vkGetPhysicalDeviceSurfaceFormatsKHR(device->handle, data.pWindow->pSurface, &formatCount, surfaceFormats);

			if (formatCount == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED)
			{
				data.pWindow->format = VK_FORMAT_B8G8R8A8_UNORM;
			}
			else
			{
				data.pWindow->format = surfaceFormats[0].format;
			}

			free(surfaceFormats);

			return result;
		}
		return VK_NOT_READY;
	}

	VkResult VulkanUtil::CreateSwapchain(VulkanData & data, VkImageUsageFlags usageFlags, int physicalDeviceIndex)
	{
		if (physicalDeviceIndex < 0 || physicalDeviceIndex >= data.vecPhysicalDevices.size() || data.pWindow == NULL || data.pWindow->pSurface == NULL ||
			data.pDevice == NULL)
		{
			return VK_NOT_READY;
		}

		if (data.pSwapchain != NULL)
		{
			return VK_SUCCESS;
		}

		PhysicalDevice* physicalDevice = data.vecPhysicalDevices[physicalDeviceIndex];

		VkSurfaceCapabilitiesKHR surfCapabilities;
		VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice->handle, data.pWindow->pSurface, &surfCapabilities);

		uint32_t presentModeCount;
		result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice->handle, data.pWindow->pSurface, &presentModeCount, NULL);
		VkPresentModeKHR* presentModes = (VkPresentModeKHR*)malloc(presentModeCount * sizeof(VkPresentModeKHR));
		result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice->handle, data.pWindow->pSurface, &presentModeCount, presentModes);

		VkExtent2D swapChainExtent;
		if (surfCapabilities.currentExtent.width == 0xFFFFFFFF)
		{
			swapChainExtent.width = data.pWindow->Width;
			swapChainExtent.height = data.pWindow->Height;
			if (swapChainExtent.width < surfCapabilities.minImageExtent.width)
				swapChainExtent.width = surfCapabilities.minImageExtent.width;
			else if (swapChainExtent.width > surfCapabilities.maxImageExtent.width)
				swapChainExtent.width = surfCapabilities.maxImageExtent.width;

			if (swapChainExtent.height < surfCapabilities.minImageExtent.height)
				swapChainExtent.height = surfCapabilities.maxImageExtent.height;
			else if (swapChainExtent.height > surfCapabilities.maxImageExtent.height)
				swapChainExtent.height = surfCapabilities.maxImageExtent.height;
		}
		else
		{
			swapChainExtent = surfCapabilities.currentExtent;
		}

		// If mailbox mode is available, use it, as is the lowest-latency non-
		// tearing mode.  If not, try IMMEDIATE which will usually be available,
		// and is fastest (though it tears).  If not, fall back to FIFO which is
		// always available.
		VkPresentModeKHR swapchainPresetMode = VK_PRESENT_MODE_FIFO_KHR;
		for (size_t i = 0; i < presentModeCount; i++)
		{
			if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				swapchainPresetMode = VK_PRESENT_MODE_MAILBOX_KHR;
				break;
			}

			if (swapchainPresetMode != VK_PRESENT_MODE_MAILBOX_KHR && presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
			{
				swapchainPresetMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
			}
		}

		// Determine the number of VkImage's to use in the swap chain.
		// We need to acquire only 1 presentable image at at time.
		// Asking for minImageCount images ensures that we can acquire
		// 1 presentable image as long as we present it before attempting
		// to acquire another.
		uint32_t desiredNumberOfSwapChainImages = surfCapabilities.minImageCount;

		VkSurfaceTransformFlagBitsKHR preTransform;
		if (surfCapabilities.supportedTransforms && VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
		{
			preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		}
		else
		{
			preTransform = surfCapabilities.currentTransform;
		}

		VkSwapchainCreateInfoKHR swapchain_ci = {};
		swapchain_ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchain_ci.pNext = NULL;
		swapchain_ci.surface = data.pWindow->pSurface;
		swapchain_ci.minImageCount = desiredNumberOfSwapChainImages;
		swapchain_ci.imageFormat = data.pWindow->format;
		swapchain_ci.imageExtent.width = swapChainExtent.width;
		swapchain_ci.imageExtent.height = swapChainExtent.height;
		swapchain_ci.preTransform = preTransform;
		swapchain_ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchain_ci.imageArrayLayers = 1;
		swapchain_ci.presentMode = swapchainPresetMode;
		swapchain_ci.oldSwapchain = VK_NULL_HANDLE;
		swapchain_ci.clipped = true;
		swapchain_ci.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
		swapchain_ci.imageUsage = usageFlags;
		swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchain_ci.queueFamilyIndexCount = 0;
		swapchain_ci.pQueueFamilyIndices = NULL;
		uint32_t queueFamilyIndices[2] = { data.u32graphicsQueueFamilyIndex, data.u32PresentQueueFamilyIndex };
		if (data.u32graphicsQueueFamilyIndex != data.u32PresentQueueFamilyIndex)
		{
			// If the graphics and present queues are from different queue families,
			// we either have to explicitly transfer ownership of images between the
			// queues, or we have to create the swapchain with imageSharingMode
			// as VK_SHARING_MODE_CONCURRENT
			swapchain_ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			swapchain_ci.queueFamilyIndexCount = 2;
			swapchain_ci.pQueueFamilyIndices = queueFamilyIndices;
		}

		data.pSwapchain = new Swapchain();
		data.pSwapchain->pDevice = data.pDevice;

		VkSamplerCreateInfo sampler_info = {};
		sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler_info.pNext = NULL;
		sampler_info.magFilter = VK_FILTER_LINEAR;
		sampler_info.minFilter = VK_FILTER_LINEAR;
		sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler_info.mipLodBias = 0.0f;
		sampler_info.maxAnisotropy = 0;
		sampler_info.minLod = 0.0f;
		sampler_info.maxLod = 1.0f;
		sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		result = vkCreateSampler(data.pDevice->handle, &sampler_info, NULL, &data.pSwapchain->pSampler);

		result = vkCreateSwapchainKHR(data.pDevice->handle, &swapchain_ci, NULL, &data.pSwapchain->handle);

		result = vkGetSwapchainImagesKHR(data.pDevice->handle, data.pSwapchain->handle, &data.pSwapchain->u32SwapchainImageCount, NULL);

		VkImage* swapchainImages = (VkImage*)malloc(data.pSwapchain->u32SwapchainImageCount * sizeof(VkImage));
		result = vkGetSwapchainImagesKHR(data.pDevice->handle, data.pSwapchain->handle, &data.pSwapchain->u32SwapchainImageCount, swapchainImages);

		for (size_t i = 0; i < data.pSwapchain->u32SwapchainImageCount; i++)
		{
			ImageViewBuffer buffer;

			buffer.pImage = swapchainImages[i];
			buffer.pView = CreateVkImageView(data.pDevice->handle, VK_IMAGE_VIEW_TYPE_2D, data.pWindow->format, VK_IMAGE_ASPECT_COLOR_BIT, buffer.pImage);
			buffer.imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			buffer.imageInfo.imageView = buffer.pView;
			buffer.imageInfo.sampler = data.pSwapchain->pSampler;
			data.pSwapchain->vecBuffers.push_back(buffer);
		}

		//free(swapchainImages);
		if (presentModes != NULL)
		{
			free(presentModes);
		}

		return result;
	}

	VkResult VulkanUtil::AttachToSwapchain(VulkanData & data, int cameraIndex, int commandBufferIndex)
	{
		if (data.pDevice == NULL || data.pSwapchain == NULL || data.pSemaphore.size() == 0)
		{
			return VK_NOT_READY;
		}

		uint32_t prevBuffer = data.pSwapchain->u32CurrentBuffer;

		VkResult result = vkAcquireNextImageKHR(data.pDevice->handle, data.pSwapchain->handle, UINT64_MAX, data.pSemaphore[cameraIndex]->pSemaphore,
												VK_NULL_HANDLE, &data.pSwapchain->u32CurrentBuffer);

		if (prevBuffer != data.pSwapchain->u32CurrentBuffer)
		{
			SetImageLayout(data, data.pSwapchain->vecBuffers[data.pSwapchain->u32CurrentBuffer].pImage, VK_IMAGE_ASPECT_COLOR_BIT,
				VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, commandBufferIndex);
		}

		return result;
	}

	VkResult VulkanUtil::PresentSwapchainToDisplay(VulkanData & data)
	{
		if (data.pSwapchain == NULL)
		{
			return VK_NOT_READY;
		}

		VkPresentInfoKHR present;
		present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present.pNext = NULL;
		present.swapchainCount = 1;
		present.pSwapchains = &data.pSwapchain->handle;
		present.pImageIndices = &data.pSwapchain->u32CurrentBuffer;
		present.pWaitSemaphores = NULL;
		present.waitSemaphoreCount = 0;
		present.pResults = NULL;

		return vkQueuePresentKHR(data.pDevice->pGraphicsQueue, &present);
	}

	VkResult VulkanUtil::CreateDepthBuffer(VulkanData & data, int commandBufferIndex)
	{
		if (data.pWindow == NULL || data.pDevice == NULL)
		{
			return VK_NOT_READY;
		}

		if (data.pDepthBuffer != NULL)
		{
			return VK_SUCCESS;
		}

		VkImageCreateInfo image_info = {};
		image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_info.pNext = NULL;
		image_info.imageType = VK_IMAGE_TYPE_2D;
		image_info.format = VK_FORMAT_D16_UNORM;
		image_info.extent.width = data.pWindow->Width;
		image_info.extent.height = data.pWindow->Height;
		image_info.extent.depth = 1;
		image_info.mipLevels = 1;
		image_info.arrayLayers = 1;
		image_info.samples = VK_SAMPLE_COUNT_1_BIT;
		image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		image_info.queueFamilyIndexCount = 0;
		image_info.pQueueFamilyIndices = NULL;
		image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		image_info.flags = 0;

		PhysicalDevice* pPhysicalDevice = data.vecPhysicalDevices[data.pDevice->physicalDeviceIndex];
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(pPhysicalDevice->handle, image_info.format, &props);
		if (props.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			image_info.tiling = VK_IMAGE_TILING_LINEAR;
		}
		else if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
		}
		else
		{
			return VK_NOT_READY;
		}

		data.pDepthBuffer = new DepthBuffer();
		data.pDepthBuffer->pDevice = data.pDevice;
		VkResult result = vkCreateImage(data.pDevice->handle, &image_info, NULL, &data.pDepthBuffer->pBuffer.pImage);

		VkMemoryRequirements memReqs;
		vkGetImageMemoryRequirements(data.pDevice->handle, data.pDepthBuffer->pBuffer.pImage, &memReqs);

		VkMemoryAllocateInfo mem_alloc = {};
		mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		mem_alloc.pNext = NULL;
		mem_alloc.allocationSize = memReqs.size;
		GetMemoryTypeFromProperties(pPhysicalDevice, memReqs.memoryTypeBits, 0, &mem_alloc.memoryTypeIndex);

		result = vkAllocateMemory(data.pDevice->handle, &mem_alloc, NULL, &data.pDepthBuffer->pMem);
		result = vkBindImageMemory(data.pDevice->handle, data.pDepthBuffer->pBuffer.pImage, data.pDepthBuffer->pMem, 0);

		data.pDepthBuffer->pBuffer.pView = CreateVkImageView(data.pDevice->handle, VK_IMAGE_VIEW_TYPE_2D, 
									image_info.format, VK_IMAGE_ASPECT_DEPTH_BIT, data.pDepthBuffer->pBuffer.pImage);

		SetImageLayout(data, data.pDepthBuffer->pBuffer.pImage, VK_IMAGE_ASPECT_DEPTH_BIT,
					   VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, commandBufferIndex);

		return result;
	}

#define GET_UNIFORM_BUFFER(name, pVulkanGo, data, shader)	\
	bool is##name##Created = pVulkanGo->Get##name##UniformBuffer(shader) != NULL;	\
	ResourceBuffer* name##Buffer = is##name##Created ? pVulkanGo->Get##name##UniformBuffer(shader) : new ResourceBuffer();	\
	name##Buffer->pDevice = data.pDevice;	\
	if (!is##name##Created)	\
		pVulkanGo->Set##name##UniformBuffer(shader, name##Buffer);

	VkResult VulkanUtil::CreateUniformBuffer(struct VulkanData& data, GameObject* pGameObject, Material* pMaterial, Camera * camera, bool isModifyCamera)
	{
		if (pGameObject == NULL || camera == NULL || data.pDevice == NULL)
		{
			return VK_NOT_READY;
		}

		VulkanGameObject* pVulkanGo = static_cast<VulkanGameObject*>(pGameObject);
		Transform* node = pGameObject->GetTransform();

		if (node->GetIsUpdateTransform() == false && isModifyCamera == false)
		{
			return VK_SUCCESS;
		}

		Shader* pVertexShader = pMaterial->GetVertexShader();
		ShaderParameters* pVertexUniformParam = pVertexShader->GetUniformShaderParameters();
		Shader* pFragmentShader = pMaterial->GetFragmentShader();
		ShaderParameters* pFragmentUniformParam = pFragmentShader->GetUniformShaderParameters();

		if (pVertexUniformParam->GetParameterCount() != 0)
		{
			GET_UNIFORM_BUFFER(Vertex, pVulkanGo, data, pVertexShader);
			updateUniformBuffer(data, pVulkanGo->GetTransform(), camera, VertexBuffer, pVertexUniformParam, isVertexCreated);
		}
		
		if (pFragmentUniformParam->GetParameterCount() != 0)
		{
			GET_UNIFORM_BUFFER(Fragment, pVulkanGo, data, pFragmentShader);
			updateUniformBuffer(data, pVulkanGo->GetTransform(), camera, FragmentBuffer, pFragmentUniformParam, isFragmentCreated);
		}

		node->SetIsUpdateTransform(false);

		return VK_SUCCESS;
	}
	VkResult VulkanUtil::CreatePipelineLayout(VulkanData & data, Material* pMaterial, GraphicsPipelineData* pPipelineData)
	{
		if (pMaterial == NULL || data.pDevice == NULL)
		{
			return VK_NOT_READY;
		}

		if (pPipelineData->m_pGraphicsPipeline->pPipelineLayout != NULL)
		{
			return VK_SUCCESS;
		}

		int bindingCount = getDescriptorSetLayoutBindingCount(pMaterial);
	
		VkDescriptorSetLayoutBinding* layoutBinding = new VkDescriptorSetLayoutBinding[bindingCount];
		uint32_t bindingIndex = 0;

		if (pMaterial->GetVertexShader()->GetUniformShaderParameters()->GetParameterCount() > 0)
		{
			layoutBinding[bindingIndex].binding = bindingIndex;
			layoutBinding[bindingIndex].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			layoutBinding[bindingIndex].descriptorCount = 1;
			layoutBinding[bindingIndex].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
			layoutBinding[bindingIndex].pImmutableSamplers = NULL;
			bindingIndex++;
		}

		if (pMaterial->GetFragmentShader()->GetUniformShaderParameters()->GetParameterCount() > 0)
		{
			layoutBinding[bindingIndex].binding = bindingIndex;
			layoutBinding[bindingIndex].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			layoutBinding[bindingIndex].descriptorCount = 1;
			layoutBinding[bindingIndex].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			layoutBinding[bindingIndex].pImmutableSamplers = NULL;
			bindingIndex++;
		}	

		for (size_t i = bindingIndex; i < bindingCount; i++)
		{
			//Texture
			layoutBinding[i].binding = i;
			layoutBinding[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			layoutBinding[i].descriptorCount = 1;
			layoutBinding[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			layoutBinding[i].pImmutableSamplers = NULL;
		}

		VkDescriptorSetLayoutCreateInfo descriptor_layout = {};
		descriptor_layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptor_layout.pNext = NULL;
		descriptor_layout.bindingCount = bindingCount;
		descriptor_layout.pBindings = layoutBinding;

		pPipelineData->m_pGraphicsPipeline->pDevice = data.pDevice;
		pPipelineData->m_pGraphicsPipeline->vecDescLayout.resize(1);
		
		VkResult result = vkCreateDescriptorSetLayout(data.pDevice->handle, &descriptor_layout, NULL, pPipelineData->m_pGraphicsPipeline->vecDescLayout.data());

		std::vector<VkPushConstantRange> constants;
		createShaderParameter(pMaterial, constants);

		VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
		pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pPipelineLayoutCreateInfo.pNext = NULL;
		pPipelineLayoutCreateInfo.pushConstantRangeCount = constants.size();
		pPipelineLayoutCreateInfo.pPushConstantRanges = constants.size() > 0 ? constants.data() : NULL;
		pPipelineLayoutCreateInfo.setLayoutCount = 1;
		pPipelineLayoutCreateInfo.pSetLayouts = pPipelineData->m_pGraphicsPipeline->vecDescLayout.data();

		result = vkCreatePipelineLayout(data.pDevice->handle, &pPipelineLayoutCreateInfo, NULL, &pPipelineData->m_pGraphicsPipeline->pPipelineLayout);
		delete[] layoutBinding;
		return result;
	}
	VkResult VulkanUtil::CreateDescriptPool(VulkanData & data, PipelineLayout* pipelineLayout, Material* pMaterial)
	{
		if (pipelineLayout == NULL || pMaterial == NULL)
		{
			return VK_NOT_READY;
		}

		if (pipelineLayout->pDescriptorPool != NULL)
		{
			return VK_SUCCESS;
		}

		pipelineLayout->pDevice = data.pDevice;

		int bindingCount = getDescriptorSetLayoutBindingCount(pMaterial);
		VkDescriptorPoolSize* poolSize = new VkDescriptorPoolSize[bindingCount];
		uint32_t bindingIndex = 0;

		if (pMaterial->GetVertexShader()->GetUniformShaderParameters()->GetParameterCount() > 0)
		{
			poolSize[bindingIndex].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSize[bindingIndex].descriptorCount = 1;
			bindingIndex++;
		}
		
		if (pMaterial->GetFragmentShader()->GetUniformShaderParameters()->GetParameterCount() > 0)
		{
			poolSize[bindingIndex].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSize[bindingIndex].descriptorCount = 1;
			bindingIndex++;
		}

		for (size_t i = bindingIndex; i < bindingCount; i++)
		{
			poolSize[i].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			poolSize[i].descriptorCount = 1;
		}

		VkDescriptorPoolCreateInfo descriptor_pool = {};
		descriptor_pool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptor_pool.pNext = NULL;
		descriptor_pool.maxSets = 1;
		descriptor_pool.poolSizeCount = bindingCount;
		descriptor_pool.pPoolSizes = poolSize;

		VkResult result = vkCreateDescriptorPool(pipelineLayout->pDevice->handle, &descriptor_pool, NULL, &pipelineLayout->pDescriptorPool);
		delete[] poolSize;

		return result;
	}

#define WRITE_DESCRIPTOR_SET(name, uniformBuffer, pMaterial, writes, pipelineLayout, bindingIndex)	\
	if (pMaterial->Get##name##Shader()->GetUniformShaderParameters()->GetParameterCount() > 0)	\
	{								\
		writes[bindingIndex] = {};	\
		writes[bindingIndex].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;	\
		writes[bindingIndex].dstSet = pipelineLayout->vecDescriptorSet[0];		\
		writes[bindingIndex].descriptorCount = 1;		\
		writes[bindingIndex].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;	\
		writes[bindingIndex].pBufferInfo = &uniformBuffer->BufferInfo;	\
		writes[bindingIndex].dstArrayElement = 0;	\
		writes[bindingIndex].dstBinding = bindingIndex;	\
		++bindingIndex;		\
	}

	VkResult VulkanUtil::UpdateDescriptSet(VulkanData & data, GameObject* pGo, PipelineLayout* pipelineLayout, 
		Material* pMaterial, GraphicsPipelineData* pPipelineData)
	{
		if (pipelineLayout == NULL || pPipelineData == NULL || pGo == NULL)
		{
			return VK_NOT_READY;
		}

		VulkanGameObject* pVulkanGo = static_cast<VulkanGameObject*>(pGo);
		ResourceBuffer* buffers[2];
		buffers[0] = pVulkanGo->GetVertexUniformBuffer(pMaterial->GetVertexShader());
		buffers[1] = pVulkanGo->GetFragmentUniformBuffer(pMaterial->GetFragmentShader());
	
		return UpdateDescriptSet(data, buffers, pipelineLayout, pMaterial, pPipelineData);
	}

	VkResult VulkanUtil::UpdateDescriptSet(VulkanData & data, ResourceBuffer ** pBuffer, PipelineLayout * pipelineLayout, Material * pMaterial, 
				GraphicsPipelineData * pPipelineData)
	{
		if (pipelineLayout == NULL || pPipelineData == NULL)
		{
			return VK_NOT_READY;
		}

		VkResult result = VK_SUCCESS;
		if (pipelineLayout->vecDescriptorSet.size() == 0)
		{
			VkDescriptorSetAllocateInfo alloc_info[1];
			alloc_info[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			alloc_info[0].pNext = NULL;
			alloc_info[0].descriptorPool = pipelineLayout->pDescriptorPool;
			alloc_info[0].descriptorSetCount = pPipelineData->m_pGraphicsPipeline->vecDescLayout.size();
			alloc_info[0].pSetLayouts = pPipelineData->m_pGraphicsPipeline->vecDescLayout.data();

			pipelineLayout->vecDescriptorSet.resize(alloc_info[0].descriptorSetCount);
			result = vkAllocateDescriptorSets(pipelineLayout->pDevice->handle, alloc_info, pipelineLayout->vecDescriptorSet.data());
		}

		int bindingCount = getDescriptorSetLayoutBindingCount(pMaterial);
		int bindingIndex = 0;
		VkWriteDescriptorSet* writes = new VkWriteDescriptorSet[bindingCount];

		WRITE_DESCRIPTOR_SET(Vertex, pBuffer[0], pMaterial, writes, pipelineLayout, bindingIndex);
		WRITE_DESCRIPTOR_SET(Fragment, pBuffer[1], pMaterial, writes, pipelineLayout, bindingIndex);

		for (size_t i = bindingIndex, index = 0; i < bindingCount; i++, index++)
		{
			writes[i] = {};
			writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writes[i].pNext = NULL;
			writes[i].dstSet = pipelineLayout->vecDescriptorSet[0];
			writes[i].descriptorCount = 1;
			writes[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			writes[i].dstArrayElement = 0;
			writes[i].dstBinding = i;

			VulkanTexture* pTexture = static_cast<VulkanTexture*>(pMaterial->GetTexture(index));
			if (pTexture != NULL)
			{
				if (!pTexture->IsFrameBuffer())
				{
					writes[i].pImageInfo = &pTexture->GetTextureImage()->stImageInfo;
				}
				else
				{
					writes[i].pImageInfo = &data.pSwapchain->vecBuffers[data.pSwapchain->u32CurrentBuffer].imageInfo;
				}
			}
		}

		vkUpdateDescriptorSets(pipelineLayout->pDevice->handle, bindingCount, writes, 0, NULL);
		delete[] writes;

		return result;

	}

	VkResult VulkanUtil::BeginRenderPass(VulkanData & data, Camera* pCamera, int commandBufferIndex, bool primaryCommandBuffer)
	{
		if (data.pSwapchain == NULL || data.pWindow == NULL || data.pDevice == NULL || pCamera == NULL)
		{
			return VK_NOT_READY;
		}

		if (pCamera->GetRenderTexture() == NULL)
		{
			if (data.pRenderPass == NULL)
			{
				data.pRenderPass = new RenderPass();
				createRenderPass(data, data.pRenderPass, true, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
				createClearValue(pCamera, data.pRenderPass);
				CreateFrameBuffer(data);
				data.pRenderPass->rpBeingInfo = new VkRenderPassBeginInfo[data.pSwapchain->u32SwapchainImageCount];
				for (size_t i = 0; i < data.pSwapchain->u32SwapchainImageCount; i++)
				{
					VkRenderPassBeginInfo& rpBeingInfo = data.pRenderPass->rpBeingInfo[i];
					rpBeingInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
					rpBeingInfo.pNext = NULL;
					rpBeingInfo.renderPass = data.pRenderPass->pRenderPass;
					rpBeingInfo.framebuffer = data.pFrameBuffer->pFrameBuffer[i];
					rpBeingInfo.renderArea.offset.x = 0;
					rpBeingInfo.renderArea.offset.y = 0;
					rpBeingInfo.renderArea.extent.width = data.pWindow->Width;
					rpBeingInfo.renderArea.extent.height = data.pWindow->Height;
					rpBeingInfo.clearValueCount = pCamera->GetClearCameraList().size();
					rpBeingInfo.pClearValues = data.pRenderPass->clearValue;
				}
			}
		}
		else
		{
			if (data.mapRenderTexturePass.find(pCamera) == data.mapRenderTexturePass.end())
			{
				RenderPass* pRenderPass = new RenderPass();
				createRenderPass(data, pRenderPass, true, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
				createClearValue(pCamera, pRenderPass);

				VulkanTexture* pTexture = static_cast<VulkanTexture*>(pCamera->GetRenderTexture());
				LoadTextureAndFrameBuffer(data, pCamera->GetRenderTexture(), pRenderPass, commandBufferIndex);
				pRenderPass->rpBeingInfo = new VkRenderPassBeginInfo[1];
				VkRenderPassBeginInfo& rpBeingInfo = pRenderPass->rpBeingInfo[0];
				rpBeingInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				rpBeingInfo.pNext = NULL;
				rpBeingInfo.renderPass = pRenderPass->pRenderPass;
				rpBeingInfo.framebuffer = pTexture->GetTextureImage()->pFrameBuffer;
				rpBeingInfo.renderArea.offset.x = 0;
				rpBeingInfo.renderArea.offset.y = 0;
				rpBeingInfo.renderArea.extent.width = pTexture->GetWidth();
				rpBeingInfo.renderArea.extent.height = pTexture->GetHeight();
				rpBeingInfo.clearValueCount = pCamera->GetClearCameraList().size();
				rpBeingInfo.pClearValues = pRenderPass->clearValue;

				data.mapRenderTexturePass.insert(std::pair<Camera*, RenderPass*>(pCamera, pRenderPass));
			}
		}

		VkRenderPassBeginInfo* info = pCamera->GetRenderTexture() == NULL ? &data.pRenderPass->rpBeingInfo[data.pSwapchain->u32CurrentBuffer] 
																		  : &data.mapRenderTexturePass[pCamera]->rpBeingInfo[0];

		if (primaryCommandBuffer)
			vkCmdBeginRenderPass(data.pDevice->vecCommandBuffer[commandBufferIndex], info, VK_SUBPASS_CONTENTS_INLINE);
		else
			vkCmdBeginRenderPass(data.pDevice->vecCommandBuffer[commandBufferIndex], info, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

		return VK_SUCCESS;
	}

	VkResult VulkanUtil::EndRenderPass(VulkanData & data, int commandBufferIndex)
	{
		if (data.pDevice == NULL)
		{
			return VK_NOT_READY;
		}

		vkCmdEndRenderPass(data.pDevice->vecCommandBuffer[commandBufferIndex]);
		return VK_SUCCESS;
	}

	VkResult VulkanUtil::BindSubMeshIndexBuffer(VulkanData & data, SubMesh* pSubMesh, int commandBufferIndex)
	{
		if (data.pDevice == NULL || pSubMesh == NULL)
		{
			return VK_NOT_READY;
		}

		VkCommandBuffer commandBufer = data.pDevice->vecCommandBuffer[commandBufferIndex];

		IndexData* pIndexData = pSubMesh->m_pIndexData;
		bool hasIndexBuffer = pIndexData != NULL && pIndexData->GetIndexCount() > 0;
		if (hasIndexBuffer)
		{
			VulkanIndexBuffer* pVulkanIndexBuffer = static_cast<VulkanIndexBuffer*>(pIndexData->GetIndexBuffer());
			vkCmdBindIndexBuffer(commandBufer, pVulkanIndexBuffer->GetResourceBuffer()->pBuffer, 0,
				pVulkanIndexBuffer->GetIndexType() == IT_16BIT ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);

			vkCmdDrawIndexed(commandBufer, pIndexData->GetIndexCount(), 1, 0, 0, 0);
		}
		else
		{
			VertexBufferBinding* pBindings = pSubMesh->pMesh->GetVertexData()->GetVertexBinding();
			const VertexBufferBinding::VertexBufferBindingMap& bindingMap = pBindings->GetBindings();
			VertexBufferBinding::VertexBufferBindingMap::const_iterator itr = bindingMap.begin();
			VertexBufferBinding::VertexBufferBindingMap::const_iterator itrEnd = bindingMap.end();

			for (; itr != itrEnd; ++itr)
			{
				VulkanVertexBuffer* pVertexBuffer = static_cast<VulkanVertexBuffer*>(itr->second);
				if (pVertexBuffer != NULL && pVertexBuffer->GetResourceBuffer() != NULL)
				{
					vkCmdDraw(commandBufer, pVertexBuffer->GetNumVertices(), 1, 0, 0);
				}
			}

		}

		return VK_SUCCESS;
	}

	VkResult VulkanUtil::BindSubMeshVertexBuffer(VulkanData & data, SubMesh * pSubMesh, int commandBufferIndex)
	{
		VkCommandBuffer commandBufer = data.pDevice->vecCommandBuffer[commandBufferIndex];

		Mesh* pMesh = pSubMesh->pMesh;
		std::vector<VkBuffer> buffer;
		std::vector<VkDeviceSize> offsets;
		VertexBufferBinding* pBindings = pMesh->GetVertexData()->GetVertexBinding();
		const VertexBufferBinding::VertexBufferBindingMap& bindingMap = pBindings->GetBindings();
		VertexBufferBinding::VertexBufferBindingMap::const_iterator itr = bindingMap.begin();
		VertexBufferBinding::VertexBufferBindingMap::const_iterator itrEnd = bindingMap.end();

		for (; itr != itrEnd; ++itr)
		{
			VulkanVertexBuffer* pVertexBuffer = static_cast<VulkanVertexBuffer*>(itr->second);
			if (pVertexBuffer != NULL && pVertexBuffer->GetResourceBuffer() != NULL)
			{
				buffer.push_back(pVertexBuffer->GetResourceBuffer()->pBuffer);
				offsets.push_back(0);
			}
		}

		vkCmdBindVertexBuffers(commandBufer, 0, buffer.size(), &buffer[0], &offsets[0]);

		return VK_SUCCESS;
	}

	void VulkanUtil::BindGraphicsPipeline(VulkanData & data, GraphicsPipelineData * pGraphicsPipeline, int commandBufferIndex)
	{
		if (pGraphicsPipeline == NULL || pGraphicsPipeline->m_pGraphicsPipeline->pPipeline == NULL)
		{
			return;
		}

		VkCommandBuffer commandBufer = data.pDevice->vecCommandBuffer[commandBufferIndex];
		vkCmdBindPipeline(commandBufer, VK_PIPELINE_BIND_POINT_GRAPHICS, pGraphicsPipeline->m_pGraphicsPipeline->pPipeline);
	}

	void VulkanUtil::BindPipeline(VulkanData & data, PipelineLayout* pPipelineLayout, GraphicsPipelineData * pPipelineData, int commandBufferIndex)
	{
		if (pPipelineData == NULL ||
			pPipelineData->m_pGraphicsPipeline == NULL || pPipelineData->m_pGraphicsPipeline->pPipelineLayout == NULL)
		{
			return;
		}

		VkCommandBuffer commandBufer = data.pDevice->vecCommandBuffer[commandBufferIndex];

		vkCmdBindDescriptorSets(commandBufer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			pPipelineData->m_pGraphicsPipeline->pPipelineLayout, 0, pPipelineLayout->vecDescriptorSet.size(), &pPipelineLayout->vecDescriptorSet[0],
			0, NULL);
	}

	VkResult VulkanUtil::BindViewport(VulkanData & data, int commandBufferIndex)
	{
		if (data.pDevice == NULL || data.pWindow == NULL)
		{
			return VK_NOT_READY;
		}

		VkViewport vp;
		vp.height = data.pWindow->Height;
		vp.width = data.pWindow->Width;
		vp.minDepth = 0.0f;
		vp.maxDepth = 1.0f;
		vp.x = 0.0f;
		vp.y = 0.0f;
		vkCmdSetViewport(data.pDevice->vecCommandBuffer[commandBufferIndex], 0, 1, &vp);

		VkRect2D scissor;
		scissor.extent.width = data.pWindow->Width;
		scissor.extent.height = data.pWindow->Height;
		scissor.offset.x = 0.0f;
		scissor.offset.y = 0.0f;
		vkCmdSetScissor(data.pDevice->vecCommandBuffer[commandBufferIndex], 0, 1, &scissor);

		return VK_SUCCESS;
	}
	VkResult VulkanUtil::BindViewport(VulkanData & data, int width, int height, int commandBufferIndex)
	{
		if (data.pDevice == NULL || data.pWindow == NULL)
		{
			return VK_NOT_READY;
		}

		VkViewport vp;
		vp.height = height;
		vp.width = width;
		vp.minDepth = 0.0f;
		vp.maxDepth = 1.0f;
		vp.x = 0.0f;
		vp.y = 0.0f;
		vkCmdSetViewport(data.pDevice->vecCommandBuffer[commandBufferIndex], 0, 1, &vp);

		VkRect2D scissor;
		scissor.extent.width = width;
		scissor.extent.height = height;
		scissor.offset.x = 0.0f;
		scissor.offset.y = 0.0f;
		vkCmdSetScissor(data.pDevice->vecCommandBuffer[commandBufferIndex], 0, 1, &scissor);

		return VK_SUCCESS;
	}
}