#ifndef _VULKAN_VERTEX_BUFFER_H_
#define _VULKAN_VERTEX_BUFFER_H_

#pragma once

#include "../Core/include/VertexBuffer.h"

namespace Vulkan
{
	struct ResourceBuffer;
	class VulkanVertexBuffer : public VertexBuffer
	{
	public:
		VulkanVertexBuffer(unsigned int vertexSize, unsigned int numVertices);
		virtual ~VulkanVertexBuffer();

		ResourceBuffer* GetResourceBuffer() const { return m_pResourceBuffer; }

	protected:
		virtual void* lockImpl(unsigned int offset, unsigned int length);
		virtual void unLockImpl();

		ResourceBuffer* m_pResourceBuffer;
	};
}
#endif
