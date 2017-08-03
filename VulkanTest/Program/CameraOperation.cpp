#include "CameraOperation.h"
#include <Windows.h>
#include <WinUser.h>
#include "../Core/include/Transform.h"
#include "../Core/include/GameObject.h"
#include "../Core/include/Camera.h"

namespace Vulkan
{
	CameraOperation::CameraOperation(Camera * pCamera, float transformRatio, float rotateRatio)
		:m_pCamera(pCamera)
		,m_fTransformRatio(transformRatio)
		,m_fRotateRatio(rotateRatio)
	{
		m_pTransform = m_pCamera->GetGameObject()->GetTransform();
	}

	CameraOperation::~CameraOperation()
	{
	}

	void CameraOperation::Update(float deltaTime)
	{
		if (GetAsyncKeyState('W'))
		{
			glm::vec3 pos = m_pTransform->GetPosition();
			pos += m_pTransform->GetForward() * deltaTime * m_fTransformRatio;
			m_pTransform->SetPosition(pos);
		}

		if (GetAsyncKeyState('A'))
		{
			glm::vec3 pos = m_pTransform->GetPosition();
			pos += m_pTransform->GetRight() * deltaTime * m_fTransformRatio;
			m_pTransform->SetPosition(pos);
		}

		if (GetAsyncKeyState('S'))
		{
			glm::vec3 pos = m_pTransform->GetPosition();
			pos -= m_pTransform->GetForward() * deltaTime * m_fTransformRatio;
			m_pTransform->SetPosition(pos);
		}

		if (GetAsyncKeyState('D'))
		{
			glm::vec3 pos = m_pTransform->GetPosition();
			pos -= m_pTransform->GetRight() * deltaTime * m_fTransformRatio;
			m_pTransform->SetPosition(pos);
		}

		if (GetAsyncKeyState('Q'))
		{
			glm::vec3 pos = m_pTransform->GetPosition();
			pos -= m_pTransform->GetUp() * deltaTime * m_fTransformRatio;
			m_pTransform->SetPosition(pos);
		}

		if (GetAsyncKeyState('E'))
		{
			glm::vec3 pos = m_pTransform->GetPosition();
			pos += m_pTransform->GetUp() * deltaTime * m_fTransformRatio;
			m_pTransform->SetPosition(pos);
		}

		float angle = 1.0f;
		if (GetAsyncKeyState('Z'))
		{
			glm::quat q = m_pTransform->GetRotation() * glm::quat(glm::vec3(0, deltaTime * m_fRotateRatio * angle, 0));
			m_pTransform->SetRotation(q);
		}

		if (GetAsyncKeyState('X'))
		{		
			glm::quat q = m_pTransform->GetRotation() * glm::quat(glm::vec3(0, deltaTime * m_fRotateRatio * (-angle), 0));
			m_pTransform->SetRotation(q);
		}

		if (GetAsyncKeyState('C'))
		{
			glm::quat q = m_pTransform->GetRotation() * glm::quat(glm::vec3(deltaTime * m_fRotateRatio * angle, 0, 0));
			m_pTransform->SetRotation(q);
		}

		if (GetAsyncKeyState('V'))
		{
			glm::quat q = m_pTransform->GetRotation() * glm::quat(glm::vec3(deltaTime * m_fRotateRatio * (-angle), 0, 0));
			m_pTransform->SetRotation(q);
		}

		if (GetAsyncKeyState('R'))
		{
			glm::quat q = m_pTransform->GetRotation() * glm::quat(glm::vec3(0, 0, deltaTime * m_fRotateRatio * angle));
			m_pTransform->SetRotation(q);
		}

		if (GetAsyncKeyState('F'))
		{
			glm::quat q = m_pTransform->GetRotation() * glm::quat(glm::vec3(0, 0, deltaTime * m_fRotateRatio * (-angle)));
			m_pTransform->SetRotation(q);
		}
	}
}