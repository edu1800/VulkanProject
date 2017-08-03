#ifndef _MESH_H_
#define _MESH_H_

#pragma once

#include "Resource.h"
#include <vector>
#include "VertexIndexDefine.h"
#include "CommonUtil.h"

namespace Vulkan
{
	class Material;
	class VertexBuffer;
	class IndexBuffer;
	class VertexData;
	class IndexData;

	class Mesh;
	struct SubMesh
	{
		SubMesh();
		~SubMesh();
		unsigned int subMeshIndex;
		unsigned int materialIndex;
		IndexData* m_pIndexData;
		void* m_pIndexBufferData;
		Mesh* pMesh;
	};

	typedef std::vector<SubMesh*> SubMeshList;

	class Mesh : public Resource
	{
	public:
		Mesh(ResourceManager* pResourceMgr, const std::string& name);
		virtual ~Mesh();

		virtual void Load();
		virtual void UnLoad();

		VertexBuffer* CreateVertexBuffer(unsigned int u32VertexCount, unsigned int u32VertexSize, unsigned short bindingIndex = 0);
		IndexBuffer* CreateIndexBuffer(IndexType indexType, unsigned int u32IndexCount, int subMeshIndex = 0);

		VertexData* GetVertexData() const { return m_pVertexData; }
		unsigned int GetVertexCount();
		unsigned int GetVertexSize();

		void* GetVertexBufferData() const { return m_pVertexBufferData; }
		void SetVertexBufferData(void* data) { m_pVertexBufferData = data; }

		unsigned int GetSubMeshCount() { return m_listSubMesh.size(); }
		SubMesh* GetSubMesh(int index) { return m_listSubMesh[index]; }
		SubMesh* CreateSubMesh();

	private:
		VertexData* m_pVertexData;

		SubMeshList m_listSubMesh;
		void* m_pVertexBufferData;
	};
}


#endif
