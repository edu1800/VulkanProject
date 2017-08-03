#include "VulkanData.h"
#include "../Core/include/Transform.h"
#include "../Core/include/CommonUtil.h"
#include "../Core/include/Resource.h"
#include "../ThirdParty/glfw/include/GLFW/glfw3.h"

namespace Vulkan
{
	VulkanData::VulkanData()
		: pInstance(NULL)
		, pDevice(NULL)
		, u32PresentQueueFamilyIndex(UINT32_MAX)
		, u32graphicsQueueFamilyIndex(UINT32_MAX)
		, pSwapchain(NULL)
		, pWindow(NULL)
		, pDepthBuffer(NULL)
		, pRenderPass(NULL)
		, pFrameBuffer(NULL)
		, pSemaphore(NULL)
		, pFence(NULL)
		, pCompositor(NULL)
	{
	}

	VulkanData::~VulkanData()
	{
		MapGraphicsPipelne::iterator itr = m_mapGraphicsPipeline.begin();
		MapGraphicsPipelne::iterator itrEnd = m_mapGraphicsPipeline.end();
		for (; itr != itrEnd; ++itr)
		{
			SAFE_DELETE(itr->second);
		}
		m_mapGraphicsPipeline.clear();

		MapPipelineLayout::iterator mItr = m_mapPipeline.begin();
		MapPipelineLayout::iterator mItrEnd = m_mapPipeline.end();
		for (; mItr != mItrEnd; ++mItr)
		{
			std::map<Material*, PipelineLayout*>& pipeline = mItr->second;
			std::map<Material*, PipelineLayout*>::iterator pItr = pipeline.begin();
			std::map<Material*, PipelineLayout*>::iterator pItrEnd = pipeline.end();
			for (; pItr != pItrEnd; ++pItr)
			{
				SAFE_DELETE(pItr->second);
			}
		}
		m_mapPipeline.clear();

		SAFE_DELETE(pCompositor);
		VEC_DELETE_POINTER(pSemaphore);
		SAFE_DELETE(pFence);
		SAFE_DELETE(pFrameBuffer);
		SAFE_DELETE(pRenderPass);
		std::map<Camera*, RenderPass*>::iterator cItr = mapRenderTexturePass.begin();
		std::map<Camera*, RenderPass*>::iterator cItrEnd = mapRenderTexturePass.end();
		for (; cItr != cItrEnd; ++cItr)
		{
			SAFE_DELETE(cItr->second);
		}
		mapRenderTexturePass.clear();
		SAFE_DELETE(pDepthBuffer);
		SAFE_DELETE(pSwapchain);
		SAFE_DELETE(pDevice);
		SAFE_DELETE(pWindow);
		VEC_DELETE_POINTER(vecPhysicalDevices);
		vkDestroyInstance(pInstance, NULL);
	}

	Device::~Device()
	{
		if (vecCommandBuffer.size() > 0)
		{
			vkFreeCommandBuffers(handle, pCommandPool, vecCommandBuffer.size(), vecCommandBuffer.data());
		}

		if (pCommandPool != NULL)
		{
			vkDestroyCommandPool(handle, pCommandPool, NULL);
		}

		vkDestroyDevice(handle, NULL);
	}

	Window::Window()
#ifdef Windows
		:connection(NULL)
		, hWnd(NULL)
		, pWindow(NULL)
#endif
		, pSurface(NULL)
		, pInstance(NULL)
	{
	}

	Window::~Window()
	{
		if (pSurface != NULL)
		{
			vkDestroySurfaceKHR(pInstance, pSurface, NULL);
			pSurface = NULL;
		}

		glfwDestroyWindow(pWindow);
	}

	Swapchain::~Swapchain()
	{
		for (size_t i = 0; i < u32SwapchainImageCount; i++)
		{
			vkDestroyImageView(pDevice->handle, vecBuffers[i].pView, NULL);
		}

		vkDestroySwapchainKHR(pDevice->handle, handle, NULL);
		vkDestroySampler(pDevice->handle, pSampler, NULL);
	}

	DepthBuffer::~DepthBuffer()
	{
		vkDestroyImageView(pDevice->handle, pBuffer.pView, NULL);
		vkDestroyImage(pDevice->handle, pBuffer.pImage, NULL);
		vkFreeMemory(pDevice->handle, pMem, NULL);
	}

	ResourceBuffer::~ResourceBuffer()
	{
		vkDestroyBuffer(pDevice->handle, pBuffer, NULL);
		vkFreeMemory(pDevice->handle, pMem, NULL);
	}

