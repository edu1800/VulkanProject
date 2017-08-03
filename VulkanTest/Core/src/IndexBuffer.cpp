#include "../include/IndexBuffer.h"

namespace Vulkan
{
	IndexBuffer::IndexBuffer(IndexType indexType, unsigned int numIndexes)
		: HardwareBuffer()
		, m_eIndexType(indexType)
		, m_u32NumIndexes(numIndexes)
	{
		switch (m_eIndexType)
		{
		case Vulkan::IT_16BIT:
			m_u32IndexSize = sizeof(unsigned short);
			break;
		case Vulkan::IT_32BIT:
			m_u32IndexSize = sizeof(unsigned int);
			break;
		default:
			break;
		}

		m_u32SizeInBytes = m_u32IndexSize * m_u32NumIndexes;
	}

	IndexBuffer::~IndexBuffer()
	{
	}

	void IndexBuffer::ReadData(unsigned int offset, unsigned int length, void * pDest)
	{
	}

	void IndexBuffer::WriteData(unsigned int offset, unsigned int length, void * pSrc)
	{
	}
}