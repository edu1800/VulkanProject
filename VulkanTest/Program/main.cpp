#include <iostream>
#include "../Core/include/Root.h"
#include "../Core/include/Renderer.h"
#include "../Core/include/Transform.h"
#include "../Core/include/Texture.h"
#include "../Core/include/TextureManager.h"
#include "../Core/include/Material.h"
#include "../Core/include/MaterialManager.h"
#include "../Core/include/Mesh.h"
#include "../Core/include/MeshManager.h"
#include "../Core/include/GameObject.h"
#include "../Core/include/GameObjectManager.h"
#include "../Core/include/Shader.h"
#include "../Core/include/ShaderManager.h"
#include "../Core/include/VertexBuffer.h"
#include "../Core/include/VertexIndexData.h"
#include "../Core/include/MeshRenderer.h"
#include "../Core/include/Camera.h"
#include "../Core/include/Light.h"
#include "MeshLoader.h"
#include "CameraOperation.h"
#include "BlurEffect.h"
#include "ShadowMap.h"
#include "ShadowCamera.h"
#include <time.h>

using namespace Vulkan;

#define XYZ1(_x_, _y_, _z_) (_x_), (_y_), (_z_), 1.f
#define UV(_u_, _v_) (_u_), (_v_)

struct Vertex {
	float posX, posY, posZ, posW; // Position data
	float r, g, b, a;             // Color
};

struct VertexUV {
	float posX, posY, posZ, posW; // Position data
	float u, v;                   // texture u,v
};

static const Vertex g_vb_solid_face_colors_Data[] = {
	//red face
	{ XYZ1(-1,-1, 1), XYZ1(1.f, 0.f, 0.f) },
	{ XYZ1(-1, 1, 1), XYZ1(1.f, 0.f, 0.f) },
	{ XYZ1(1,-1, 1), XYZ1(1.f, 0.f, 0.f) },
	{ XYZ1(1,-1, 1), XYZ1(1.f, 0.f, 0.f) },
	{ XYZ1(-1, 1, 1), XYZ1(1.f, 0.f, 0.f) },
	{ XYZ1(1, 1, 1), XYZ1(1.f, 0.f, 0.f) },
	//green face
	{ XYZ1(-1,-1,-1), XYZ1(0.f, 1.f, 0.f) },
	{ XYZ1(1,-1,-1), XYZ1(0.f, 1.f, 0.f) },
	{ XYZ1(-1, 1,-1), XYZ1(0.f, 1.f, 0.f) },
	{ XYZ1(-1, 1,-1), XYZ1(0.f, 1.f, 0.f) },
	{ XYZ1(1,-1,-1), XYZ1(0.f, 1.f, 0.f) },
	{ XYZ1(1, 1,-1), XYZ1(0.f, 1.f, 0.f) },
	//blue face
	{ XYZ1(-1, 1, 1), XYZ1(0.f, 0.f, 1.f) },
	{ XYZ1(-1,-1, 1), XYZ1(0.f, 0.f, 1.f) },
	{ XYZ1(-1, 1,-1), XYZ1(0.f, 0.f, 1.f) },
	{ XYZ1(-1, 1,-1), XYZ1(0.f, 0.f, 1.f) },
	{ XYZ1(-1,-1, 1), XYZ1(0.f, 0.f, 1.f) },
	{ XYZ1(-1,-1,-1), XYZ1(0.f, 0.f, 1.f) },
	//yellow face
	{ XYZ1(1, 1, 1), XYZ1(1.f, 1.f, 0.f) },
	{ XYZ1(1, 1,-1), XYZ1(1.f, 1.f, 0.f) },
	{ XYZ1(1,-1, 1), XYZ1(1.f, 1.f, 0.f) },
	{ XYZ1(1,-1, 1), XYZ1(1.f, 1.f, 0.f) },
	{ XYZ1(1, 1,-1), XYZ1(1.f, 1.f, 0.f) },
	{ XYZ1(1,-1,-1), XYZ1(1.f, 1.f, 0.f) },
	//magenta face
	{ XYZ1(1, 1, 1), XYZ1(1.f, 0.f, 1.f) },
	{ XYZ1(-1, 1, 1), XYZ1(1.f, 0.f, 1.f) },
	{ XYZ1(1, 1,-1), XYZ1(1.f, 0.f, 1.f) },
	{ XYZ1(1, 1,-1), XYZ1(1.f, 0.f, 1.f) },
	{ XYZ1(-1, 1, 1), XYZ1(1.f, 0.f, 1.f) },
	{ XYZ1(-1, 1,-1), XYZ1(1.f, 0.f, 1.f) },
	//cyan face
	{ XYZ1(1,-1, 1), XYZ1(0.f, 1.f, 1.f) },
	{ XYZ1(1,-1,-1), XYZ1(0.f, 1.f, 1.f) },
	{ XYZ1(-1,-1, 1), XYZ1(0.f, 1.f, 1.f) },
	{ XYZ1(-1,-1, 1), XYZ1(0.f, 1.f, 1.f) },
	{ XYZ1(1,-1,-1), XYZ1(0.f, 1.f, 1.f) },
	{ XYZ1(-1,-1,-1), XYZ1(0.f, 1.f, 1.f) },
};

