#include "../include/MaterialManager.h"
#include "../include/Material.h"
#include "../include/Shader.h"
#include "../include/ShaderManager.h"

namespace Vulkan
{
	SINGLETON_IMPL(MaterialManager)

	static char *defaultVertShaderText =
	"#version 400\n"
	"#extension GL_ARB_separate_shader_objects : enable\n"
	"#extension GL_ARB_shading_language_420pack : enable\n"
	"layout (std140, binding = 0) uniform bufferVals {\n"
	"    mat4 mvp;\n"
	"} myBufferVals;\n"
	"layout (location = 0) in vec3 pos;\n"
	"layout (location = 1) in vec3 normal;\n"
	"layout (location = 2) in vec2 tex;\n"
	"out gl_PerVertex { \n"
	"    vec4 gl_Position;\n"
	"};\n"
	"void main() {\n"
	"   gl_Position = myBufferVals.mvp * vec4(pos, 1);\n"
	"}\n";

	static char *defaultFragShaderText =
	"#version 400\n"
	"#extension GL_ARB_separate_shader_objects : enable\n"
	"#extension GL_ARB_shading_language_420pack : enable\n"
	"layout (location = 0) out vec4 outColor;\n"
	"void main() {\n"
	"   outColor = vec4(0.8, 0, 0, 1);\n"
	"}\n";

	MaterialManager::MaterialManager()
	{
		m_pDefaultMaterial = static_cast<Material*>(Create("Default"));

		Shader* vertexShader = static_cast<Shader*>(ShaderManager::GetSingleton().Create("DefaultVertex"));
		vertexShader->SetEntryName("main");
		vertexShader->SetShaderType(VERTEX);
		vertexShader->SetShaderText(defaultVertShaderText);

		Shader* fragmentShader = static_cast<Shader*>(ShaderManager::GetSingleton().Create("DefaultFrag"));
		fragmentShader->SetEntryName("main");
		fragmentShader->SetShaderType(FRAGMENT);
		fragmentShader->SetShaderText(defaultFragShaderText);
		
		m_pDefaultMaterial->AddShader(vertexShader);
		m_pDefaultMaterial->AddShader(fragmentShader);
	}

	MaterialManager::~MaterialManager()
	{
		m_pDefaultMaterial = NULL;
	}

	Resource * MaterialManager::createImpl(const std::string & name)
	{
		return new Material(this, name);
	}
}