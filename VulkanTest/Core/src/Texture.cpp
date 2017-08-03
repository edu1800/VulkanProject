#include "../include/Texture.h"
#include "../include/CommonUtil.h"
#include "../../ThirdParty/stb-master/stb_image.h"

namespace Vulkan
{
	Texture::Texture(const std::string& fileName, ResourceManager* pResourceMgr, const std::string& name)
		:Resource(pResourceMgr, name)
		, m_FileName(fileName)
		, m_iTextureWidth(0)
		, m_iTextureHeight(0)
		, m_iTextureDepth(1)
		, m_eTextureType(TEX_TYPE_2D)
		, m_iMipmap(1)
		, m_eTextureMinFilter(TF_LINEAR)
		, m_eTextureMagFilter(TF_LINEAR)
		, m_eAddressModeU(TAM_CLAMP)
		, m_eAddressModeV(TAM_CLAMP)
		, m_eAddressModeW(TAM_CLAMP)
		, m_bIsEnableAnisotropy(false)
		, m_iMaxAnisotropy(1)
		, m_bIsLoaded(false)
		, m_bIsRenderTarget(false)
		, m_bIsFrameBuffer(false)
	{
		m_eType = TEXTURE;

		Load();
		UnLoad();
/*
		std::string ext = CommonUtil::GetFileExtension(m_FileName);
		if (ext == "ppm")
		{
			readTexture_ppm(0, NULL);
		}*/
	}

	Texture::Texture(int width, int height, bool isFrameBuffer, ResourceManager * pResourceMgr, const std::string & name)
		: Resource(pResourceMgr, name)
		, m_iTextureWidth(width)
		, m_iTextureHeight(height)
		, m_iTextureDepth(1)
		, m_eTextureType(TEX_TYPE_2D)
		, m_iMipmap(1)
		, m_eTextureMinFilter(TF_LINEAR)
		, m_eTextureMagFilter(TF_LINEAR)
		, m_eAddressModeU(TAM_CLAMP)
		, m_eAddressModeV(TAM_CLAMP)
		, m_eAddressModeW(TAM_CLAMP)
		, m_bIsEnableAnisotropy(false)
		, m_iMaxAnisotropy(1)
		, m_bIsLoaded(false)
		, m_bIsRenderTarget(true)
		, m_bIsFrameBuffer(isFrameBuffer)
	{
		m_eType = TEXTURE;
	}

	Texture::~Texture()
	{
		UnLoad();
	}

	void Texture::Load()
	{
		if (!m_bIsRenderTarget)
		{
			int texChannel = 0;
			m_pPixels = stbi_load(m_FileName.c_str(), &m_iTextureWidth, &m_iTextureHeight, &texChannel, STBI_rgb_alpha);
		}
	}

	void Texture::UnLoad()
	{
		if (m_pPixels != NULL)
		{
			stbi_image_free(m_pPixels);
			m_pPixels = NULL;
		}
	}

	void Texture::copyTexture(int rowPitch, unsigned char * dataPtr)
	{
		if (dataPtr == NULL) {
			// If no destination pointer, caller only wanted dimensions
			return;
		}

		for (size_t i = 0; i < m_iTextureHeight; i++)
		{
			memcpy(&dataPtr[i * rowPitch], &m_pPixels[i * m_iTextureWidth * 4], m_iTextureWidth * 4);
		}
	}
}