static const VertexUV g_vb_texture_Data[] = {
	//left face
	{ XYZ1(-1,-1,-1), UV(1.f, 0.f) },  // lft-top-front
	{ XYZ1(-1, 1, 1), UV(0.f, 1.f) },  // lft-btm-back
	{ XYZ1(-1,-1, 1), UV(0.f, 0.f) },  // lft-top-back
	{ XYZ1(-1, 1, 1), UV(0.f, 1.f) },  // lft-btm-back
	{ XYZ1(-1,-1,-1), UV(1.f, 0.f) },  // lft-top-front
	{ XYZ1(-1, 1,-1), UV(1.f, 1.f) },  // lft-btm-front
									   //front face
	{ XYZ1(-1,-1,-1), UV(0.f, 0.f) },  // lft-top-front
	{ XYZ1(1,-1,-1), UV(1.f, 0.f) },  // rgt-top-front
	{ XYZ1(1, 1,-1), UV(1.f, 1.f) },  // rgt-btm-front
	{ XYZ1(-1,-1,-1), UV(0.f, 0.f) },  // lft-top-front
	{ XYZ1(1, 1,-1), UV(1.f, 1.f) },  // rgt-btm-front
	{ XYZ1(-1, 1,-1), UV(0.f, 1.f) },  // lft-btm-front
									   //top face
	{ XYZ1(-1,-1,-1), UV(0.f, 1.f) },  // lft-top-front
	{ XYZ1(1,-1, 1), UV(1.f, 0.f) },  // rgt-top-back
	{ XYZ1(1,-1,-1), UV(1.f, 1.f) },  // rgt-top-front
	{ XYZ1(-1,-1,-1), UV(0.f, 1.f) },  // lft-top-front
	{ XYZ1(-1,-1, 1), UV(0.f, 0.f) },  // lft-top-back
	{ XYZ1(1,-1, 1), UV(1.f, 0.f) },  // rgt-top-back
									  //bottom face
	{ XYZ1(-1, 1,-1), UV(0.f, 0.f) },  // lft-btm-front
	{ XYZ1(1, 1, 1), UV(1.f, 1.f) },  // rgt-btm-back
	{ XYZ1(-1, 1, 1), UV(0.f, 1.f) },  // lft-btm-back
	{ XYZ1(-1, 1,-1), UV(0.f, 0.f) },  // lft-btm-front
	{ XYZ1(1, 1,-1), UV(1.f, 0.f) },  // rgt-btm-front
	{ XYZ1(1, 1, 1), UV(1.f, 1.f) },  // rgt-btm-back
									  //right face
	{ XYZ1(1, 1,-1), UV(0.f, 1.f) },  // rgt-btm-front
	{ XYZ1(1,-1, 1), UV(1.f, 0.f) },  // rgt-top-back
	{ XYZ1(1, 1, 1), UV(1.f, 1.f) },  // rgt-btm-back
	{ XYZ1(1,-1, 1), UV(1.f, 0.f) },  // rgt-top-back
	{ XYZ1(1, 1,-1), UV(0.f, 1.f) },  // rgt-btm-front
	{ XYZ1(1,-1,-1), UV(0.f, 0.f) },  // rgt-top-front
									  //back face
	{ XYZ1(-1, 1, 1), UV(1.f, 1.f) },  // lft-btm-back
	{ XYZ1(1, 1, 1), UV(0.f, 1.f) },  // rgt-btm-back
	{ XYZ1(-1,-1, 1), UV(1.f, 0.f) },  // lft-top-back
	{ XYZ1(-1,-1, 1), UV(1.f, 0.f) },  // lft-top-back
	{ XYZ1(1, 1, 1), UV(0.f, 1.f) },  // rgt-btm-back
	{ XYZ1(1,-1, 1), UV(0.f, 0.f) },  // rgt-top-back
};

