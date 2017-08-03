#include "../include/ShaderManager.h"
#include "../include/Shader.h"
#include "../include/Renderer.h"
#include "../include/Root.h"
#include <fstream>

namespace Vulkan
{
	SINGLETON_IMPL(ShaderManager)
	ShaderManager::ShaderManager()
	{
	}

	ShaderManager::~ShaderManager()
	{
	}

	Resource * ShaderManager::createImpl(const std::string & name, const std::string & filename)
	{
		Shader* pShader = static_cast<Shader*>(createImpl(name));

		size_t shaderSize = 0;
		char* shaderCode = NULL;
#if defined(__ANDROID__)
		// Load shader from compressed asset
		AAsset* asset = AAssetManager_open(androidApp->activity->assetManager, filename.c_str(), AASSET_MODE_STREAMING);
		assert(asset);
		shaderSize = AAsset_getLength(asset);
		assert(shaderSize > 0);

		shaderCode = new char[shaderSize];
		AAsset_read(asset, shaderCode, shaderSize);
		AAsset_close(asset);
#else
		std::ifstream is(filename.c_str(), std::ios::in | std::ios::ate);

		if (is.is_open())
		{
			is.seekg(0, std::ios::end);
			shaderSize = is.tellg();
			is.seekg(0, std::ios::beg);
			// Copy file contents into a buffer
			shaderCode = new char[shaderSize + 1];
			memset(shaderCode, '\0', shaderSize + 1);
			is.read(shaderCode, shaderSize);
			is.close();
			assert(shaderSize > 0);
		}
#endif
		pShader->SetShaderText(shaderCode);
		delete[] shaderCode;

		return pShader;
	}

	Resource* ShaderManager::createImpl(const std::string& name)
	{
		return Root::GetSingleton().GetRenderer()->CreateShader(this, name);
	}
}