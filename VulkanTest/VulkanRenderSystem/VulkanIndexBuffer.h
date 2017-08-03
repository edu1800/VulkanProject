#ifndef _VULKAN_INDEX_BUFFER_H_
#define _VULKAN_INDEX_BUFFER_H_

#pragma once

#include "../Core/include/IndexBuffer.h"

namespace Vulkan
{
	struct ResourceBuffer;
	class VulkanIndexBuffer : public IndexBuffer
	{
	public:
		VulkanIndexBuffer(IndexType indexType, unsigned int numIndexes);
		virtual ~VulkanIndexBuffer();

		ResourceBuffer* GetResourceBuffer() const { return m_pResourceBuffer; }

	protected:
		virtual void* lockImpl(unsigned int offset, unsigned int length);
		virtual void unLockImpl();

		ResourceBuffer* m_pResourceBuffer;
	};
}

#endif
