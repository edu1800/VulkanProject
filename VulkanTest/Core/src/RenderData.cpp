#include "../include/RenderData.h"
#include "../include/GameObject.h"
#include "../include/GameObjectManager.h"
#include "../include/MeshManager.h"
#include "../include/MeshRenderer.h"
#include "../include/Mesh.h"
#include "../include/Material.h"
#include "../include/VertexIndexData.h"
#include "../include/VertexBuffer.h"
#include "../include/Camera.h"
#include "../include/Transform.h"
#include <map>
#include <algorithm>

namespace Vulkan
{
	struct MeshObject
	{
		MeshRenderer* pMesh;
		SubMesh* pSubMesh;
		bool isStatic;
		MeshObject(MeshRenderer* mesh, SubMesh* subMesh, bool bStatic)
		{
			pMesh = mesh;
			pSubMesh = subMesh;
			isStatic = bStatic;
		}
	};

	struct MeshObjectData
	{
		unsigned int vertexCount;
		unsigned int indexCount;
		VertexDeclaration* pDeclaration;
		std::vector<MeshObject> objects;
		bool hasIndexBuffer;
	};

	struct DepthSortDescendingLess
	{
		const Camera* pCamera;
		DepthSortDescendingLess(const Camera* cam)
			:pCamera(cam)
		{

		}

		bool operator() (const TransparantRenderMeshData& a, const TransparantRenderMeshData& b) const
		{
			if (a == b)
			{
				return false;
			}

			glm::vec3 aPos = a.mesh.pMeshRenderer->GetGameObject()->GetTransform()->GetPosition();
			glm::vec3 bPos = b.mesh.pMeshRenderer->GetGameObject()->GetTransform()->GetPosition();
			glm::vec3 camPos = pCamera->GetGameObject()->GetTransform()->GetPosition();
			float aDepth = glm::distance2(aPos, camPos);
			float bDepth = glm::distance2(bPos, camPos);
			if (glm::distance2(aDepth, bDepth) <= glm::epsilon<float>())
			{
				return false;
			}
			else
			{
				return aDepth > bDepth;
			}
		}
	};

	RenderData::RenderData()
	{
	}

	RenderData::~RenderData()
	{
	}

