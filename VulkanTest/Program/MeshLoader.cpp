#include "MeshLoader.h"
#include <vector>
#include "../Core/include/GameObject.h"
#include "../Core/include/GameObjectManager.h"
#include "../Core/include/Mesh.h"
#include "../Core/include/MeshRenderer.h"
#include "../Core/include/MeshManager.h"
#include "../Core/include/VertexIndexData.h"
#include "../Core/include/Material.h"
#include "../Core/include/MaterialManager.h"
#include "../Core/include/Shader.h"
#include "../Core/include/ShaderManager.h"
#include "../Core/include/Texture.h"
#include "../Core/include/TextureManager.h"

#define TINYOBJLOADER_IMPLEMENTATION 
#include "../ThirdParty/tinyobjloader/tiny_obj_loader.h"
#include "../ThirdParty/glm/vec3.hpp"
#include <set>
#include <map>

namespace Vulkan
{
	struct VertexNormalUV
	{
		float posX, posY, posZ;
		float normalX, normalY, normalZ;
		float u, v;
		VertexNormalUV()
		{
			memset(this, 0, sizeof(VertexNormalUV));
		}
	};

	struct IndexDataOffset
	{
		unsigned int begin;
		unsigned int end;
		unsigned int materialIndex;
	};

	MeshLoader::MeshLoader()
	{
		
	}

	MeshLoader::~MeshLoader()
	{
	}

	void SetTextureFormat(Texture* pTexture)
	{
		pTexture->SetAddressModeU(TAM_WRAP);
		pTexture->SetAddressModeV(TAM_WRAP);
		pTexture->SetAddressModeW(TAM_WRAP);
		pTexture->SetEnableAnisotropy(true);
		pTexture->SetMaxAnisotropy(2);
		pTexture->SetMinFilter(TF_NEAREST);
		pTexture->SetMagFilter(TF_NEAREST);
	}

