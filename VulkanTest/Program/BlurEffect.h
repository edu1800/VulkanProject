#ifndef _BLUR_EFFECT_H_
#define _BLUR_EFFECT_H_

#pragma once

namespace Vulkan
{
	class Camera;
	class BlurEffect
	{
	public:
		BlurEffect(int widowWidth, int windowHeight);
		~BlurEffect();

		void SetCamera(Camera* pCamera);
	};
}

#endif
