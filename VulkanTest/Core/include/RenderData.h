#ifndef _RENDER_DATA_H_
#define _RENDER_DATA_H_

#pragma once

#include <vector>
#include <map>

namespace Vulkan
{
	class Material;
	struct SubMesh;
	class MeshRenderer;
	class Camera;
	struct SubMeshData
	{
		SubMesh* pSubMesh;
		MeshRenderer* pMeshRenderer;
		bool isDirty;

		SubMeshData()
			:pSubMesh(NULL)
			,pMeshRenderer(NULL)
			,isDirty(true)
		{

		}

		SubMeshData(SubMesh* subMesh, MeshRenderer* meshRenderer)
		{
			pSubMesh = subMesh;
			pMeshRenderer = meshRenderer;
			isDirty = true;
		}

		bool operator==(const SubMeshData& a) const 
		{
			return pSubMesh == a.pSubMesh && pMeshRenderer == a.pMeshRenderer;
		}

		void SetDirty(bool dirty)
		{
			isDirty = dirty;
		}
	};
	struct RenderMeshData
	{
		Material* pMaterial;
		std::vector<SubMeshData> listMesh;
	}; 

	struct TransparantRenderMeshData
	{
		Material* pMaterial;
		SubMeshData mesh;

		TransparantRenderMeshData(Material* mat, SubMesh* subMesh, MeshRenderer* mr)
			:pMaterial(mat)
		{
			mesh.pSubMesh = subMesh;
			mesh.pMeshRenderer = mr;
		}

		bool operator==(const TransparantRenderMeshData& a) const
		{
			return pMaterial == a.pMaterial && mesh == a.mesh;
		}
	};

	typedef std::vector<RenderMeshData> RenderDataList;
	typedef std::vector<TransparantRenderMeshData> TransparantRenderDataList;
	typedef std::map<Camera*, RenderDataList> RenderDataCameraMap;
	typedef std::map<Camera*, TransparantRenderDataList> TransparantRenderDataCameraMap;
	class RenderData
	{
	public:
		RenderData();
		~RenderData();

		void Init();

		RenderDataList* GetRenderDataList(Camera* pCamera);
		TransparantRenderDataList* GetTransparantList(Camera* pCamera);

		void SortTranspanctObjects(Camera* pCamera);

	private:
		RenderDataCameraMap m_mapRenderData;
		TransparantRenderDataCameraMap m_mapTransparantData;
	};
}

#endif
