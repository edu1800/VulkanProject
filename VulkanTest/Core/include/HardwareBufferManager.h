#ifndef _HARDWARE_BUFFER_MANAGER_H_
#define _HARDWARE_BUFFER_MANAGER_H_

#pragma once

#include "Singleton.h"
#include "CommonUtil.h"
#include "VertexIndexDefine.h"
#include <set>

namespace Vulkan
{
	class VertexBuffer;
	class IndexBuffer;
	class HardwareBufferManager : public Singleton<HardwareBufferManager>
	{
	public:
		HardwareBufferManager();
		virtual ~HardwareBufferManager();

		VertexBuffer* CreateVertexBuffer(unsigned int vertexSize, unsigned int numVerts);
		void DestroyVertexBuffer(VertexBuffer* pBuffer);

		IndexBuffer* CreateIndexBuffer(IndexType indexType, unsigned int numIndexes);
		void DestroyIndexBuffer(IndexBuffer* pBuffer);

		SINGLETON_DECLARE(HardwareBufferManager)
	private:
		typedef std::set<VertexBuffer*> VertexBufferList;
		VertexBufferList m_listVertexBuffer;

		typedef std::set<IndexBuffer*> IndexBufferList;
		IndexBufferList m_listIndexBuffer;
	};
}

#endif	
