#ifndef _ROOT_H_
#define _ROOT_H_

#pragma once

#include "Singleton.h"
#include "CommonUtil.h"

namespace Vulkan
{
	class Renderer;
	class GameObjectManager;
	class MaterialManager;
	class TextureManager;
	class MeshManager;
	class ShaderManager;
	class HardwareBufferManager;
	class CompositorManager;
	class Root : public Singleton<Root>
	{
	public:
		Root();
		~Root();

		void Initialize();
		Renderer* GetRenderer() const { return m_pRenderer; }

		SINGLETON_DECLARE(Root)

	private:
		GameObjectManager* m_pGameObjectMgr;
		MaterialManager* m_pMaterialMgr;
		TextureManager* m_pTextureMgr;
		MeshManager* m_pMeshMgr;
		ShaderManager* m_pShaderMgr;
		Renderer* m_pRenderer;
		HardwareBufferManager* m_pHardwareBufferMgr;
		CompositorManager* m_pCompositorMgr;
	};
}

#endif
