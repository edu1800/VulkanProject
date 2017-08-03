#ifndef _VULKAN_RENDERER_H_
#define _VULKAN_RENDERER_H_

#pragma once

#include "../Core/include/Renderer.h"
#include <map>

namespace Vulkan
{
	struct VulkanData;
	struct SubMeshData;
	class RenderData;
	class Material;
	class Frustum;
	struct GraphicsPipelineData;
	class VulkanRenderer : public Renderer
	{
	public:
		VulkanRenderer ();
		~VulkanRenderer ();

		VulkanData* GetVulkanData() const { return m_pVulkanData; }

		virtual void Initialize(const char * windowName, int width, int height);
		virtual bool Render();

		virtual VertexBuffer* CreateVertexBuffer(unsigned int vertexSize, unsigned int numVerts);
		virtual IndexBuffer* CreateIndexBuffer(IndexType indexType, unsigned int numIndexes);
		virtual GameObject* CreateGameObject(ResourceManager * pResourceMgr, const std::string & name);
		virtual Shader* CreateShader(ResourceManager* pResourceMgr, const std::string& name);
		virtual Texture* CreateTexture(ResourceManager* pResourceMgr, const std::string& name, const std::string& filename);
		virtual Texture* CreateRenderTexture(ResourceManager* pResourceMgr, int width, int height, bool isFrameBuffer, const std::string& name);

	protected:
		void initBeforeRender();
		bool bindAndDrawSubMesh(Camera* pCamera, SubMeshData* pSubMeshData, Mesh* pPrevMesh, Frustum* pFrustum, GraphicsPipelineData* pGraphicsPipelineData, 
			Material* pMaterial, bool isModifyCamera, int commandBufferIndex);
		GraphicsPipelineData* getGraphicsPipelineData(Material* pMaterial);

		VulkanData* m_pVulkanData;
		RenderData* m_pRenderData;
		bool m_bIsInitBeforeRender;
	};
}

#endif
