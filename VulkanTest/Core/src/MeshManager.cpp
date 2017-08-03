#include "../include/MeshManager.h"
#include "../include/Mesh.h"

namespace Vulkan
{
	SINGLETON_IMPL(MeshManager)
	MeshManager::MeshManager()
		:ResourceManager()
	{
	}

	MeshManager::~MeshManager()
	{
	}

	Mesh * MeshManager::CreateMesh(const std::string & name, unsigned int numVertices, unsigned int vertexSize, void * pData)
	{
		Mesh* pMesh = static_cast<Mesh*>(MeshManager::GetSingleton().Create(name));
		pMesh->SetVertexBufferData(pData);
		VertexBuffer* pBuffer = pMesh->CreateVertexBuffer(numVertices, vertexSize);
		return pMesh;
	}

	Mesh * MeshManager::CreateMesh(const std::string & name, unsigned int numVertices, unsigned int vertexSize, void * pVertexData,
								   IndexType indexType, unsigned int numIndexes, void* pIndexData)
	{
		Mesh* pMesh = CreateMesh(name, numVertices, vertexSize, pVertexData);
		SubMesh* pSubMesh = pMesh->CreateSubMesh();
		pMesh->CreateIndexBuffer(indexType, numIndexes, pSubMesh->subMeshIndex);
		pSubMesh->m_pIndexBufferData = pIndexData;
		return pMesh;
	}

	Resource* MeshManager::createImpl(const std::string& name)
	{
		return new Mesh(this, name);
	}
}

