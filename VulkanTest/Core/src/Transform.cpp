#include "../include/Transform.h"
#include "../../ThirdParty/glm/gtx/transform.hpp"
#include "../include/GameObject.h"

namespace Vulkan
{
	Transform::Transform()
		: m_Position(0.0f, 0.0f, 0.0f)
		, m_Rotation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f))
		, m_Scale(1.0f, 1.0f, 1.0f)
		, m_Forward(0.0f, 0.0f, 1.0f)
		, m_Right(1.0f, 0.0f, 0.0f)
		, m_Up(0.0f, 1.0f, 0.0f)
		, m_IsUpdateTransform(false)
	{
	}

	Transform::~Transform()
	{
	}

	void Transform::SetRotation(const glm::quat & quat)
	{
		m_Rotation = quat;
		m_Forward = quat * glm::vec3(0, 0, 1);
		m_Up = quat * glm::vec3(0, 1, 0);
		m_Right = glm::cross(m_Up, m_Forward);
		UpdateTransformMatrix();
	}

	void Transform::SetForward(const glm::vec3 & forward)
	{
		glm::quat q = glm::rotation(m_Forward, forward);
		q = m_Rotation * q;
		SetRotation(q);
	}

	void Transform::SetRight(const glm::vec3 & right)
	{
		glm::quat q = glm::rotation(m_Right, right);
		q = m_Rotation * q;
		SetRotation(q);
	}

	void Transform::SetUp(const glm::vec3 & up)
	{
		glm::quat q = glm::rotation(m_Up, up);
		q = m_Rotation * q;
		SetRotation(q);
	}

	void Transform::UpdateTransformMatrix()
	{
		glm::mat4 translate = glm::translate(m_Position);
		glm::mat4 rotation = glm::toMat4(m_Rotation);
		glm::mat4 scale = glm::scale(m_Scale);

		m_TransformMatrix = translate * rotation * scale;

		GameObject* pParent = m_pGameObject->GetParent();
		while (pParent != NULL)
		{
			m_TransformMatrix = pParent->GetTransform()->GetTransformMatrix() * m_TransformMatrix;
			pParent = pParent->GetParent();
		}

		m_IsUpdateTransform = true;
	}

	glm::mat4 Transform::GetTransformMatrix()
	{
		return m_TransformMatrix;
	}
}

