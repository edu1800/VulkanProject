#ifndef _VULKAN_DATA_H_
#define _VULKAN_DATA_H_

#pragma once

#include <map>
#include <vector>
#include "../ThirdParty/glfw/include/GLFW/glfw3.h"
#include "../ThirdParty/vulkan/include/vulkan.h"

namespace Vulkan
{
	struct PhysicalDevice
	{
		VkPhysicalDevice handle = NULL;
		uint32_t queueFamilyPropertyCount = 0;
		std::vector<VkQueueFamilyProperties> vecQueueFamilyProps;
		VkPhysicalDeviceMemoryProperties memProps;
		VkPhysicalDeviceProperties props;
	};

	struct Device
	{
		~Device();
		int physicalDeviceIndex = 0;
		VkDevice handle = NULL;
		uint32_t u32QueueFamilyIndex = 0;
		VkQueue pGraphicsQueue = NULL;
		VkQueueFlagBits eQueueFlagBits;
		VkCommandPool pCommandPool = NULL;
		std::vector<VkCommandBuffer> vecCommandBuffer;
	};

	struct Window
	{
		Window();
		~Window();

		int Width;
		int Height;

		VkSurfaceKHR pSurface;
		VkFormat format;
		VkInstance pInstance;
		GLFWwindow* pWindow = NULL;

#ifdef Windows
		HINSTANCE connection;	// hInstance - Windows Instance
		char name[80];			// Name to put on the window/icon
		HWND hWnd;		    	// hWnd - window handle	
#endif
	};

	struct ImageViewBuffer
	{
		VkImage pImage = NULL;
		VkImageView pView = NULL;
		VkDescriptorImageInfo imageInfo;
	};

	struct Swapchain
	{
		~Swapchain();

		VkSampler pSampler = NULL;
		VkSwapchainKHR handle = NULL;
		Device* pDevice = NULL;
		uint32_t u32SwapchainImageCount = 0;
		std::vector<ImageViewBuffer> vecBuffers;
		uint32_t u32CurrentBuffer = -1;
	};

	struct DepthBuffer
	{
		~DepthBuffer();

		Device* pDevice = NULL;
		ImageViewBuffer pBuffer;
		VkDeviceMemory pMem = NULL;
	};

	struct ResourceBuffer
	{
		~ResourceBuffer();

		Device* pDevice = NULL;
		VkBuffer pBuffer = NULL;
		VkDeviceMemory pMem = NULL;
		VkDescriptorBufferInfo BufferInfo;
	};

	struct TextureImage
	{
		~TextureImage();
		Device* pDevice = NULL;
		VkImage pImage = NULL;
		VkDeviceMemory pMem = NULL;
		VkImageView pImageView = NULL;
		VkImage pStagingImage = NULL;
		VkDeviceMemory pStagingMem = NULL;
		VkImageView pStageImageView = NULL;
		VkSampler pSampler = NULL;
		VkDescriptorImageInfo stImageInfo;
		VkFramebuffer pFrameBuffer = NULL;
	};

	struct GraphicsPipeline
	{
		GraphicsPipeline();
		~GraphicsPipeline();
		Device* pDevice = NULL;
		VkPipeline pPipeline = NULL;
		VkPipelineLayout pPipelineLayout = NULL;
		std::vector<VkDescriptorSetLayout> vecDescLayout;
	};

	struct PipelineLayout
	{
		~PipelineLayout();
		Device* pDevice = NULL;
		std::vector<VkDescriptorSet> vecDescriptorSet;
		VkDescriptorPool pDescriptorPool = NULL;
	};

	struct RenderPass
	{
		~RenderPass();
		Device* pDevice = NULL;
		VkRenderPass pRenderPass = NULL;
		VkRenderPassBeginInfo* rpBeingInfo = NULL;
		VkClearValue* clearValue = NULL;
	};

	struct ShaderModule
	{
		~ShaderModule();
		Device* pDevice = NULL;
		VkPipelineShaderStageCreateInfo stShaderState;
	};

	struct FrameBuffer
	{
		virtual ~FrameBuffer();
		Device* pDevice = NULL;
		VkFramebuffer* pFrameBuffer = NULL;
		uint32_t u32FrameBufferCount = 0;
	};

	struct Fence
	{
		~Fence();
		Device* pDevice = NULL;
		VkFence pFence = NULL;
	};

	struct Semaphore
	{
		~Semaphore();
		Device* pDevice = NULL;
		VkSemaphore pSemaphore = NULL;
	};

	struct GraphicsPipelineData
	{
		GraphicsPipelineData();
		~GraphicsPipelineData();
		GraphicsPipeline* m_pGraphicsPipeline = NULL;
		bool isInit = false;
	};

	class Material;
	struct CompositorFrameBuffer : FrameBuffer
	{
		virtual ~CompositorFrameBuffer();
		Semaphore* pSemaphore = NULL;
		Material* pMaterial = NULL;
		GraphicsPipelineData* pGraphicsPipeline = NULL;
		PipelineLayout* pPilelineLayout = NULL;
		ResourceBuffer* pVertexUniformBuffer = NULL;
		ResourceBuffer* pFragmentUniformBuffer = NULL;
		int iWidth = 0;
		int iHeight = 0;
		int iTextureWidth = 0;
		int iTextureHeight = 0;
	};

	struct CompositorPassData
	{
		~CompositorPassData();
		std::vector<CompositorFrameBuffer*> listFrameBuffer;
		bool isInit = false;
	};

	class Camera;
	struct CompositorData
	{
		~CompositorData();
		Device* pDevice = NULL;
		Semaphore* pSemaphore = NULL;
		VkSampler pSampler = NULL;
		RenderPass* pRenderPass = NULL;
		std::map< Camera*, std::vector<CompositorPassData*>> listPassData;
	};

	class Material;
	class Camera;
	struct SubMesh;
	typedef std::map<Material*, GraphicsPipelineData*> MapGraphicsPipelne;
	typedef std::map<SubMesh*, std::map<Material*, PipelineLayout*>> MapPipelineLayout;

	struct VulkanData
	{
		VulkanData();
		~VulkanData();
		VkInstance pInstance;
		std::vector<const char*> vecInstanceLayerName;
		std::vector<const char*> vecInstanceExtensionName;

		std::vector<PhysicalDevice*> vecPhysicalDevices;

		Device* pDevice;
		std::vector<const char*> vecDeviceLayerName;
		std::vector<const char*> vecDeviceExtensionName;
		uint32_t u32PresentQueueFamilyIndex;
		uint32_t u32graphicsQueueFamilyIndex;

		Window* pWindow;
		Swapchain* pSwapchain;
		DepthBuffer* pDepthBuffer;
		RenderPass* pRenderPass;
		std::map<Camera*, RenderPass*> mapRenderTexturePass;
		FrameBuffer* pFrameBuffer;
		std::vector<Semaphore*> pSemaphore;
		Fence* pFence;

		CompositorData* pCompositor;

		MapGraphicsPipelne m_mapGraphicsPipeline;
		MapPipelineLayout m_mapPipeline;

		VkDynamicState dynamicStateEnables[VK_DYNAMIC_STATE_RANGE_SIZE];
		VkPipelineDynamicStateCreateInfo dynamicState = {};
		VkPipelineViewportStateCreateInfo vp = {};
		VkPipelineMultisampleStateCreateInfo ms;
	};
}

#endif
