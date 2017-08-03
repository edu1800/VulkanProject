#ifndef _VULKAN_UTIL_H_
#define _VULKAN_UTIL_H_

#pragma once

#include <string>
#include "../ThirdParty/vulkan/include/vulkan.h"
#include "../ThirdParty/vulkan/include/vk_sdk_platform.h"
#include "VulkanData.h"
#include "../ThirdParty/glm/vec3.hpp"
#include "../ThirdParty/glm/gtx/quaternion.hpp"
#include "../ThirdParty/glslang/Public/ShaderLang.h"
#include "../ThirdParty/glslang/Include/ResourceLimits.h"
#include "../Core/include/Shader.h"
#include "../Core/include/MaterialParameterDefine.h"
#include "../Core/include/VertexIndexDefine.h"

namespace Vulkan
{
	typedef std::vector<VkVertexInputAttributeDescription> InputAttributeList;
	typedef std::vector<VkVertexInputBindingDescription> InputBindingList;

	class Transform;
	class Camera;
	class GameObject;
	class Mesh;
	struct SubMesh;
	class Material;
	class Texture;
	class MeshRenderer;
	class VertexDeclaration;
	class VertexBufferBinding;
	struct GraphicsPipelineData;
	class VulkanUtil
	{
	public:
		static VkResult CreateVkInstance(struct VulkanData& data, const char* appName, int appVersion, const char* engineName, int engineVersion);
		static void AddInstanceExtensionName(struct VulkanData& data);
		static void AddDeviceExtensionName(struct VulkanData& data);

		static VkResult GetPhysicalDevices(struct VulkanData& data);

