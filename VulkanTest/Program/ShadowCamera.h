#ifndef _SHADOW_CAMERA_H_
#define _SHADOW_CAMERA_H_

#pragma once

#include "../Core/include/Camera.h"

namespace Vulkan
{
	class MeshRenderer;
	class Light;
	class Texture;
	class ShadowCamera : public Camera
	{
	public:
		ShadowCamera();
		virtual ~ShadowCamera();

		void Init(int width, int height, Light* pLight);
		virtual bool IsRender(MeshRenderer* pRenderer);

		virtual bool UpdateViewMatrix();
		virtual void UpdateProjectionMatrix();

		void UpdateCameraInfoToShader();
	protected:
		Light* m_pLight;
	};
}

#endif
