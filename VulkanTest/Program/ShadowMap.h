#ifndef _SHADOW_MAP_H_
#define _SHADOW_MAP_H_

#pragma once

#include "../Core/include/CommonUtil.h"

namespace Vulkan
{
	class ShadowCamera;
	class Light;
	class ShadowMap
	{
	public:
		ShadowMap(int width, int height, Light* pLight);
		virtual ~ShadowMap();

		GET_SET_DECLARE(ShadowCamera, ShadowCamera*, m_pShadowCamera)
		GET_SET_DECLARE(Light, Light*, m_pLight)
		GET_SET_DECLARE(Width, int, m_iWidth)
		GET_SET_DECLARE(Height, int, m_iHeight)

	protected:
		int m_iWidth;
		int m_iHeight;
		ShadowCamera* m_pShadowCamera;
		Light* m_pLight;
	};
}

#endif
