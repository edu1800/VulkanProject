#ifndef _RENDERER_H_
#define _RENDERER_H_

#pragma once

#include <string>
#include "VertexIndexDefine.h"

namespace Vulkan
{
	class Camera;
	class VertexBuffer;
	class IndexBuffer;
	class GameObject;
	class ResourceManager;
	class Shader;
	class Mesh;
	class Texture;
	class Renderer
	{
	public:
		Renderer();
		~Renderer();

		virtual void Initialize(const char * windowName, int width, int height);
		virtual bool Render();

		Camera* GetCamera() const { return m_pCamera; }
		virtual VertexBuffer* CreateVertexBuffer(unsigned int vertexSize, unsigned int numVerts) = 0;
		virtual IndexBuffer* CreateIndexBuffer(IndexType indexType, unsigned int numIndexes) = 0;
		virtual GameObject* CreateGameObject(ResourceManager * pResourceMgr, const std::string & name) = 0;
		virtual Shader* CreateShader(ResourceManager* pResourceMgr, const std::string& name) = 0;
		virtual Texture* CreateTexture(ResourceManager* pResourceMgr, const std::string& name, const std::string& filename) = 0;
		virtual Texture* CreateRenderTexture(ResourceManager* pResourceMgr, int width, int height, bool isFrameBuffer, const std::string& name) = 0;

	protected:
		GameObject* m_pCameraGo;
		Camera* m_pCamera;
	};
}

#endif
