#ifndef _SHADER_H_
#define _SHADER_H_

#pragma once

#include "Resource.h"
#include <string>
#include <map>
#include "ShaderParameter.h"

namespace Vulkan
{
	enum ShaderType
	{
		VERTEX,
		FRAGMENT
	};

	struct ShaderModule;
	class Shader : public Resource
	{
	public:
		Shader(ResourceManager* pResourceMgr, const std::string& name);
		virtual ~Shader();

		virtual void Load();
		virtual void UnLoad();

		void SetShaderType(ShaderType type) { m_eShaderType = type; onSetShaderType(); }
		ShaderType GetShaderType() const { return m_eShaderType; }

		void SetShaderText(const std::string& text) { m_Text = text; }
		const std::string& GetShaderText() const { return m_Text; }

		void SetEntryName(const std::string& entryName) { m_EntryName = entryName; }
		const std::string& GetEntryName() const { return m_EntryName; }

		ShaderParameters* GetShaderParameters()  { return &m_Parameters; }
		ShaderParameters* GetUniformShaderParameters() { return &m_UniformParameter; }
		ShaderParametersInt* GetConstantParameters() { return &m_ConstantParameter; }

	protected:
		void onSetShaderType();

		ShaderType m_eShaderType;
		std::string m_Text;
		std::string m_EntryName;
		ShaderParameters m_Parameters;
		ShaderParameters m_UniformParameter;
		ShaderParametersInt m_ConstantParameter;
	};
}
#endif