	void BuildMaterial(const std::vector<tinyobj::material_t> materials, const std::string& textureFolder)
	{
		Shader* pVertexShader = static_cast<Shader*>(ShaderManager::GetSingleton().GetResource("TextureVertex"));
		Shader* pFragShader = static_cast<Shader*>(ShaderManager::GetSingleton().GetResource("TextureFrag"));
		Shader* pFragAlphaShader = static_cast<Shader*>(ShaderManager::GetSingleton().GetResource("TextureAlphaFrag"));
		Shader* pFragNormalShader = static_cast<Shader*>(ShaderManager::GetSingleton().GetResource("TextureNoramlFrag"));
		Shader* pFragAlphaNormalShader = static_cast<Shader*>(ShaderManager::GetSingleton().GetResource("TextureAlphaNormalFrag"));
		for (size_t i = 0; i < materials.size(); i++)
		{
			tinyobj::material_t mat = materials[i];
			if (mat.diffuse_texname != "")
			{
				Material* pMaterial = static_cast<Material*>(MaterialManager::GetSingleton().Create(mat.name));
				if (mat.dissolve < 1.0f || mat.alpha_texname != "")
				{
					pMaterial->SetSceneBlending(SBF_SRC_ALPHA, SBF_ONE_MINUS_SRC_ALPHA);
					pMaterial->EnableDepthWrite(false);
				}
				
				Texture* pTexture = static_cast<Texture*>(TextureManager::GetSingleton().GetResource(mat.diffuse_texname));
				if (pTexture == NULL)
				{
					pTexture = static_cast<Texture*>(TextureManager::GetSingleton().Create(mat.diffuse_texname, textureFolder + mat.diffuse_texname));
					SetTextureFormat(pTexture);
				}

				pMaterial->AddTexture(pTexture);

				pMaterial->AddShader(pVertexShader);
				ShaderParameters* pFragParam = NULL;
				if (mat.alpha_texname == "")
				{
					if (mat.bump_texname != "")
					{
						Texture* pNormalTexture = static_cast<Texture*>(TextureManager::GetSingleton().GetResource(mat.bump_texname));
						if (pNormalTexture == NULL)
						{
							pNormalTexture = static_cast<Texture*>(TextureManager::GetSingleton().Create(mat.bump_texname, textureFolder + mat.bump_texname));
							SetTextureFormat(pNormalTexture);
						}

						pMaterial->AddTexture(pNormalTexture);
						pMaterial->AddShader(pFragNormalShader);
						pFragParam = pFragNormalShader->GetShaderParameters();
					}
					else
					{
						pFragParam = pFragShader->GetShaderParameters();
						pMaterial->AddShader(pFragShader);
					}
				}
				else
				{
					if (mat.bump_texname != "")
					{
						Texture* pNormalTexture = static_cast<Texture*>(TextureManager::GetSingleton().GetResource(mat.bump_texname));
						if (pNormalTexture == NULL)
						{
							pNormalTexture = static_cast<Texture*>(TextureManager::GetSingleton().Create(mat.bump_texname, textureFolder + mat.bump_texname));
							SetTextureFormat(pNormalTexture);
						}

						pMaterial->AddTexture(pNormalTexture);
						pMaterial->AddShader(pFragAlphaNormalShader);
						pFragParam = pFragAlphaNormalShader->GetShaderParameters();
					}
					else
					{
						pFragParam = pFragAlphaShader->GetShaderParameters();
						pMaterial->AddShader(pFragAlphaShader);
					}

					Texture* pAlphaTexture = static_cast<Texture*>(TextureManager::GetSingleton().GetResource(mat.alpha_texname));
					if (pAlphaTexture == NULL)
					{
						pAlphaTexture = static_cast<Texture*>(TextureManager::GetSingleton().Create(mat.alpha_texname, textureFolder + mat.alpha_texname));
						SetTextureFormat(pAlphaTexture);
					}

					pMaterial->AddTexture(pAlphaTexture);
				}

				pFragParam->SetParameter("Ka", glm::vec3(mat.ambient[0], mat.ambient[1], mat.ambient[2]));
				pFragParam->SetParameter("Kd", glm::vec3(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]));
				pFragParam->SetParameter("Ks", glm::vec3(mat.specular[0], mat.specular[1], mat.specular[2]));
				pFragParam->SetParameter("ns", mat.shininess);
			}
		}
	}

	void createShader(const char* vertexShaderName, const char* fileName, ShaderType type)
	{
		Shader* pShader = static_cast<Shader*>(ShaderManager::GetSingleton().Create(vertexShaderName, fileName));
		pShader->SetEntryName("main");
		pShader->SetShaderType(type);

		if (type == FRAGMENT)
		{
			ShaderParameters* pParam = pShader->GetShaderParameters();
			pParam->AddParamter("Ka", GCT_FLOAT3, 1);
			pParam->AddParamter("Kd", GCT_FLOAT3, 1);
			pParam->AddParamter("Ks", GCT_FLOAT3, 1);
			pParam->AddParamter("ns", GCT_FLOAT1, 1);
		}
		else if (type == VERTEX)
		{
			ShaderParameters* pParam = pShader->GetUniformShaderParameters();
			pParam->RemoveAll();
			pParam->AddParamter("localToWorld", GCT_MATRIX_4X4, 1);
			pParam->AddParamter("viewMatrix", GCT_MATRIX_4X4, 1);
			pParam->AddParamter("projMatrix", GCT_MATRIX_4X4, 1);
			pParam->AddParamter("LightMatrix", GCT_MATRIX_4X4, 1);
			pParam->AddParamter("camWorldPos", GCT_FLOAT3, 1);
		}
	}

	void MeshLoader::Init()
	{
		createShader("TextureVertex", "../Resource/TextureVertex.vert", VERTEX);
		createShader("TextureFrag", "../Resource/TextureFrag.frag", FRAGMENT);
		createShader("TextureAlphaFrag", "../Resource/TextureAlphaFrag.frag", FRAGMENT);
		createShader("TextureNoramlFrag", "../Resource/TextureNormalFrag.frag", FRAGMENT);
		createShader("TextureAlphaNormalFrag", "../Resource/TextureAlphaNormalFrag.frag", FRAGMENT);
	}

	void updateBounds(glm::vec3& minVerex, glm::vec3& maxVertex, float posX, float posY, float posZ)
	{
		if (posX < minVerex.x)
		{
			minVerex.x = posX;
		}

		if (posY < minVerex.y)
		{
			minVerex.y = posY;
		}

		if (posZ < minVerex.z)
		{
			minVerex.z = posZ;
		}

		if (posX > maxVertex.x)
		{
			maxVertex.x = posX;
		}

		if (posY > maxVertex.y)
		{
			maxVertex.y = posY;
		}

		if (posZ > maxVertex.z)
		{
			maxVertex.z = posZ;
		}
	}

	GameObject * MeshLoader::LoadScene(const std::string & name)
	{
		std::string folder = name.substr(0, name.find_last_of('/') + 1);
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string err;
		bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, name.c_str(), folder.c_str());
		if (!ret)
		{
			return NULL;
		}

		BuildMaterial(materials, folder);

		GameObject* root = GameObjectManager::GetSingleton().CreateGameObject(name);
		std::vector<unsigned short> indexList;
		std::vector<VertexNormalUV> vertexData;
		std::vector<unsigned int> indexData;
		std::vector<IndexDataOffset> indexDataOffsetList;

		for (size_t s = 0; s < shapes.size(); s++)
		{
			size_t indexOffset = 0;
			GameObject* pMeshGo = GameObjectManager::GetSingleton().CreateGameObject(shapes[s].name);
			pMeshGo->SetParent(root);
			pMeshGo->SetStatic(true);
			pMeshGo->SetLayer(0xFFFFFFFFFFFFFFFF);

			MeshRenderer* pRenderer = pMeshGo->AddComponent<MeshRenderer>();
			pRenderer->SetCastShadow(true);
			std::map<std::tuple<int, int, int>, unsigned short> vertexToIndexMap;
			vertexData.clear();
			indexData.clear();
			indexDataOffsetList.clear();
			IndexDataOffset* indexDataOffset = NULL;
			int materialId = -1;

			glm::vec3 minVertex(10000, 10000, 10000);
			glm::vec3 maxVertex(-10000, -10000, -10000);

			for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
			{
				indexList.clear();
				int curMaterialId = shapes[s].mesh.material_ids[f];
				if (curMaterialId != materialId)
				{
					materialId = curMaterialId;
					if (indexDataOffset != NULL)
					{
						indexDataOffset->end = indexData.size() - 1;
					}
					indexDataOffsetList.push_back(IndexDataOffset());
					indexDataOffset = &indexDataOffsetList.back();
					indexDataOffset->begin = indexData.size();
					indexDataOffset->materialIndex = materialId;
				}

				int fv = shapes[s].mesh.num_face_vertices[f];
				for (size_t v = 0; v < fv; v++)
				{
					tinyobj::index_t idx = shapes[s].mesh.indices[indexOffset + v];
					if (vertexToIndexMap.find(std::tuple<int, int, int>(idx.vertex_index, idx.normal_index, idx.texcoord_index)) == vertexToIndexMap.end())
					{
						VertexNormalUV vertex;
						if (idx.vertex_index < 0)
						{
							continue;
						}

						vertex.posX = attrib.vertices[3 * idx.vertex_index + 0];
						vertex.posY = attrib.vertices[3 * idx.vertex_index + 1];
						vertex.posZ = attrib.vertices[3 * idx.vertex_index + 2];

						updateBounds(minVertex, maxVertex, vertex.posX, vertex.posY, vertex.posZ);

						if (idx.normal_index >= 0)
						{
							vertex.normalX = attrib.normals[3 * idx.normal_index + 0];
							vertex.normalY = attrib.normals[3 * idx.normal_index + 1];
							vertex.normalZ = attrib.normals[3 * idx.normal_index + 2];
						}
						
						if (idx.texcoord_index >= 0)
						{
							vertex.u = attrib.texcoords[2 * idx.texcoord_index + 0];
							vertex.v = 1.0f - attrib.texcoords[2 * idx.texcoord_index + 1];
						}

						vertexToIndexMap[std::tuple<int, int, int>(idx.vertex_index, idx.normal_index, idx.texcoord_index)] = vertexData.size();
						indexList.push_back(vertexData.size());
						vertexData.push_back(vertex);
					}
					else
					{
						indexList.push_back(vertexToIndexMap[std::tuple<int, int, int>(idx.vertex_index, idx.normal_index, idx.texcoord_index)]);
					}			
				}

				for (size_t i = 0; i < indexList.size(); i++)
				{
					if (i < 3)
					{
						indexData.push_back(indexList[i]);
					}
					else
					{
						indexData.push_back(indexList[0]);
						indexData.push_back(indexList[i - 1]);
						indexData.push_back(indexList[i]);
					}
				}

				indexOffset += fv;
			}

			indexDataOffset->end = indexData.size() - 1;

			VertexNormalUV* vetexBuffer = new VertexNormalUV[vertexData.size()];
			memcpy(vetexBuffer, &vertexData[0], sizeof(VertexNormalUV) * vertexData.size());

			Mesh* pMesh = MeshManager::CreateMesh(shapes[s].name, vertexData.size(), sizeof(VertexNormalUV), (void*)vetexBuffer);
			
			float radius = glm::distance(maxVertex, minVertex) * 0.5f;
			pMeshGo->SetRadius(radius);
			pMeshGo->SetCenter((maxVertex + minVertex) * 0.5f);
			pRenderer->SetMesh(pMesh);

			for (size_t i = 0; i < indexDataOffsetList.size(); i++)
			{
				IndexDataOffset& data = indexDataOffsetList[i];
				unsigned int indexCount = data.end - data.begin + 1;
				IndexType indexType = indexCount > 65535 ? IT_32BIT : IT_16BIT;
				unsigned int indexSize = indexType == IT_32BIT ? sizeof(unsigned int) : sizeof(unsigned short);
				void* indexBuffer = NULL;
				if (indexType == IT_16BIT)
				{
					unsigned short* pBuffer = new unsigned short[indexCount];
					for (int j = data.begin; j <= data.end; ++j)
					{
						pBuffer[j - data.begin] = (unsigned short)indexData[j];
					}

					indexBuffer = pBuffer;
				}
				else
				{
					indexBuffer = new unsigned int[indexCount];
					memcpy(indexBuffer, &indexData[data.begin], indexSize * indexCount);
				}

				SubMesh* pSubMesh = pMesh->CreateSubMesh();
				pSubMesh->materialIndex = i;
				pSubMesh->m_pIndexData = new IndexData(indexType, indexCount);
				pSubMesh->m_pIndexBufferData = indexBuffer;

				tinyobj::material_t mat = materials[data.materialIndex];
				Material* pMaterial = MaterialManager::GetSingleton().GetDefaultMaterial();
				if (MaterialManager::GetSingleton().IsContain(mat.name))
				{
					pMaterial = static_cast<Material*>(MaterialManager::GetSingleton().GetResource(mat.name));
				}

				if (i == 0)
				{
					pRenderer->SetMaterial(pMaterial, 0);
				}
				else
				{
					pRenderer->AddMaterial(pMaterial);
				}
			}
		}

		return root;	
	}
}