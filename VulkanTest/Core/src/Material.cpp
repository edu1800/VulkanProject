#include "../include/Material.h"
#include "../include/CommonUtil.h"

namespace Vulkan
{
	Material::Material(ResourceManager* pResourceMgr, const std::string& name)
		:Resource(pResourceMgr, name)
		,m_pVertexShader(NULL)
		,m_pFragmentShader(NULL)
		,m_bEnableDepthBias(false)
		,m_fDepthBiasValue(0.0f)
		,m_fDepthBiasClamp(0.0f)
		,m_fDepthBiasSlope(0.0f)
		,m_iColorMask(CM_R_BIT | CM_G_BIT | CM_B_BIT | CM_A_BIT)
		,m_eSourceBlendFactor(SBF_ONE)
		,m_eDestBlendFactor(SBF_ZERO)
		,m_eSourceBlendFactorAlpha(SBF_ONE)
		,m_eDestBlendFactorAlpha(SBF_ZERO)
		,m_eBlendOperation(SBO_ADD)
		,m_eAlphaBlendOperation(SBO_ADD)
		,m_bIsDepthTest(true)
		,m_bIsDepthWrite(true)
		,m_eDepthCompareFunc(DCF_LESS_EQUAL)
		, m_eCullMode(CM_FLAG_BACK)
	{
		m_eType = MATERIAL;
	}

	Material::~Material()
	{
		
	}

	void Material::AddTexture(Texture * pTexture)
	{
		m_vecTextures.push_back(pTexture);
	}

	Texture * Material::GetTexture(int index)
	{
		if (index < 0 || index >= GetTextureCount())
		{
			return NULL;
		}

		return m_vecTextures[index];
	}

	void Material::Load()
	{
	}

	void Material::UnLoad()
	{
	}

	void Material::AddShader(Shader * pShader)
	{
		if (pShader == NULL)
		{
			return;
		}

		switch (pShader->GetShaderType())
		{
		case VERTEX:
			m_pVertexShader = pShader;
			break;
		case FRAGMENT:
			m_pFragmentShader = pShader;
			break;
		default:
			break;
		}
	}

	void Material::SetSceneBlending(SceneBlendFactor sourceBlendFactor, SceneBlendFactor destBlendFactor)
	{
		SetSceneBlending(sourceBlendFactor, destBlendFactor, SBF_ONE, SBF_ZERO);
	}

	void Material::SetSceneBlending(SceneBlendFactor sourceBlendFactor, SceneBlendFactor destBlendFactor, SceneBlendFactor sourceAlphaBlendFactor, SceneBlendFactor destAlphaBlendFactor)
	{
		m_eSourceBlendFactor = sourceBlendFactor;
		m_eDestBlendFactor = destBlendFactor;
		m_eSourceBlendFactorAlpha = sourceAlphaBlendFactor;
		m_eDestBlendFactorAlpha = destAlphaBlendFactor;
	}

	void Material::SetSceneBlendingOperation(SceneBlendOperation operation)
	{
		SetSeparateSceneBlendingOperation(operation, SBO_ADD);
	}

	void Material::SetSeparateSceneBlendingOperation(SceneBlendOperation colourOperation, SceneBlendOperation alphaOperation)
	{
		m_eBlendOperation = colourOperation;
		m_eAlphaBlendOperation = alphaOperation;
	}
	bool Material::IsOpaque()
	{
		return m_eSourceBlendFactor == SBF_ONE && m_eDestBlendFactor == SBF_ZERO && m_eSourceBlendFactorAlpha == SBF_ONE && m_eDestBlendFactorAlpha == SBF_ZERO;
	}
}