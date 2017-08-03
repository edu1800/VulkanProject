#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#pragma once

#include "Resource.h"
#include <string>

namespace Vulkan
{
	enum TextureType
	{
		TEX_TYPE_1D,
		TEX_TYPE_2D,
		TEX_TYPE_3D,
		TEX_TYPE_CUBE_MAP,
		TEX_TYPE_2D_ARRAY
	};

	enum TextureFilter
	{
		TF_NEAREST,
		TF_LINEAR,
		TF_CUBIC
	};

	enum TextureAddressMode
	{
		TAM_WRAP,
		TAM_MIRROR,
		TAM_CLAMP,
		TAM_BORDER
	};

	class Texture : public Resource
	{
	public:
		Texture(const std::string& fileName, ResourceManager* pResourceMgr, const std::string& name);
		Texture(int width, int height, bool isFrameBuffer, ResourceManager* pResourceMgr, const std::string& name);
		virtual ~Texture();

		virtual void Load();
		virtual void UnLoad();

		int GetWidth() const { return m_iTextureWidth; }
		int GetHeight() const { return m_iTextureHeight; }
		int GetDepth() const { return m_iTextureDepth; }

		void SetTextureType(TextureType type) { m_eTextureType = type; }
		TextureType GetTextureType() const { return m_eTextureType; }

		void SetMipmap(int mipmap) { m_iMipmap = mipmap; }
		int GetMipmap() const { return m_iMipmap; }

		void SetMinFilter(TextureFilter filter) { m_eTextureMinFilter = filter; }
		TextureFilter GetMinFilter() const { return m_eTextureMinFilter; }

		void SetMagFilter(TextureFilter filter) { m_eTextureMagFilter = filter; }
		TextureFilter GetMagFilter() const { return m_eTextureMagFilter; }

		void SetAddressModeU(TextureAddressMode mode) { m_eAddressModeU = mode; }
		TextureAddressMode GetAddressModeU() const { return m_eAddressModeU; }

		void SetAddressModeV(TextureAddressMode mode) { m_eAddressModeV = mode; }
		TextureAddressMode GetAddressModeV() const { return m_eAddressModeV; }

		void SetAddressModeW(TextureAddressMode mode) { m_eAddressModeW = mode; }
		TextureAddressMode GetAddressModeW() const { return m_eAddressModeW; }

		void SetEnableAnisotropy(bool enable) { m_bIsEnableAnisotropy = enable; }
		bool IsEnableAnisotropy() const { return m_bIsEnableAnisotropy; }

		void SetMaxAnisotropy(int anisotropy) { m_iMaxAnisotropy = anisotropy; }
		int GetmaxAnisotropy() const { return m_iMaxAnisotropy; }

		const std::string& GetFileName() const { return m_FileName; }

		void* GetPixels() const { return m_pPixels; }

		bool IsRenderTarget() const { return m_bIsRenderTarget; }
		bool IsFrameBuffer() const { return m_bIsFrameBuffer; }

	protected:
		void copyTexture(int rowPitch, unsigned char* dataPtr);

		std::string m_FileName;
		int m_iTextureWidth;
		int m_iTextureHeight;
		int m_iTextureDepth;
		TextureType m_eTextureType;
		int m_iMipmap;
		bool m_bIsRenderTarget;
		bool m_bIsFrameBuffer;

		TextureFilter m_eTextureMinFilter;
		TextureFilter m_eTextureMagFilter;

		TextureAddressMode m_eAddressModeU;
		TextureAddressMode m_eAddressModeV;
		TextureAddressMode m_eAddressModeW;

		bool m_bIsEnableAnisotropy;
		int m_iMaxAnisotropy;

		unsigned char* m_pPixels;

		bool m_bIsLoaded;
	};

}
#endif
