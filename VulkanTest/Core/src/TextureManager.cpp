#include "../include/TextureManager.h"
#include "../include/Texture.h"
#include "../include/Renderer.h"
#include "../include/Root.h"

namespace Vulkan
{
	SINGLETON_IMPL(TextureManager)
	TextureManager::TextureManager()
	{
	}

	TextureManager::~TextureManager()
	{
	}

	Resource * TextureManager::Create(const std::string & name)
	{
		return Create(name, name + ".ppm");
	}

	Resource * TextureManager::Create(const std::string & name, const std::string & fileName)
	{
		return ResourceManager::Create(name, fileName);
	}

	Resource * TextureManager::CreateRenderTexture(const std::string & name, int width, int height, bool isFrameBuffer)
	{
		if (IsContain(name))
		{
			return NULL;
		}

		Resource* pResource = Root::GetSingleton().GetRenderer()->CreateRenderTexture(this, width, height, isFrameBuffer, name);
		if (pResource != NULL)
		{
			add(pResource);
		}
		return pResource;
	}

	Resource * TextureManager::createImpl(const std::string & name, const std::string & filename)
	{
		return Root::GetSingleton().GetRenderer()->CreateTexture(this, name, filename);
	}

	Resource * TextureManager::createImpl(const std::string & name)
	{
		return NULL;
	}

}