static const VertexUV g_vb_texture_index_data[] = {
	{ XYZ1(-1.5f,-1.5f,0.0f), UV(0.f, 0.f) },
	{ XYZ1(1.5f,-1.5f,0.0f), UV(1.f, 0.f) },
	{ XYZ1(1.5f,1.5f,0.0f), UV(1.f, 1.f) },
	{ XYZ1(-1.5f,1.5f,0.0f), UV(0.f, 1.f) }
};

static const unsigned short g_ib_data[] = {
	0, 1, 2, 2, 3, 0
};


int main()
{
	Root* root = new Root();
	root->Initialize();

	Texture* texture = static_cast<Texture*>(TextureManager::GetSingleton().Create("lunarg", "Hydrangeas.jpg"));
	Material* mat = static_cast<Material*>(MaterialManager::GetSingleton().Create("Mat1"));

	/*static const char *vertShaderText =
		"#version 400\n"
		"#extension GL_ARB_separate_shader_objects : enable\n"
		"#extension GL_ARB_shading_language_420pack : enable\n"
		"layout (std140, binding = 0) uniform bufferVals {\n"
		"    mat4 mvp;\n"
		"} myBufferVals;\n"
		"layout (location = 0) in vec4 pos;\n"
		"layout (location = 1) in vec4 inColor;\n"
		"layout (location = 0) out vec4 outColor;\n"
		"out gl_PerVertex { \n"
		"    vec4 gl_Position;\n"
		"};\n"
		"void main() {\n"
		"   outColor = inColor;\n"
		"   gl_Position = myBufferVals.mvp * pos;\n"
		"}\n";

	static const char *fragShaderText =
		"#version 400\n"
		"#extension GL_ARB_separate_shader_objects : enable\n"
		"#extension GL_ARB_shading_language_420pack : enable\n"
		"layout (location = 0) in vec4 color;\n"
		"layout (location = 0) out vec4 outColor;\n"
		"void main() {\n"
		"   outColor = color;\n"
		"}\n";*/

	/*const char *vertShaderText =
		"#version 400\n"
		"#extension GL_ARB_separate_shader_objects : enable\n"
		"#extension GL_ARB_shading_language_420pack : enable\n"
		"layout (std140, binding = 0) uniform buf {\n"
		"        mat4 mvp;\n"
		"} ubuf;\n"
		"layout (location = 0) in vec4 pos;\n"
		"layout (location = 1) in vec2 inTexCoords;\n"
		"layout (location = 0) out vec2 texcoord;\n"
		"out gl_PerVertex { \n"
		"    vec4 gl_Position;\n"
		"};\n"
		"void main() {\n"
		"   texcoord = inTexCoords;\n"
		"   gl_Position = ubuf.mvp * pos;\n"
		"}\n";

	const char *fragShaderText =
		"#version 400\n"
		"#extension GL_ARB_separate_shader_objects : enable\n"
		"#extension GL_ARB_shading_language_420pack : enable\n"
		"layout (binding = 1) uniform sampler2D tex;\n"
		"layout (location = 0) in vec2 texcoord;\n"
		"layout (location = 0) out vec4 outColor;\n"
		"void main() {\n"
		"   outColor = textureLod(tex, texcoord, 0.0);\n"
		"}\n";*/

	char *vertShaderText =
		"#version 400\n"
		"#extension GL_ARB_separate_shader_objects : enable\n"
		"#extension GL_ARB_shading_language_420pack : enable\n"
		"layout (std140, set = 0, binding = 0) uniform buf {\n"
		"    mat4 mvp;\n"
		"} ubuf;\n"
		"layout (location = 0) in vec4 pos;\n"
		"layout (location = 1) in vec2 inTexCoords;\n"
		"layout (location = 0) out vec2 outTexCoords;\n"
		"out gl_PerVertex { \n"
		"    vec4 gl_Position;\n"
		"};\n"
		"void main() {\n"
		"   gl_Position = ubuf.mvp * pos;\n"
		"   outTexCoords = inTexCoords;\n"
		"}\n";

	char *fragShaderText =
		"#version 400\n"
		"#extension GL_ARB_separate_shader_objects : enable\n"
		"#extension GL_ARB_shading_language_420pack : enable\n"
		"layout(push_constant) uniform pushBlock {\n"
		"    int iFoo;\n"
		"    float fBar;\n"
		"} pushConstantsBlock;\n"
		"layout (binding = 1) uniform sampler2D tex;\n"
		"layout (location = 0) in vec2 inTexCoords;\n"
		"layout (location = 0) out vec4 outColor;\n"
		"void main() {\n"

		"    vec4 green = vec4(0.0, 1.0, 0.0, 1.0);\n"
		"    vec4 red   = vec4(1.0, 0.0, 0.0, 1.0);\n"

		// Start with passing color
		"    vec4 resColor = green;\n"

		// See if we've read in the correct push constants
		"    if (pushConstantsBlock.iFoo != 2)\n"
		"        resColor = red;\n"
		"    if (pushConstantsBlock.fBar != 1.0f)\n"
		"        resColor = red;\n"

		// Create a border to see the cube more easily
		"   if (inTexCoords.x < 0.01 || inTexCoords.x > 0.99)\n"
		"       resColor *= vec4(0.1, 0.1, 0.1, 1.0);\n"
		"   if (inTexCoords.y < 0.01 || inTexCoords.y > 0.99)\n"
		"       resColor *= vec4(0.1, 0.1, 0.1, 1.0);\n"
		"   outColor = textureLod(tex, inTexCoords, 0.0) * resColor;\n"
		"}\n";


	Shader* vertexShader = static_cast<Shader*>(ShaderManager::GetSingleton().Create("Vertex"));
	vertexShader->SetEntryName("main");
	vertexShader->SetShaderType(VERTEX);
	vertexShader->SetShaderText(vertShaderText);

	Shader* fragmentShader = static_cast<Shader*>(ShaderManager::GetSingleton().Create("Frag"));
	fragmentShader->SetEntryName("main");
	fragmentShader->SetShaderType(FRAGMENT);
	fragmentShader->SetShaderText(fragShaderText);

	ShaderParameters* pFragParam = fragmentShader->GetShaderParameters();
	pFragParam->AddParamter("iFoo", GCT_INT1, 1);
	pFragParam->AddParamter("fBar", GCT_FLOAT1, 1);
	pFragParam->SetParameter("iFoo", 2);
	pFragParam->SetParameter("fBar", 1.0f);

	mat->AddShader(vertexShader);
	mat->AddShader(fragmentShader);

	/*Mesh* mesh = MeshManager::CreateMesh("Mesh", sizeof(g_vb_solid_face_colors_Data) / sizeof(g_vb_solid_face_colors_Data[0]),
		sizeof(g_vb_solid_face_colors_Data[0]), (void*)g_vb_solid_face_colors_Data);*/
	/*Mesh* mesh = MeshManager::CreateMesh("Mesh", sizeof(g_vb_texture_Data) / sizeof(g_vb_texture_Data[0]),
		sizeof(g_vb_texture_Data[0]), (void*)g_vb_texture_Data);*/
	//Mesh* mesh = MeshManager::CreateMesh("Mesh", sizeof(g_vb_texture_index_data) / sizeof(g_vb_texture_index_data[0]),
	//	sizeof(g_vb_texture_index_data[0]), (void*)g_vb_texture_index_data, IT_16BIT, sizeof(g_ib_data) / sizeof(g_ib_data[0]), (void*)g_ib_data);
	//VertexDeclaration* pDecl = mesh->GetVertexData()->GetVertexDeclaration();
	//pDecl->RemoveAllElements();
	//pDecl->AddElement(0, 0, VET_FLOAT4, VES_POSITION);
	////pDecl->AddElement(0, 16, VET_COLOUR, VES_DIFFUSE);
	//pDecl->AddElement(0, 16, VET_FLOAT2, VES_TEXTURE_COORDINATES);
	//mat->AddTexture(texture);

	//GameObject* go = GameObjectManager::CreateGameObject("go");
	//GameObject* go2 = GameObjectManager::CreateGameObject("go2");
	//go->GetTransform()->SetPosition(glm::vec3(0.0f, -3.0f, 0.0f));
	//go2->GetTransform()->SetPosition(glm::vec3(0.0f, 3.0f, 0.0f));
	//
	//MeshRenderer* render1 = go->AddComponent<MeshRenderer>();
	//render1->SetMaterial(mat);
	//render1->SetMesh(mesh);

	//MeshRenderer* render2 = go2->AddComponent<MeshRenderer>();
	//render2->SetMaterial(mat);
	//render2->SetMesh(mesh);

	MeshLoader::Init();
	BlurEffect blurEffect(1366, 768);
	///GameObject* pObjGo = MeshLoader::LoadScene("dabrovic-sponza/sponza_n.obj");
	GameObject* pObjGo = MeshLoader::LoadScene("crytek-sponza/sponza.obj");

	Renderer* pRenderer = root->GetRenderer();
	pRenderer->Initialize("Test", 1366, 768);

	blurEffect.SetCamera(pRenderer->GetCamera());

	CameraOperation co(pRenderer->GetCamera(), 60.0f, 0.4f);
	pRenderer->GetCamera()->GetGameObject()->GetTransform()->SetPosition(glm::vec3(0.35f, 0.35f, 0.35f));
	pRenderer->GetCamera()->GetGameObject()->GetTransform()->SetRotation(glm::quat(glm::vec3(0.0f, 80.0f, 0)));
	pRenderer->GetCamera()->SetFarPlane(5000.0f);
	pRenderer->GetCamera()->SetNearPlane(20.0f);
	pRenderer->GetCamera()->SetClearColor(0.5f, 0.7f, 0.9f, 1.0f);
	GameObjectManager::GetSingleton().GetLightList()[0]->GetGameObject()->GetTransform()->SetPosition(glm::vec3(0.35f, 30.0f, 0.0f));
	GameObjectManager::GetSingleton().GetLightList()[0]->GetGameObject()->GetTransform()->SetForward(glm::vec3(5.0f, -1.0f, 2.0f));
	//GameObjectManager::GetSingleton().GetLightList()[0]->SetLightType(Light::LT_SPOTLIGHT);

	ShadowMap shadowMap(1366, 768, GameObjectManager::GetSingleton().GetLightList()[0]);
	ShadowCamera* shadowCam = shadowMap.GetShadowCamera();
	shadowCam->GetGameObject()->SetParent(GameObjectManager::GetSingleton().GetLightList()[0]->GetGameObject());
	/*shadowCam->SetNearPlane(pRenderer->GetCamera()->GetNearPlane());
	shadowCam->SetFarPlane(pRenderer->GetCamera()->GetFarPlane());*/
	shadowCam->SetNearPlane(-450.0f);
	shadowCam->SetFarPlane(450.0f);
	shadowCam->SetFOV(pRenderer->GetCamera()->GetFOV());
	shadowCam->UpdateCameraInfoToShader();

	shadowCam->UpdateTransform();
	glm::mat4x4 lightSpace = shadowCam->GetProjectionMatrix() * shadowCam->GetViewMatrix();
	for (size_t i = 0; i < pObjGo->GetChildCount(); i++)
	{
		GameObject* pChild = pObjGo->GetChild(i);
		MeshRenderer* pMeshRenderer = pChild->GetComponent<MeshRenderer>();
		if (pMeshRenderer != NULL)
		{
			for (size_t j = 0; j < pMeshRenderer->GetMaterialCount(); j++)
			{
				Material* pMaterial = pMeshRenderer->GetMaterial(j);
				if (pMaterial != NULL)
				{
					pMaterial->GetVertexShader()->GetUniformShaderParameters()->SetParameter("LightMatrix", lightSpace);
					if ((pMaterial->GetTextureCount() < 2 && (pMaterial->GetFragmentShader()->GetName() == "TextureFrag")) || 
						(pMaterial->GetTextureCount() < 3 && (pMaterial->GetFragmentShader()->GetName() == "TextureNoramlFrag")))
					{
						pMaterial->AddTexture(shadowCam->GetRenderTexture());
					}

					if (pMaterial->GetName().find("brick") != std::string::npos || pMaterial->GetName().find("floor") != std::string::npos /*|| 
						pMaterial->GetName().find("column") != std::string::npos*/)
					{
						pMeshRenderer->SetCastShadow(false);
					}
				}
			}
		}
	}

	while (true)
	{
		clock_t startTime = clock();
		if (!pRenderer->Render())
		{
			break;
		}
		clock_t endTime = clock();
		clock_t duration = endTime - startTime;

		float deltaTime = (float)duration / CLOCKS_PER_SEC;
		co.Update(deltaTime);
	}

	delete root;
	return 0;
}