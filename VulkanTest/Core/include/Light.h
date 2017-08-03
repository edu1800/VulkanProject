#ifndef _LIGHT_H_
#define _LIGHT_H_

#pragma once

#include "Component.h"
#include "CommonUtil.h"
#include "../../ThirdParty/glm/matrix.hpp"

namespace Vulkan
{
	class Light : public Component
	{
	public:
		enum LightType
		{
			LT_POINT,
			LT_DIRECTION,
			LT_SPOTLIGHT
		};

		Light();
		virtual ~Light();

		GET_SET_DECLARE(LightType, LightType, m_eLightType)
		GET_SET_DECLARE_STRUCT(DiffuseColor, glm::vec3, m_DiffuseColor)
		GET_SET_DECLARE_STRUCT(Specular, glm::vec3, m_SpecularColor)
		GET_SET_DECLARE(Intensity, float, m_fIntensity)

		glm::vec3 GetDirection();

	protected:
		LightType m_eLightType;
		glm::vec3 m_DiffuseColor;
		glm::vec3 m_SpecularColor;
		float m_fIntensity;
	};
}

#endif
