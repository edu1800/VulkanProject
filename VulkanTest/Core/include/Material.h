#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#pragma once

#include <vector>
#include "Resource.h"
#include "Shader.h"
#include "MaterialParameterDefine.h"

namespace Vulkan 
{
	class Texture;
	typedef std::vector<Texture*> TextureList;
	class Material : public Resource
	{
	public:
		Material(ResourceManager* pResourceMgr, const std::string& name);
		virtual ~Material();

		void AddTexture(Texture* pTexture);
		const TextureList& GetTextureList() { return m_vecTextures; }
		Texture* GetTexture(int index);
		int GetTextureCount() const { return m_vecTextures.size(); }

		virtual void Load();
		virtual void UnLoad();

		void AddShader(Shader* pShader);

		void SetColorMask(int value) { m_iColorMask = value; }
		int GetColorMask() const { return m_iColorMask; }

		void SetIsEnableDepthBias(bool isEnable) { m_bEnableDepthBias = isEnable; }
		bool IsEnableDepthBias() const { return m_bEnableDepthBias; }

		void SetDepthBiasValue(float value) { m_fDepthBiasValue = value; }
		float GetDepthBiasValue() const { return m_fDepthBiasValue; }

		void SetDepthBiasClamp(float value) { m_fDepthBiasClamp = value; }
		float GetDepthBiasClamp() const { return m_fDepthBiasClamp; }

		void SetDepthBiasSlope(float value) { m_fDepthBiasSlope = value; }
		float GetDepthBiasSlope() const { return m_fDepthBiasSlope; }

		void SetCullMode(CullMode mode) { m_eCullMode = mode; }
		CullMode GetCullMode() const { return m_eCullMode; }

		void SetSceneBlending(SceneBlendFactor sourceBlendFactor, SceneBlendFactor destBlendFactor);
		void SetSceneBlending(SceneBlendFactor sourceBlendFactor, SceneBlendFactor destBlendFactor, 
							  SceneBlendFactor sourceAlphaBlendFactor, SceneBlendFactor destAlphaBlendFactor);

		void SetSceneBlendingOperation(SceneBlendOperation operation);
		void SetSeparateSceneBlendingOperation(SceneBlendOperation colourOperation, SceneBlendOperation alphaOperation);

		SceneBlendFactor GetSourceBlendFactor() const { return m_eSourceBlendFactor; }
		SceneBlendFactor GetDestBlendFactor() const { return m_eDestBlendFactor; }
		SceneBlendFactor GetSourceBlendFactorAlpha() const { return m_eSourceBlendFactorAlpha; }
		SceneBlendFactor GetDestBlendFactorAlpha() const { return m_eDestBlendFactorAlpha; }

		SceneBlendOperation GetBlendOperation() const { return m_eBlendOperation; }
		SceneBlendOperation GetAlphaBlendOperation() const { return m_eAlphaBlendOperation; }

		void EnableDepthTest(bool isEnable) { m_bIsDepthTest = isEnable; }
		bool IsEnableDepthTest() const { return m_bIsDepthTest; }
		void EnableDepthWrite(bool isEnable) { m_bIsDepthWrite = isEnable; }
		bool IsEnableDepthWrite() const { return m_bIsDepthWrite; }

		void SetDepthCompareFunction(DepthCompareFunction func) { m_eDepthCompareFunc = func; }
		DepthCompareFunction GetDepthCompareFunction() const { return m_eDepthCompareFunc; }

		bool IsOpaque();

		Shader* GetVertexShader() const { return m_pVertexShader; }
		Shader* GetFragmentShader() const { return m_pFragmentShader; }
	private:
		TextureList m_vecTextures;
		Shader* m_pVertexShader;
		Shader* m_pFragmentShader;

		bool m_bEnableDepthBias;
		float m_fDepthBiasValue;
		float m_fDepthBiasClamp;
		float m_fDepthBiasSlope;

		CullMode m_eCullMode;
		int m_iColorMask;

		SceneBlendFactor m_eSourceBlendFactor;
		SceneBlendFactor m_eDestBlendFactor;
		SceneBlendFactor m_eSourceBlendFactorAlpha;
		SceneBlendFactor m_eDestBlendFactorAlpha;

		SceneBlendOperation m_eBlendOperation;
		SceneBlendOperation m_eAlphaBlendOperation;

		bool m_bIsDepthTest;
		bool m_bIsDepthWrite;
		DepthCompareFunction m_eDepthCompareFunc;
	};
}
#endif
