#ifndef _TRANSFORM_H_
#define _TRANSFORM_H_

#pragma once

#include "../../ThirdParty/glm/matrix.hpp"
#include "../../ThirdParty/glm/gtx/quaternion.hpp"
#include "CommonUtil.h"
#include "Component.h"

namespace Vulkan
{
	class Transform : public Component
	{
	public:
		Transform();
		virtual ~Transform();

		void SetPosition(const glm::vec3& pos) { m_Position = pos; UpdateTransformMatrix(); }
		glm::vec3 GetPosition() const { return m_Position; }

		void SetRotation(const glm::quat& quat);
		glm::quat GetRotation() const { return m_Rotation; }

		void SetScale(const glm::vec3& scale) { m_Scale = scale; UpdateTransformMatrix(); }
		glm::vec3 GetScale() const { return m_Scale; }

		void SetForward(const glm::vec3& forward);
		const glm::vec3& GetForward() const { return m_Forward; }

		void SetRight(const glm::vec3& right);
		const glm::vec3& GetRight() const { return m_Right; }

		void SetUp(const glm::vec3& up);
		const glm::vec3& GetUp() const { return m_Up; }

		void UpdateTransformMatrix();

		glm::mat4 GetTransformMatrix();

		GET_SET_DECLARE(IsUpdateTransform, bool, m_IsUpdateTransform)

	protected:
		glm::vec3 m_Position;
		glm::quat m_Rotation;
		glm::vec3 m_Scale;

		glm::vec3 m_Forward;
		glm::vec3 m_Right;
		glm::vec3 m_Up;

		glm::mat4 m_TransformMatrix;

		bool m_IsUpdateTransform;
		
	};
}
#endif
