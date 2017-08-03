#include "../include/VertexIndexData.h"
#include "../include/VertexBuffer.h"
#include "../include/CommonUtil.h"
#include "../include/IndexBuffer.h"
#include "../include/HardwareBufferManager.h"

namespace Vulkan
{
	VertexData::VertexData()
	{
		m_pVertexDeclaration = new VertexDeclaration();
		m_pVertexBufferBinding = new VertexBufferBinding();

		unsigned int offset = 0;
		m_pVertexDeclaration->AddElement(0, 0, VET_FLOAT3, VES_POSITION);
		offset += VertexElement::GetTypeSize(VET_FLOAT3);
		m_pVertexDeclaration->AddElement(0, offset, VET_FLOAT3, VES_NORMAL);
		offset += VertexElement::GetTypeSize(VET_FLOAT3);
		m_pVertexDeclaration->AddElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES);
		offset += VertexElement::GetTypeSize(VET_FLOAT2);
		//m_pVertexDeclaration->AddElement(0, offset, VET_COLOUR, VES_DIFFUSE);
	}

	VertexData::~VertexData()
	{
		SAFE_DELETE(m_pVertexDeclaration);
		SAFE_DELETE(m_pVertexBufferBinding);
	}
	
	IndexData::IndexData(IndexType indexType, unsigned int indexCount)
	{
		if (indexCount != 0)
		{
			m_pIndexBuffer = HardwareBufferManager::GetSingleton().CreateIndexBuffer(indexType, indexCount);
		}
		else
		{
			m_pIndexBuffer = NULL;
		}
	}
	
	IndexData::~IndexData()
	{
		if (m_pIndexBuffer != NULL)
		{
			HardwareBufferManager::GetSingleton().DestroyIndexBuffer(m_pIndexBuffer);
			m_pIndexBuffer = NULL;
		}
	}
	
	unsigned int IndexData::GetIndexCount()
	{
		return m_pIndexBuffer == NULL ? 0 : m_pIndexBuffer->GetNumIndexes();
	}
	
	IndexType IndexData::GetIndexType()
	{
		return m_pIndexBuffer == NULL ? IT_16BIT : m_pIndexBuffer->GetIndexType();
	}
}

