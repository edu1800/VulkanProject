#ifndef _INDEX_BUFFER_H_
#define _INDEX_BUFFER_H_

#pragma once

#include "HardwareBufferBase.h"
#include "VertexIndexData.h"
#include "VertexIndexDefine.h"

namespace Vulkan
{
	class IndexBuffer : public HardwareBuffer
	{
	public:
		IndexBuffer(IndexType indexType, unsigned int numIndexes);
		virtual ~IndexBuffer();

		virtual void ReadData(unsigned int offset, unsigned int length, void* pDest);
		virtual void WriteData(unsigned int offset, unsigned int length, void* pSrc);

		IndexType GetIndexType() const { return m_eIndexType; }
		unsigned int GetNumIndexes() const { return m_u32NumIndexes; }
		unsigned int GetIndexSize() const { return m_u32IndexSize; }
	protected:
		IndexType m_eIndexType;
		unsigned int m_u32NumIndexes;
		unsigned int m_u32IndexSize;
	};
}
#endif