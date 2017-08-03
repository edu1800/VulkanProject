#include "VulkanRenderer.h"
#include "VulkanUtil.h"
#include "../Core/include/CommonUtil.h"
#include <iostream>
#include "../Core/include/GameObject.h"
#include "../Core/include/GameObjectManager.h"
#include "../Core/include/MeshRenderer.h"
#include "../Core/include/MaterialManager.h"
#include "../Core/include/Mesh.h"
#include "VulkanVertexBuffer.h"
#include "VulkanIndexBuffer.h"
#include "VulkanGameObject.h"
#include "VulkanShader.h"
#include "VulkanTexture.h"
#include "../Core/include/RenderData.h"
#include "../Core/include/Frustum.h"
#include "../Core/include/Camera.h"
#include "../Core/include/Transform.h"

namespace Vulkan
{
	VulkanRenderer::VulkanRenderer()
		:Renderer()
		,m_pVulkanData(NULL)
		,m_bIsInitBeforeRender(false)
	{
		m_pRenderData = new RenderData();
	}

	VulkanRenderer::~VulkanRenderer()
	{
		SAFE_DELETE(m_pRenderData);
		SAFE_DELETE(m_pVulkanData);
	}

	void VulkanRenderer::Initialize(const char * windowName, int width, int height)
	{
		Renderer::Initialize(windowName, width, height);

		m_pVulkanData = new VulkanData();
		VulkanUtil::CreateConnection(*m_pVulkanData);
		VulkanUtil::CreateDisplayWindow(*m_pVulkanData, windowName, width, height);
		VulkanUtil::AddInstanceExtensionName(*m_pVulkanData);
		VulkanUtil::AddDeviceExtensionName(*m_pVulkanData);
		VkResult result = VulkanUtil::CreateVkInstance(*m_pVulkanData, "TestSample", 1, "TestSample", 1);
		if (result == VK_ERROR_INCOMPATIBLE_DRIVER)
		{
			std::cout << "cannot find a compatible Vulkan ICD\n";
			exit(-1);
		}
		else if (result)
		{
			std::cout << "unkown error\n";
			exit(-1);
		}

		VulkanUtil::GetPhysicalDevices(*m_pVulkanData);
		VulkanUtil::CreateSurface(*m_pVulkanData);
		VulkanUtil::CreateDevice(*m_pVulkanData);
		VulkanUtil::CreateCommandPool(*m_pVulkanData);
		VulkanUtil::CreateDynamicState(*m_pVulkanData);
		VulkanUtil::CreateSwapchain(*m_pVulkanData);
		VulkanUtil::CreateDepthBuffer(*m_pVulkanData, 0);
		VulkanUtil::CreateFence(*m_pVulkanData);
	}

	void VulkanRenderer::initBeforeRender()
	{
		if (m_bIsInitBeforeRender)
		{
			return;
		}

		m_pRenderData->Init();
		VulkanUtil::CreateCompositors(*m_pVulkanData);
		VulkanUtil::CreateSemphore(*m_pVulkanData, GameObjectManager::GetSingleton().GetCameraList().size());
		CameraList cameraList = GameObjectManager::GetSingleton().GetCameraList();
		int commandBufferNum = cameraList.size();
		if (m_pVulkanData->pCompositor != NULL)
		{
			commandBufferNum += m_pVulkanData->pCompositor->listPassData.size();
		}
		VulkanUtil::CreateCommandBuffer(*m_pVulkanData, commandBufferNum);
		m_bIsInitBeforeRender = true;
	}

