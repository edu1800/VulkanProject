#ifndef _CAMERA_OPERATION_H_
#define _CAMERA_OPERATION_H_

#pragma once

namespace Vulkan
{
	class Camera;
	class Transform;
	class CameraOperation
	{
	public:
		CameraOperation(Camera* pCamera, float transformRatio, float rotateRatio);
		virtual ~CameraOperation();

		void Update(float deltaTime);

	private:
		Camera* m_pCamera;
		Transform* m_pTransform;
		float m_fTransformRatio;
		float m_fRotateRatio;
	};
}

#endif
