#include "../include/HardwareBufferManager.h"
#include "../include/VertexBuffer.h"
#include "../include/IndexBuffer.h"
#include "../include/Root.h"
#include "../include/Renderer.h"

namespace Vulkan
{
	SINGLETON_IMPL(HardwareBufferManager)

	HardwareBufferManager::HardwareBufferManager()
	{
	}

	HardwareBufferManager::~HardwareBufferManager()
	{
		VertexBufferList::reverse_iterator itr = m_listVertexBuffer.rbegin();
		VertexBufferList::reverse_iterator itrEnd = m_listVertexBuffer.rend();
		
		for (; itr != itrEnd; ++itr)
		{
			delete *itr;
		}
		m_listVertexBuffer.clear();

		IndexBufferList::reverse_iterator iItr = m_listIndexBuffer.rbegin();
		IndexBufferList::reverse_iterator iItrEnd = m_listIndexBuffer.rend();

		for (; iItr != iItrEnd; ++iItr)
		{
			delete *iItr;
		}
		m_listIndexBuffer.clear();
	}

	VertexBuffer * HardwareBufferManager::CreateVertexBuffer(unsigned int vertexSize, unsigned int numVerts)
	{
		VertexBuffer* pBuffer = Root::GetSingleton().GetRenderer()->CreateVertexBuffer(vertexSize, numVerts);
		m_listVertexBuffer.insert(pBuffer);
		return pBuffer;
	}

	void HardwareBufferManager::DestroyVertexBuffer(VertexBuffer * pBuffer)
	{
		m_listVertexBuffer.erase(pBuffer);
		SAFE_DELETE(pBuffer);
	}

	IndexBuffer * HardwareBufferManager::CreateIndexBuffer(IndexType indexType, unsigned int numIndexes)
	{
		IndexBuffer* pBuffer = Root::GetSingleton().GetRenderer()->CreateIndexBuffer(indexType, numIndexes);
		m_listIndexBuffer.insert(pBuffer);
		return pBuffer;
	}

	void HardwareBufferManager::DestroyIndexBuffer(IndexBuffer * pBuffer)
	{
		m_listIndexBuffer.erase(pBuffer);
		SAFE_DELETE(pBuffer);
	}
}