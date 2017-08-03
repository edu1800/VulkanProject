#ifndef _COMPOSITOR_H_
#define _COMPOSITOR_H_

#pragma once

#include "Resource.h"
#include "CommonUtil.h"
#include <vector>

namespace Vulkan
{
	class Material;
	class Texture;
	enum CompositorTextureMode
	{
		CTM_NONE,
		//使用frameBuffer輸出
		CTM_FRAMEBUFFER,
		//使用貼圖輸出
		CTM_TEXTURE
	};

	struct CompositorPass
	{
		Material* pMaterial;
		CompositorTextureMode eMode;
		Texture* pTexture;
		int iWidth;
		int iHeight;

		CompositorPass(Material* mat, CompositorTextureMode mode, int width, int height, Texture* texture)
			:pMaterial(mat)
			,eMode(mode)
			,iWidth(width)
			,iHeight(height)
			,pTexture(texture)
		{

		}
	};

	typedef std::vector<CompositorPass> CompositorPassList;

	class Camera;
	class Compositor : public Resource
	{
	public:
		Compositor(ResourceManager* pResourceMgr, const std::string& name);
		virtual ~Compositor();

		virtual void Load();
		virtual void UnLoad();

		void AddPass(Material* pMaterial, CompositorTextureMode mode, int width, int height, Texture* texture);
		const CompositorPassList& GetPassList() const { return m_listCompositor; }
		size_t GetPassCount() { return m_listCompositor.size(); }
		CompositorPass* GetCompositorPass(unsigned int index);

		GET_SET_DECLARE(Camera, Camera*, m_pCamera)
	protected:
		CompositorPassList m_listCompositor;
		Camera* m_pCamera;
	};
}

#endif
