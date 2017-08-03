#ifndef _TEXTURE_MANAGER_H_
#define _TEXTURE_MANAGER_H_

#pragma once

#include "ResourceManager.h"
#include "Singleton.h"
#include "CommonUtil.h"

namespace Vulkan
{
	class TextureManager : public ResourceManager, public Singleton<TextureManager>
	{
	public:
		TextureManager();
		virtual ~TextureManager();

		virtual Resource* Create(const std::string& name);
		virtual Resource* Create(const std::string& name, const std::string& fileName);
		Resource* CreateRenderTexture(const std::string& name, int width, int height, bool isFrameBuffer);

		SINGLETON_DECLARE(TextureManager)
	protected:
		virtual Resource* createImpl(const std::string& name);
		virtual Resource* createImpl(const std::string& name, const std::string& filename);
	};
}

#endif