	bool VulkanRenderer::Render()
	{
		if (!VulkanUtil::OnWindowDrawing(*m_pVulkanData))
			return false;

		initBeforeRender();

		GameObjectManager::GetSingleton().SortCamera();

		CameraList cameraList = GameObjectManager::GetSingleton().GetCameraList();
		int commandBufferIndex = 0;

		CameraList::iterator cItr = cameraList.begin();
		CameraList::iterator cItrEnd = cameraList.end();

		for (int cameraIndex = 0; cItr != cItrEnd; ++cItr, ++cameraIndex)
		{
			Camera* pCamera = *cItr;

			VulkanUtil::BeingCommandBuffer(*m_pVulkanData, commandBufferIndex);
			VulkanUtil::CreateCompositorShaders(*m_pVulkanData, pCamera, commandBufferIndex);

			if (pCamera->GetRenderTexture() == NULL)
			{
				VulkanUtil::AttachToSwapchain(*m_pVulkanData, cameraIndex, commandBufferIndex);
			}

			VulkanUtil::BeginRenderPass(*m_pVulkanData, pCamera, commandBufferIndex);

			if (pCamera->GetRenderTexture() == NULL)
			{
				VulkanUtil::BindViewport(*m_pVulkanData, commandBufferIndex);
			}
			else
			{
				VulkanUtil::BindViewport(*m_pVulkanData, pCamera->GetRenderTexture()->GetWidth(), pCamera->GetRenderTexture()->GetHeight(), commandBufferIndex);
			}

			//bool isModifyCamera = pCamera->UpdateViewMatrix();
			pCamera->UpdateViewMatrix();
			bool isModifyCamera = true;
			Frustum frustum;
			frustum.Update(pCamera->GetProjectionMatrix() * pCamera->GetViewMatrix());

			Mesh* pPrevMesh = NULL;
			RenderDataList* renderDataList = m_pRenderData->GetRenderDataList(pCamera);
			if (renderDataList != NULL)
			{
				RenderDataList::iterator itr = renderDataList->begin();
				RenderDataList::iterator itrEnd = renderDataList->end();
				for (; itr != itrEnd; ++itr)
				{
					if (itr->listMesh.size() == 0)
					{
						continue;
					}

					Material* pMaterial = pCamera->GetReplaceMaterial() == NULL ? itr->pMaterial : pCamera->GetReplaceMaterial();
					GraphicsPipelineData* pGraphicsPipelineData = getGraphicsPipelineData(pMaterial);

					if (pGraphicsPipelineData->isInit)
					{
						VulkanUtil::BindGraphicsPipeline(*m_pVulkanData, pGraphicsPipelineData, commandBufferIndex);
					}

					std::vector<SubMeshData>::iterator rItr = itr->listMesh.begin();
					std::vector<SubMeshData>::iterator rItrEnd = itr->listMesh.end();
					for (; rItr != rItrEnd; ++rItr)
					{
						SubMeshData& subMeshData = *rItr;
						bindAndDrawSubMesh(pCamera, &subMeshData, pPrevMesh, &frustum, pGraphicsPipelineData, pMaterial, isModifyCamera, commandBufferIndex);
					}
				}
			}

			if (isModifyCamera)
			{
				m_pRenderData->SortTranspanctObjects(pCamera);
			}

			TransparantRenderDataList* tranRenderDataList = m_pRenderData->GetTransparantList(pCamera);
			if (tranRenderDataList != NULL)
			{
				TransparantRenderDataList::iterator tItr = tranRenderDataList->begin();
				TransparantRenderDataList::iterator tItrEnd = tranRenderDataList->end();
				for (; tItr != tItrEnd; ++tItr)
				{
					Material* pMaterial = pCamera->GetReplaceMaterial() == NULL ? tItr->pMaterial : pCamera->GetReplaceMaterial();
					GraphicsPipelineData* pGraphicsPipelineData = getGraphicsPipelineData(pMaterial);
					if (pGraphicsPipelineData->isInit)
					{
						VulkanUtil::BindGraphicsPipeline(*m_pVulkanData, pGraphicsPipelineData, commandBufferIndex);
					}

					SubMeshData& subMeshData = tItr->mesh;
					bindAndDrawSubMesh(pCamera, &subMeshData, pPrevMesh, &frustum, pGraphicsPipelineData, pMaterial, isModifyCamera, commandBufferIndex);
				}
			}

			VulkanUtil::EndRenderPass(*m_pVulkanData, commandBufferIndex);
			VulkanUtil::EndCommandBuffer(*m_pVulkanData, commandBufferIndex);

			if (m_pVulkanData->pCompositor != NULL && m_pVulkanData->pCompositor->listPassData.find(pCamera) != m_pVulkanData->pCompositor->listPassData.end())
			{
				VulkanUtil::SubmitQueue(*m_pVulkanData, m_pVulkanData->pSemaphore[cameraIndex], m_pVulkanData->pCompositor->pSemaphore, commandBufferIndex);
				commandBufferIndex++;
				VulkanUtil::DrawCompositors(*m_pVulkanData, pCamera, commandBufferIndex);
			}
			else
			{
				VulkanUtil::SubmitQueue(*m_pVulkanData, m_pVulkanData->pSemaphore[cameraIndex], 
									     cameraIndex < cameraList.size() - 1 ? m_pVulkanData->pSemaphore[cameraIndex + 1] : NULL, commandBufferIndex);
			}
			commandBufferIndex++;
		}	

		VulkanUtil::PresentSwapchainToDisplay(*m_pVulkanData);
		return true;
	}

	VertexBuffer * VulkanRenderer::CreateVertexBuffer(unsigned int vertexSize, unsigned int numVerts)
	{
		return new VulkanVertexBuffer(vertexSize, numVerts);
	}

	IndexBuffer * VulkanRenderer::CreateIndexBuffer(IndexType indexType, unsigned int numIndexes)
	{
		return new VulkanIndexBuffer(indexType, numIndexes);
	}

	GameObject * VulkanRenderer::CreateGameObject(ResourceManager * pResourceMgr, const std::string & name)
	{
		return new VulkanGameObject(pResourceMgr, name);
	}

