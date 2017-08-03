#ifndef _FRUSTUM_H_
#define _FRUSTUM_H_

#pragma once

#include <array>
#include <math.h>
#include "../../ThirdParty/glm/glm.hpp"

namespace Vulkan
{
	class Frustum
	{
	public:
		enum side { LEFT = 0, RIGHT = 1, TOP = 2, BOTTOM = 3, BACK = 4, FRONT = 5 };

		void Update(const glm::mat4& matrix);
		bool CheckSphere(const glm::vec3& pos, float radius);

	private:
		std::array<glm::vec4, 6> planes;
	};
}

#endif