		static VkResult CreateDevice(struct VulkanData& data, int physicalDeviceIndex = 0);
		static VkResult CreateDevice(struct VulkanData& data, VkQueueFlagBits flag, int physicalDeviceIndex = 0);
		static VkResult SubmitQueue(struct VulkanData& data, Semaphore* waitSemaphore, int commandBufferIndex = 0);
		static VkResult SubmitQueue(struct VulkanData& data, Semaphore* waitSemaphore, Semaphore* signalSemphore, int commandBufferIndex = 0);
		static bool IsFinishSubmitQueue(struct VulkanData& data);
		static VkResult CreateCommandPool(struct VulkanData& data);
		static VkResult CreateCommandBuffer(struct VulkanData& data, int commandBufferCount = 1,
			VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		static VkResult BeingCommandBuffer(struct VulkanData& data, int commandBufferIndex = 0);
		static VkResult EndCommandBuffer(struct VulkanData& data, int commandBufferIndex = 0);
		static void EndSingleTimeCommandBuffer(struct VulkanData& data, int commandBufferIndex = 0);

		static void CreateConnection(struct VulkanData& data);
		static void CreateDisplayWindow(struct VulkanData& data, const char* windowName, int width, int height);
		static bool OnWindowDrawing(struct VulkanData& data);
		static VkResult CreateSurface(struct VulkanData& data, int physicalDeviceIndex = 0);
		static VkResult CreateSwapchain(struct VulkanData& data,
			VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
			| VK_IMAGE_USAGE_STORAGE_BIT,
			int physicalDeviceIndex = 0);
		static VkResult AttachToSwapchain(struct VulkanData& data, int cameraIndex, int commandBufferIndex);
		static VkResult PresentSwapchainToDisplay(struct VulkanData& data);

		static VkResult CreateDepthBuffer(struct VulkanData& data, int commandBufferIndex);

		static VkResult CreateUniformBuffer(struct VulkanData& data, GameObject* pGameObject, Material* pMaterial, Camera* camera, bool isModifyCamera);

		static VkResult CreatePipelineLayout(struct VulkanData& data, Material* pMaterial, GraphicsPipelineData* pPipelineData);

		static VkResult CreateDescriptPool(VulkanData & data, PipelineLayout* pipelineLayout, Material* pMaterial);
		static VkResult UpdateDescriptSet(VulkanData & data, GameObject* pGo, PipelineLayout* pipelineLayout, Material* pMaterial, 
						GraphicsPipelineData* pPipelineData);
		static VkResult UpdateDescriptSet(VulkanData & data, ResourceBuffer** pBuffer, PipelineLayout* pipelineLayout, Material* pMaterial,
			GraphicsPipelineData* pPipelineData);

		static VkResult BeginRenderPass(struct VulkanData& data, Camera* pCamera, int commandBufferIndex = 0, bool primaryCommandBuffer = true);
		static VkResult EndRenderPass(struct VulkanData& data, int commandBufferIndex = 0);
		static VkResult BindSubMeshIndexBuffer(struct VulkanData& data, SubMesh* pSubMesh, int commandBufferIndex = 0);
		static VkResult BindSubMeshVertexBuffer(struct VulkanData& data, SubMesh* pSubMesh, int commandBufferIndex = 0);
		static void BindGraphicsPipeline(struct VulkanData& data, GraphicsPipelineData* pGraphicsPipeline, int commandBufferIndex = 0);
		static void BindPipeline(struct VulkanData& data, PipelineLayout* pPipelineLayout, GraphicsPipelineData* pPipelineData, int commandBufferIndex = 0);
		static VkResult BindViewport(struct VulkanData& data, int commandBufferIndex = 0);
		static VkResult BindViewport(struct VulkanData& data, int width, int height, int commandBufferIndex = 0);


		static void SetImageLayout(struct VulkanData& data, VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout,
					int commandBufferIndex = 0);

		static ResourceBuffer* CreateResourceBuffer(struct VulkanData& data, unsigned int bufferSize, VkBufferUsageFlagBits bufferUsage, VkFlags requirementsMask);

		static bool CreateShader(struct VulkanData& data, Material* pMaterial, GraphicsPipelineData* pPipelineData, int commandBufferIndex = 0);
		static void LoadTexture(struct VulkanData& data, Material* pMaterial, int commandBufferIndex);
		static void LoadTextureAndFrameBuffer(struct VulkanData& data, Texture* pTexture, RenderPass* pRenderPass, int commandBufferIndex);
		static VkResult CreateFrameBuffer(struct VulkanData& data);

		static void BindDataVertexBuffer(Mesh* pMesh);
		static void BindDataIndexBuffer(SubMesh* pMesh);

		static void CreateDynamicState(struct VulkanData& data);
		static VkResult CreatePipeline(struct VulkanData& data, Camera* pCamera, Material* pMaterial, SubMesh* pSubMesh, GraphicsPipelineData* pPipelineData, RenderPass* pRenderPass = NULL);

		static VkResult CreateSemphore(struct VulkanData& data, int count);
		static VkResult CreateFence(struct VulkanData& data);

		static void CreateCompositors(struct VulkanData& data);
		static void DrawCompositors(struct VulkanData& data, Camera* pCamera, int commandBufferIndex);
		static void CreateCompositorShaders(struct VulkanData& data, Camera* pCamera, int commandBufferIndex = 0);

		static VkImage CreateVkImage(VkDevice device, VkImageType imageType, VkFormat format, uint32_t width, uint32_t height, VkImageUsageFlags usageBits, VkImageTiling tiling);
		static VkImageView CreateVkImageView(VkDevice device, VkImageViewType viewType, VkFormat format, VkImageAspectFlags aspactMask, VkImage image);

		static bool GetMemoryTypeFromProperties(PhysicalDevice* pPhysicaldevice, uint32_t typeBits, VkFlags requirementsMask, uint32_t* typeIndex);
	private:
		static VkResult createDevice(struct VulkanData& data, VkQueueFlagBits flag, int physicalDeviceIndex, uint32_t queueFamilyIndex);
		static int getDescriptorSetLayoutBindingCount(Material* pMaterial);

		static bool glslToSPV(const VkShaderStageFlagBits shaderType, const char* pShader, std::vector<unsigned int>& spirv);
		static EShLanguage FindLanguage(const VkShaderStageFlagBits shaderType);
		static void initShaderResource(TBuiltInResource& resources);
		static bool createShader(Device* pDevice, Shader* pShader);
		static void createShaderParameter(Material* pMaterial, std::vector<VkPushConstantRange>& constants);
		static void updateShaderParameter(Material* pMaterial, Device * pDevice, GraphicsPipeline* pPipeline, int commandBufferIndex);
		static VkShaderStageFlagBits getShaderStageFlagBits(ShaderType type);

		static VkCullModeFlagBits GetCullModeFlagBits(CullMode mode);
		static VkBlendOp GetBlendingOperation(SceneBlendOperation operation);
		static VkBlendFactor GetBlendingFactor(SceneBlendFactor factor);
		static VkCompareOp GetCompareOperation(DepthCompareFunction func);

		static void buildInputAttributeDescription(VertexDeclaration* pDecl, InputAttributeList& inputAttrList, unsigned short bindingIndex = 0);
		static VkFormat transformFormat(VertexElementType eType);
		static void buildInputBinding(VertexBufferBinding* pBinding, InputBindingList& bindingList);

		static void updateUniformBuffer(struct VulkanData& data, Transform* pTransform, Camera* pCamera, ResourceBuffer* pBuffer, ShaderParameters* pParam, bool isCreated);
		static void createShaderConstantParameterInfo(Shader* pShader, VkSpecializationInfo& specInfo, std::vector<VkSpecializationMapEntry>& specMapEntryList);

		static void createClearValue(Camera *pCamera, RenderPass* pRenderPass);
		static void createRenderPass(struct VulkanData& data, RenderPass* pRenderPass, bool isClear, VkImageLayout finalLayout);
	};
}

#endif
