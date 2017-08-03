#include "BlurEffect.h"
#include "../Core/include/CompositorManager.h"
#include "../Core/include/Compositor.h"
#include "../Core/include/Material.h"
#include "../Core/include/MaterialManager.h"
#include "../Core/include/Shader.h"
#include "../Core/include/ShaderManager.h"
#include "../Core/include/Texture.h"
#include "../Core/include/TextureManager.h"

namespace Vulkan
{
	BlurEffect::BlurEffect(int windowWidth, int windowHeight)
	{
		Shader* pVertexShader = static_cast<Shader*>(ShaderManager::GetSingleton().Create("Blur", "../Resource/gaussblur.vert"));
		pVertexShader->SetEntryName("main");
		pVertexShader->SetShaderType(VERTEX);
		pVertexShader->GetUniformShaderParameters()->RemoveAll();

		Shader* pHFragShader = static_cast<Shader*>(ShaderManager::GetSingleton().Create("HBlur", "../Resource/gaussblur.frag"));
		pHFragShader->SetEntryName("main");
		pHFragShader->SetShaderType(FRAGMENT);
		pHFragShader->GetConstantParameters()->AddParamter(0, GCT_INT1, 1);
		pHFragShader->GetConstantParameters()->SetParameter(0, 1);
		pHFragShader->GetUniformShaderParameters()->RemoveAll();
		pHFragShader->GetUniformShaderParameters()->AddParamter("blurScale", GCT_FLOAT1, 1);
		pHFragShader->GetUniformShaderParameters()->SetParameter("blurScale", 0.5f);
		pHFragShader->GetUniformShaderParameters()->AddParamter("blurStrength", GCT_FLOAT1, 1);
		pHFragShader->GetUniformShaderParameters()->SetParameter("blurStrength", 0.5f);

		Shader* pVFragShader = static_cast<Shader*>(ShaderManager::GetSingleton().Create("VBlur", "../Resource/gaussblur.frag"));
		pVFragShader->SetEntryName("main");
		pVFragShader->SetShaderType(FRAGMENT);
		pVFragShader->GetConstantParameters()->AddParamter(0, GCT_INT1, 1);
		pVFragShader->GetConstantParameters()->SetParameter(0, 0);
		pVFragShader->GetUniformShaderParameters()->RemoveAll();
		pVFragShader->GetUniformShaderParameters()->AddParamter("blurScale", GCT_FLOAT1, 1);
		pVFragShader->GetUniformShaderParameters()->SetParameter("blurScale", 0.5f);
		pVFragShader->GetUniformShaderParameters()->AddParamter("blurStrength", GCT_FLOAT1, 1);
		pVFragShader->GetUniformShaderParameters()->SetParameter("blurStrength", 0.5f);

		Shader* pDownSampleFragShader = static_cast<Shader*>(ShaderManager::GetSingleton().Create("DownSample", "../Resource/TextureDownSampleColor.frag"));
		pDownSampleFragShader->SetEntryName("main");
		pDownSampleFragShader->SetShaderType(FRAGMENT);
		pDownSampleFragShader->GetUniformShaderParameters()->RemoveAll();

		Shader* pUpSampleFragShader = static_cast<Shader*>(ShaderManager::GetSingleton().Create("UpSample", "../Resource/TextureColor.frag"));
		pUpSampleFragShader->SetEntryName("main");
		pUpSampleFragShader->SetShaderType(FRAGMENT);
		pUpSampleFragShader->GetUniformShaderParameters()->RemoveAll();

		int textureWidth = windowWidth / 2;
		int textureHeight = windowHeight / 2;
		Texture* pBlurTexture = static_cast<Texture*>(TextureManager::GetSingleton().CreateRenderTexture("BlurTexture", windowWidth, windowHeight, true));
		Texture* pDownSampleTexture = static_cast<Texture*>(TextureManager::GetSingleton().CreateRenderTexture("DownSampleTexture", textureWidth, textureHeight, false));
	
		Material* pHMaterial = static_cast<Material*>(MaterialManager::GetSingleton().Create("HBlur"));
		pHMaterial->SetSceneBlendingOperation(SBO_ADD);
		//pHMaterial->SetSceneBlending(SBF_ONE, SBF_ONE, SBF_SRC_ALPHA, SBF_DST_ALPHA);
		pHMaterial->AddShader(pVertexShader);
		pHMaterial->AddShader(pHFragShader);
		pHMaterial->SetCullMode(CM_FLAG_FRONT);
		pHMaterial->AddTexture(pDownSampleTexture);

		Material* pVMaterial = static_cast<Material*>(MaterialManager::GetSingleton().Create("VBlur"));
		pVMaterial->SetSceneBlendingOperation(SBO_ADD);
		//pVMaterial->SetSceneBlending(SBF_ONE, SBF_ONE, SBF_SRC_ALPHA, SBF_DST_ALPHA);
		pVMaterial->AddShader(pVertexShader);
		pVMaterial->AddShader(pVFragShader);
		pVMaterial->SetCullMode(CM_FLAG_FRONT);
		pVMaterial->AddTexture(pDownSampleTexture);

		Material* pDownSampleMaterial = static_cast<Material*>(MaterialManager::GetSingleton().Create("DownSample"));
		pDownSampleMaterial->AddShader(pVertexShader);
		pDownSampleMaterial->AddShader(pDownSampleFragShader);
		pDownSampleMaterial->SetCullMode(CM_FLAG_FRONT);
		pDownSampleMaterial->AddTexture(pBlurTexture);

		Material* pUpSampleMaterial = static_cast<Material*>(MaterialManager::GetSingleton().Create("UpSample"));
		pUpSampleMaterial->SetSceneBlending(SBF_ONE, SBF_ONE, SBF_SRC_ALPHA, SBF_DST_ALPHA);
		pUpSampleMaterial->AddShader(pVertexShader);
		pUpSampleMaterial->AddShader(pUpSampleFragShader);
		pUpSampleMaterial->SetCullMode(CM_FLAG_FRONT);
		pUpSampleMaterial->AddTexture(pDownSampleTexture);

		Compositor* pCompositor = static_cast<Compositor*>(CompositorManager::GetSingleton().Create("Blur"));
		pCompositor->AddPass(pDownSampleMaterial, CTM_TEXTURE, textureWidth, textureHeight, pDownSampleTexture);
		pCompositor->AddPass(pHMaterial, CTM_TEXTURE, textureWidth, textureHeight, pDownSampleTexture);
		pCompositor->AddPass(pVMaterial, CTM_TEXTURE, textureWidth, textureHeight, pDownSampleTexture);
		pCompositor->AddPass(pUpSampleMaterial, CTM_FRAMEBUFFER, windowWidth, windowHeight, NULL);
	}

	BlurEffect::~BlurEffect()
	{
	}

	void BlurEffect::SetCamera(Camera * pCamera)
	{
		Compositor* pCompositor = static_cast<Compositor*>(CompositorManager::GetSingleton().GetResource("Blur"));
		pCompositor->SetCamera(pCamera);
	}
}