#include "../include/Shader.h"

namespace Vulkan
{
	Shader::Shader(ResourceManager * pResourceMgr, const std::string & name)
		:Resource(pResourceMgr, name)
		,m_eShaderType(VERTEX)
		,m_Text("")
	{
		m_eType = SHADER;
		m_UniformParameter.SetIsPadding(false);
		m_ConstantParameter.SetIsPadding(false);
		onSetShaderType();
	}

	Shader::~Shader()
	{

	}

	void Shader::Load()
	{
	}

	void Shader::UnLoad()
	{
	}

	void Shader::onSetShaderType()
	{
		m_Parameters.RemoveParamter("LightPosition");
		m_Parameters.RemoveParamter("LightDirection");
		m_Parameters.RemoveParamter("LightColor");
		m_Parameters.RemoveParamter("LightIntensity");

		m_UniformParameter.RemoveAll();

		if (m_eShaderType == VERTEX)
		{
			m_UniformParameter.AddParamter("localToWorld", GCT_MATRIX_4X4, 1);
			m_UniformParameter.AddParamter("viewMatrix", GCT_MATRIX_4X4, 1);
			m_UniformParameter.AddParamter("projMatrix", GCT_MATRIX_4X4, 1);
			m_UniformParameter.AddParamter("camWorldPos", GCT_FLOAT3, 1);

			m_Parameters.AddParamter("LightPosition", GCT_FLOAT3, 1);
			m_Parameters.AddParamter("LightDirection", GCT_FLOAT3, 1);
			m_Parameters.AddParamter("LightColor", GCT_FLOAT3, 1);
			m_Parameters.AddParamter("LightIntensity", GCT_FLOAT1, 1);
		}
		else
		{

		}
	}
}