	Shader * VulkanRenderer::CreateShader(ResourceManager * pResourceMgr, const std::string & name)
	{
		return new VulkanShader(pResourceMgr, name);
	}

	Texture * VulkanRenderer::CreateTexture(ResourceManager * pResourceMgr, const std::string & name, const std::string& filename)
	{
		return new VulkanTexture(filename, pResourceMgr, name);
	}

	Texture * VulkanRenderer::CreateRenderTexture(ResourceManager * pResourceMgr, int width, int height, bool isFrameBuffer, const std::string & name)
	{
		return new VulkanTexture(width, height, isFrameBuffer, pResourceMgr, name);
	}

	bool VulkanRenderer::bindAndDrawSubMesh(Camera* pCamera, SubMeshData * pSubMeshData, Mesh* pPrevMesh, Frustum* pFrustum, GraphicsPipelineData* pGraphicsPipelineData, 
		Material* pMaterial, bool isModifyCamera, int commandBufferIndex)
	{
		if (pSubMeshData == NULL || pFrustum == NULL || pGraphicsPipelineData == NULL || pMaterial == NULL)
		{
			return false;
		}

		GameObject* go = pSubMeshData->pMeshRenderer->GetGameObject();
		SubMesh* pSubMesh = pSubMeshData->pSubMesh;

		if (!pFrustum->CheckSphere(go->GetCenter(), go->GetRadius()))
		{
			return false;
		}

		PipelineLayout* pipeline = NULL;
		if (m_pVulkanData->m_mapPipeline.find(pSubMesh) == m_pVulkanData->m_mapPipeline.end())
		{
			m_pVulkanData->m_mapPipeline.insert(std::pair<SubMesh*, std::map<Material*, PipelineLayout*>>(pSubMesh, std::map<Material*, PipelineLayout*>()));
		}

		std::map<Material*, PipelineLayout*>& materialPipeline = m_pVulkanData->m_mapPipeline[pSubMesh];
		if (materialPipeline.find(pMaterial) == materialPipeline.end())
		{
			materialPipeline.insert(std::pair<Material*, PipelineLayout*>(pMaterial, new PipelineLayout()));
		}
		pipeline = materialPipeline[pMaterial];


		if (pGraphicsPipelineData->isInit == false)
		{
			VulkanUtil::LoadTexture(*m_pVulkanData, pMaterial, commandBufferIndex);
			VulkanUtil::CreatePipelineLayout(*m_pVulkanData, pMaterial, pGraphicsPipelineData);
			VulkanUtil::CreateShader(*m_pVulkanData, pMaterial, pGraphicsPipelineData, commandBufferIndex);
			VulkanUtil::CreatePipeline(*m_pVulkanData, pCamera, pMaterial, pSubMesh, pGraphicsPipelineData);
			VulkanUtil::BindGraphicsPipeline(*m_pVulkanData, pGraphicsPipelineData, commandBufferIndex);
			pGraphicsPipelineData->isInit = true;
		}

		VulkanUtil::CreateUniformBuffer(*m_pVulkanData, go, pMaterial, pCamera, isModifyCamera);

		if (pSubMeshData->isDirty)
		{
			VulkanUtil::CreateDescriptPool(*m_pVulkanData, pipeline, pMaterial);
			VulkanUtil::BindDataVertexBuffer(pSubMesh->pMesh);
			VulkanUtil::BindDataIndexBuffer(pSubMesh);
			VulkanUtil::UpdateDescriptSet(*m_pVulkanData, go, pipeline, pMaterial, pGraphicsPipelineData);
			pSubMeshData->isDirty = false;
		}

		VulkanUtil::BindPipeline(*m_pVulkanData, pipeline, pGraphicsPipelineData, commandBufferIndex);
		if (pPrevMesh != pSubMesh->pMesh)
		{
			VulkanUtil::BindSubMeshVertexBuffer(*m_pVulkanData, pSubMesh, commandBufferIndex);
			pPrevMesh = pSubMesh->pMesh;
		}

		VulkanUtil::BindSubMeshIndexBuffer(*m_pVulkanData, pSubMesh, commandBufferIndex);

		return true;
	}

	GraphicsPipelineData * VulkanRenderer::getGraphicsPipelineData(Material * pMaterial)
	{
		if (pMaterial == NULL)
		{
			return NULL;
		}

		GraphicsPipelineData* pGraphicsPipelineData = NULL;
		if (m_pVulkanData->m_mapGraphicsPipeline.find(pMaterial) == m_pVulkanData->m_mapGraphicsPipeline.end())
		{
			m_pVulkanData->m_mapGraphicsPipeline.insert(std::pair<Material*, GraphicsPipelineData*>(pMaterial, new GraphicsPipelineData()));
		}

		return m_pVulkanData->m_mapGraphicsPipeline[pMaterial];
	}
}