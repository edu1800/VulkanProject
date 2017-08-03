#include "../include/Root.h"
#include "../include/GameObjectManager.h"
#include "../include/MaterialManager.h"
#include "../include/TextureManager.h"
#include "../include/MeshManager.h"
#include "../include/ShaderManager.h"
#include "../include/HardwareBufferManager.h"
#include "../include/CompositorManager.h"
#include "../VulkanRenderSystem/VulkanRenderer.h"
#include <iostream>

namespace Vulkan
{
	SINGLETON_IMPL(Root)
	Root::Root()
		:m_pGameObjectMgr(NULL)
		,m_pMaterialMgr(NULL)
		,m_pTextureMgr(NULL)
		,m_pMeshMgr(NULL)
		,m_pShaderMgr(NULL)
		,m_pRenderer(NULL)
		,m_pHardwareBufferMgr(NULL)
		,m_pCompositorMgr(NULL)
	{

	}

	Root::~Root()
	{
		SAFE_DELETE(m_pRenderer);
		m_pGameObjectMgr->RemoveAll();
		SAFE_DELETE(m_pGameObjectMgr);
		SAFE_DELETE(m_pMeshMgr);
		SAFE_DELETE(m_pHardwareBufferMgr);
		SAFE_DELETE(m_pShaderMgr);
		SAFE_DELETE(m_pTextureMgr);
		SAFE_DELETE(m_pMaterialMgr);
	}

	void Root::Initialize()
	{
		m_pGameObjectMgr = new GameObjectManager();
		m_pRenderer = new VulkanRenderer();
		m_pShaderMgr = new ShaderManager();
		m_pTextureMgr = new TextureManager();
		m_pMaterialMgr = new MaterialManager();
		m_pMeshMgr = new MeshManager();		
		m_pHardwareBufferMgr = new HardwareBufferManager();
		m_pCompositorMgr = new CompositorManager();
	}
}