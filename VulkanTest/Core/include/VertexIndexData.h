#ifndef _VERTEX_INDEX_DATA_H_
#define _VERTEX_INDEX_DATA_H_

#pragma once

#include "VertexIndexDefine.h"

namespace Vulkan
{
	class VertexDeclaration;
	class VertexBufferBinding;
	class IndexBuffer;
	class VertexData
	{
	public:
		VertexData();
		~VertexData();

		VertexDeclaration* GetVertexDeclaration() const { return m_pVertexDeclaration; }
		VertexBufferBinding* GetVertexBinding() const { return m_pVertexBufferBinding; }

	private:
		VertexDeclaration* m_pVertexDeclaration;
		VertexBufferBinding* m_pVertexBufferBinding;
		unsigned int m_u32VertexStart;
		unsigned int m_u32VertexCount;
	};

	class IndexData
	{
	public:
		IndexData(IndexType indexType, unsigned int indexCount);
		~IndexData();

		IndexBuffer* GetIndexBuffer() const { return m_pIndexBuffer; }
		unsigned int GetIndexCount();
		IndexType GetIndexType();
	private:
		IndexBuffer* m_pIndexBuffer;
	};
}

#endif
