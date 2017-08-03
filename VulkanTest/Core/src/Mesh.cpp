#include "../include/Mesh.h"
#include "../include/CommonUtil.h"
#include "../include/VertexBuffer.h"
#include "../include/VertexIndexData.h"
#include "../include/HardwareBufferManager.h"
#include "../include/MaterialManager.h"

namespace Vulkan
{
	Mesh::Mesh(ResourceManager * pResourceMgr, const std::string & name)
		:Resource(pResourceMgr, name)
		,m_pVertexBufferData(NULL)
	{
		m_eType = MESH;
		m_pVertexData = new VertexData();
	}

	Mesh::~Mesh()
	{
		SAFE_DELETE(m_pVertexBufferData);
		SAFE_DELETE(m_pVertexData);
		VEC_DELETE_POINTER(m_listSubMesh);
	}

	void Mesh::Load()
	{
	}

	void Mesh::UnLoad()
	{
	}

	VertexBuffer* Mesh::CreateVertexBuffer(unsigned int u32VertexCount, unsigned int u32VertexSize, unsigned short bindingIndex)
	{
		VertexBufferBinding* pBinding = m_pVertexData->GetVertexBinding();
		VertexBuffer* pBuffer = pBinding->GetBuffer(bindingIndex);
		if (pBuffer != NULL)
		{
			if (pBuffer->GetNumVertices() != u32VertexCount || pBuffer->GetVertexSize() != u32VertexSize)
			{
				HardwareBufferManager::GetSingleton().DestroyVertexBuffer(pBuffer);
				pBuffer = NULL;
			}
		}

		if (pBuffer == NULL)
		{
			pBuffer = HardwareBufferManager::GetSingleton().CreateVertexBuffer(u32VertexSize, u32VertexCount);
		}
		
		pBinding->SetBinding(bindingIndex, pBuffer);
		m_pVertexData->GetVertexDeclaration()->SetBindingIndex(0);
		return pBuffer;
	}

	IndexBuffer * Mesh::CreateIndexBuffer(IndexType indexType, unsigned int u32IndexCount, int subMeshIndex)
	{
		SubMesh* pSubMesh = m_listSubMesh[subMeshIndex];
		if (pSubMesh->m_pIndexData != NULL)
		{
			SAFE_DELETE(pSubMesh->m_pIndexData);
		}
		
		pSubMesh->m_pIndexData = new IndexData(indexType, u32IndexCount);

		return pSubMesh->m_pIndexData->GetIndexBuffer();
	}

	unsigned int Mesh::GetVertexCount()
	{
		unsigned int vertexCount = 0;
		VertexBufferBinding* pBinding = m_pVertexData->GetVertexBinding();
		const VertexBufferBinding::VertexBufferBindingMap& bindings = pBinding->GetBindings();
		VertexBufferBinding::VertexBufferBindingMap::const_iterator itr = bindings.begin();
		VertexBufferBinding::VertexBufferBindingMap::const_iterator itrEnd = bindings.end();
		for (; itr != itrEnd; ++itr)
		{
			vertexCount += itr->second->GetNumVertices();
		}

		return vertexCount;
	}

	unsigned int Mesh::GetVertexSize()
	{
		VertexBufferBinding* pBinding = m_pVertexData->GetVertexBinding();
		VertexBuffer* pBuffer = pBinding->GetBuffer(0);
		if (pBuffer == NULL)
		{
			return 0;
		}

		return pBuffer->GetVertexSize();
	}

	SubMesh * Mesh::CreateSubMesh()
	{
		SubMesh* pSubMesh = new SubMesh();
		pSubMesh->subMeshIndex = m_listSubMesh.size();
		pSubMesh->pMesh = this;
		m_listSubMesh.push_back(pSubMesh);
		return pSubMesh;
	}

	SubMesh::SubMesh()
		:subMeshIndex(0)
		,materialIndex(0)
		,m_pIndexBufferData(NULL)
		,m_pIndexData(NULL)
		,pMesh(NULL)
	{
	}

	SubMesh::~SubMesh()
	{
		SAFE_DELETE(m_pIndexBufferData);
		SAFE_DELETE(m_pIndexData);
	}
}