	void RenderData::Init()
	{
		std::map<Material*, std::vector<MeshObjectData>> mapObjects;
		std::map<Material*, std::vector<MeshObjectData>> mapTransparacy;

		const ResourceManager::ResourceMap resourceMap = GameObjectManager::GetSingleton().GetAllResources();
		ResourceManager::ResourceMap::const_iterator itr = resourceMap.begin();
		ResourceManager::ResourceMap::const_iterator itrEnd = resourceMap.end();

		for (; itr != itrEnd; ++itr)
		{
			GameObject* go = static_cast<GameObject*>(itr->second);
			MeshRenderer* pRenderer = go->GetComponent<MeshRenderer>();

			if (pRenderer == NULL || pRenderer->GetMesh() == NULL)
			{
				continue;
			}

			Mesh* pMesh = pRenderer->GetMesh();
			unsigned int subMeshCount = pMesh->GetSubMeshCount();
			for (size_t i = 0; i < subMeshCount; i++)
			{
				SubMesh* pSubMesh = pMesh->GetSubMesh(i);
				Material* pMaterial = pRenderer->GetMaterial(pSubMesh->materialIndex);
				if (pMaterial == NULL)
				{
					continue;
				}

				if (pMaterial->IsOpaque())
				{
					if (mapObjects.find(pMaterial) == mapObjects.end())
					{
						mapObjects.insert(std::pair<Material*, std::vector<MeshObjectData>>(pMaterial, std::vector<MeshObjectData>()));
					}
				}
				else
				{
					if (mapTransparacy.find(pMaterial) == mapTransparacy.end())
					{
						mapTransparacy.insert(std::pair<Material*, std::vector<MeshObjectData>>(pMaterial, std::vector<MeshObjectData>()));
					}
				}

				VertexDeclaration* pDecl = pMesh->GetVertexData()->GetVertexDeclaration();
				std::vector<MeshObjectData>& objectData = pMaterial->IsOpaque() ? mapObjects[pMaterial] : mapTransparacy[pMaterial];
				std::vector<MeshObjectData>::iterator oItr = objectData.begin();
				std::vector<MeshObjectData>::iterator oItrEnd = objectData.end();
				MeshObjectData* pMeshObjectData = NULL;
				for (; oItr != oItrEnd; ++oItr)
				{
					if (*oItr->pDeclaration == *pDecl && oItr->hasIndexBuffer == (pSubMesh->m_pIndexData->GetIndexCount() > 0))
					{
						pMeshObjectData = &(*oItr);
						break;
					}
				}

				if (pMeshObjectData == NULL)
				{
					objectData.push_back(MeshObjectData());
					pMeshObjectData = &objectData.back();
					pMeshObjectData->hasIndexBuffer = pSubMesh->m_pIndexData->GetIndexCount() > 0;
					pMeshObjectData->indexCount = 0;
					pMeshObjectData->pDeclaration = pMesh->GetVertexData()->GetVertexDeclaration();
					pMeshObjectData->vertexCount = 0;
				}

				if (go->GetStatic())
				{
					pMeshObjectData->indexCount += pSubMesh->m_pIndexData->GetIndexCount();
					pMeshObjectData->vertexCount += pMesh->GetVertexCount();
				}

				pMeshObjectData->objects.push_back(MeshObject(pRenderer, pSubMesh, go->GetStatic()));
			}
		}

		m_mapRenderData.clear();
		const CameraList& cameraList = GameObjectManager::GetSingleton().GetCameraList();
		CameraList::const_iterator cItr = cameraList.begin();
		CameraList::const_iterator cItrEnd = cameraList.end();
		for (; cItr != cItrEnd; ++cItr)
		{
			Camera* pCamera = *cItr;
			m_mapRenderData.insert(std::pair<Camera*, RenderDataList>(pCamera, RenderDataList()));
			RenderDataList& renderDataList = m_mapRenderData[pCamera];

			std::map<Material*, std::vector<MeshObjectData>>::iterator oItr = mapObjects.begin();
			std::map<Material*, std::vector<MeshObjectData>>::iterator oItrEnd = mapObjects.end();
			for (; oItr != oItrEnd; ++oItr)
			{
				Material* pMaterial = oItr->first;
				
				renderDataList.push_back(RenderMeshData());
				RenderMeshData& data = renderDataList.back();
				data.pMaterial = pMaterial;
				bool needCreateMergeMesh = false;
				std::vector<MeshObjectData>::iterator mItr = oItr->second.begin();
				std::vector<MeshObjectData>::iterator mItrEnd = oItr->second.end();
				for (; mItr != mItrEnd; ++mItr)
				{
					/*unsigned int vertexSize = mItr->pDeclaration->GetVertexSize(0);
					unsigned int indexSize = sizeof(unsigned short);
					char* vertexData = mItr->vertexCount > 0 ? (char*)malloc(mItr->vertexCount * vertexSize) : NULL;
					char* indexData = mItr->indexCount > 0 ? (char*)malloc(mItr->indexCount * indexSize) : NULL;*/
					std::vector<MeshObject>::iterator moItr = mItr->objects.begin();
					std::vector<MeshObject>::iterator moItrEnd = mItr->objects.end();
					for (; moItr != moItrEnd; ++moItr)
					{
						//if (moItr->isStatic)
						if (false)
						{
							/*needCreateMergeMesh = true;
							if (vertexData != NULL)
							{
							unsigned int copySize = vertexSize * moItr->pMesh->GetVertexCount();
							memcpy(vertexData, moItr->pMesh->GetVertexBufferData(), copySize);
							vertexData += copySize;
							}

							if (indexData != NULL)
							{
							unsigned int copySize = indexSize * moItr->pMesh->GetIndexData()->GetIndexCount();
							memcpy(indexData, moItr->pMesh->GetIndexBufferData(), copySize);
							indexData += copySize;
							}*/
						}
						else
						{
							if (pCamera->IsRender(moItr->pMesh))
							{
								data.listMesh.push_back(SubMeshData(moItr->pSubMesh, moItr->pMesh));
							}
						}
					}
				}

				if (needCreateMergeMesh)
				{

				}
			}

			m_mapTransparantData.insert(std::pair<Camera*, TransparantRenderDataList>(pCamera, TransparantRenderDataList()));
			TransparantRenderDataList& transparantRenderDataList = m_mapTransparantData[pCamera];

			oItr = mapTransparacy.begin();
			oItrEnd = mapTransparacy.end();
			for (; oItr != oItrEnd; ++oItr)
			{
				Material* pMaterial = oItr->first;
				std::vector<MeshObjectData>::iterator mItr = oItr->second.begin();
				std::vector<MeshObjectData>::iterator mItrEnd = oItr->second.end();
				for (; mItr != mItrEnd; ++mItr)
				{
					std::vector<MeshObject>::iterator moItr = mItr->objects.begin();
					std::vector<MeshObject>::iterator moItrEnd = mItr->objects.end();
					for (; moItr != moItrEnd; ++moItr)
					{
						if (pCamera->IsRender(moItr->pMesh))
						{
							transparantRenderDataList.push_back(TransparantRenderMeshData(pMaterial, moItr->pSubMesh, moItr->pMesh));
						}
					}
				}
			}
		}
	}

	RenderDataList* RenderData::GetRenderDataList(Camera * pCamera)
	{
		if (m_mapRenderData.find(pCamera) == m_mapRenderData.end())
		{
			return NULL;
		}

		return &m_mapRenderData[pCamera];
	}

	TransparantRenderDataList* RenderData::GetTransparantList(Camera * pCamera)
	{
		if (m_mapTransparantData.find(pCamera) == m_mapTransparantData.end())
		{
			return NULL;
		}

		return &m_mapTransparantData[pCamera];
	}

	void RenderData::SortTranspanctObjects(Camera * pCamera)
	{
		TransparantRenderDataList* transList = GetTransparantList(pCamera);
		if (transList != NULL)
		{
			std::sort(transList->begin(), transList->end(), DepthSortDescendingLess(pCamera));
		}
	}
}