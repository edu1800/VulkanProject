#include "VulkanIndexBuffer.h"
#include "VulkanData.h"
#include "VulkanRenderer.h"
#include "../Core/include/Root.h"
#include "VulkanUtil.h"

namespace Vulkan
{
	Vulkan::VulkanIndexBuffer::VulkanIndexBuffer(IndexType indexType, unsigned int numIndexes)
		:IndexBuffer(indexType, numIndexes)
	{
	}

	Vulkan::VulkanIndexBuffer::~VulkanIndexBuffer()
	{
	}

	void * Vulkan::VulkanIndexBuffer::lockImpl(unsigned int offset, unsigned int length)
	{
		VulkanData* pVulkanData = static_cast<VulkanRenderer*>(Root::GetSingleton().GetRenderer())->GetVulkanData();
		if (pVulkanData == NULL || pVulkanData->pDevice == NULL)
		{
			return NULL;
		}

		if (m_pResourceBuffer != NULL)
		{
			SAFE_DELETE(m_pResourceBuffer);
		}

		m_pResourceBuffer = VulkanUtil::CreateResourceBuffer(*pVulkanData, m_u32SizeInBytes, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		uint8_t* pData;
		vkMapMemory(m_pResourceBuffer->pDevice->handle, m_pResourceBuffer->pMem, 0, m_pResourceBuffer->BufferInfo.range, 0, (void**)&pData);

		return pData;
	}

	void Vulkan::VulkanIndexBuffer::unLockImpl()
	{
		VulkanData* pVulkanData = static_cast<VulkanRenderer*>(Root::GetSingleton().GetRenderer())->GetVulkanData();
		if (pVulkanData == NULL || pVulkanData->pDevice == NULL || m_pResourceBuffer == NULL)
		{
			return;
		}

		vkUnmapMemory(m_pResourceBuffer->pDevice->handle, m_pResourceBuffer->pMem);
		vkBindBufferMemory(m_pResourceBuffer->pDevice->handle, m_pResourceBuffer->pBuffer, m_pResourceBuffer->pMem, 0);
	}
}