	PipelineLayout::~PipelineLayout()
	{
		if (pDescriptorPool != NULL)
		{
			vkDestroyDescriptorPool(pDevice->handle, pDescriptorPool, NULL);
		}
	}

	RenderPass::~RenderPass()
	{
		if (pDevice != NULL)
		{
			if (clearValue != NULL)
			{
				delete[] clearValue;
				clearValue = NULL;
			}

			vkDestroyRenderPass(pDevice->handle, pRenderPass, NULL);
		}

		if (rpBeingInfo != NULL)
		{
			delete[] rpBeingInfo;
			rpBeingInfo = NULL;
		}
	}

	ShaderModule::~ShaderModule()
	{
		if (pDevice != NULL)
		{
			vkDestroyShaderModule(pDevice->handle, stShaderState.module, NULL);
		}
	}

	FrameBuffer::~FrameBuffer()
	{
		if (pDevice != NULL)
		{
			for (size_t i = 0; i < u32FrameBufferCount; i++)
			{
				vkDestroyFramebuffer(pDevice->handle, pFrameBuffer[i], NULL);
			}

			free(pFrameBuffer);
			pFrameBuffer = NULL;
		}
	}

	GraphicsPipeline::GraphicsPipeline()
		:pDevice(NULL)
		, pPipeline(NULL)
		, pPipelineLayout(NULL)
	{
	}

	GraphicsPipeline::~GraphicsPipeline()
	{
		if (pDevice != NULL)
		{
			vkDestroyPipeline(pDevice->handle, pPipeline, NULL);
		}

		if (pPipelineLayout != NULL)
		{
			vkDestroyPipelineLayout(pDevice->handle, pPipelineLayout, NULL);
		}

		for (size_t i = 0; i < vecDescLayout.size(); i++)
		{
			vkDestroyDescriptorSetLayout(pDevice->handle, vecDescLayout[i], NULL);
		}

		vecDescLayout.clear();
	}

	Semaphore::~Semaphore()
	{
		if (pDevice != NULL)
		{
			vkDestroySemaphore(pDevice->handle, pSemaphore, NULL);
		}
	}
	Fence::~Fence()
	{
		if (pDevice != NULL)
		{
			vkDestroyFence(pDevice->handle, pFence, NULL);
		}
	}

	TextureImage::~TextureImage()
	{
		if (pDevice != NULL)
		{
			vkDestroySampler(pDevice->handle, pSampler, NULL);
			vkDestroyImageView(pDevice->handle, pImageView, NULL);
			vkDestroyImage(pDevice->handle, pImage, NULL);
			vkFreeMemory(pDevice->handle, pMem, NULL);

			if (pStageImageView != NULL)
			{
				vkDestroyImageView(pDevice->handle, pStageImageView, NULL);
			}

			if (pStagingImage != NULL)
			{
				vkDestroyImage(pDevice->handle, pStagingImage, NULL);
			}

			if (pStagingMem != NULL)
			{
				vkFreeMemory(pDevice->handle, pStagingMem, NULL);
			}

			if (pFrameBuffer != NULL)
			{
				vkDestroyFramebuffer(pDevice->handle, pFrameBuffer, NULL);
			}
		}
	}

	GraphicsPipelineData::GraphicsPipelineData()
		:isInit(false)
	{
		m_pGraphicsPipeline = new GraphicsPipeline();
	}

	GraphicsPipelineData::~GraphicsPipelineData()
	{
		SAFE_DELETE(m_pGraphicsPipeline);
	}

	CompositorData::~CompositorData()
	{
		SAFE_DELETE(pRenderPass);
		std::map< Camera*, std::vector<CompositorPassData*>>::iterator cItr = listPassData.begin();
		std::map< Camera*, std::vector<CompositorPassData*>>::iterator cItrEnd = listPassData.end();
		for (; cItr != cItrEnd; ++cItr)
		{
			VEC_DELETE_POINTER(cItr->second);
		}
		listPassData.clear();

		if (pSampler != NULL)
		{
			vkDestroySampler(pDevice->handle, pSampler, NULL);
		}
	}

	CompositorPassData::~CompositorPassData()
	{
		VEC_DELETE_POINTER(listFrameBuffer);
	}

	CompositorFrameBuffer::~CompositorFrameBuffer()
	{
		SAFE_DELETE(pVertexUniformBuffer);
		SAFE_DELETE(pFragmentUniformBuffer);
		SAFE_DELETE(pPilelineLayout);
		SAFE_DELETE(pGraphicsPipeline);
		SAFE_DELETE(pSemaphore);
	}
}