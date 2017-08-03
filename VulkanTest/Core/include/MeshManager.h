#ifndef _MESH_MANAGER_H_
#define _MESH_MANAGER_H_

#pragma once

#include "ResourceManager.h"
#include "Singleton.h"
#include "CommonUtil.h"
#include "VertexIndexDefine.h"

namespace Vulkan
{
	class Mesh;
	class MeshManager : public ResourceManager, public Singleton<MeshManager>
	{
	public:
		MeshManager();
		virtual ~MeshManager();

		static Mesh* CreateMesh(const std::string & name, unsigned int numVertices, unsigned int vertexSize, void * pData);
		static Mesh* CreateMesh(const std::string& name, unsigned int numVertices, unsigned int vertexSize, void* pVertexData,
								IndexType indexType, unsigned int numIndexes, void* pIndexData);

		SINGLETON_DECLARE(MeshManager)
	protected:
		virtual Resource* createImpl(const std::string& name);
	};
}

#endif
