#ifndef _MATERIAL_PARAMETER_DEFINE_H_
#define _MATERIAL_PARAMETER_DEFINE_H_

#pragma once

namespace Vulkan
{
	enum CullMode
	{
		CM_FLAG_NONE = 0,
		CM_FLAG_FRONT = 1,
		CM_FLAG_BACK = 2,
		CM_FLAG_FRONT_AND_BACK = 3
	};

	enum ColorMask
	{
		CM_R_BIT = 0x00000001,
		CM_G_BIT = 0x00000002,
		CM_B_BIT = 0x00000004,
		CM_A_BIT = 0x00000008,
	};

	enum SceneBlendOperation
	{
		SBO_ADD = 0,
		SBO_SUBTRACT = 1,
		SBO_REVERSE_SUBTRACT = 2,
		SBO_MIN = 3,
		SBO_MAX = 4
	};

	enum SceneBlendFactor
	{
		SBF_ZERO = 0,
		SBF_ONE = 1,
		SBF_SRC_COLOUR = 2,
		SBF_DST_COLOUR = 3,
		SBF_ONE_MINUS_SRC_COLOUR = 4,
		SBF_ONE_MINUS_DST_COLOUR = 5,
		SBF_SRC_ALPHA = 6,
		SBF_DST_ALPHA = 7,
		SBF_ONE_MINUS_SRC_ALPHA = 8,
		SBF_ONE_MINUS_DST_ALPHA = 9
	};

	enum DepthCompareFunction
	{
		DCF_ALWAYS_FAIL,
		DCF_ALWAYS_PASS,
		DCF_LESS,
		DCF_LESS_EQUAL,
		DCF_EQUAL,
		DCF_NOT_EQUAL,
		DCF_GREATER_EQUAL,
		DCF_GREATER
	};

	enum StencilOperation
	{
		/// Leave the stencil buffer unchanged
		SOP_KEEP,
		/// Set the stencil value to zero
		SOP_ZERO,
		/// Set the stencil value to the reference value
		SOP_REPLACE,
		/// Increase the stencil value by 1, clamping at the maximum value
		SOP_INCREMENT,
		/// Decrease the stencil value by 1, clamping at 0
		SOP_DECREMENT,
		/// Increase the stencil value by 1, wrapping back to 0 when incrementing the maximum value
		SOP_INCREMENT_WRAP,
		/// Decrease the stencil value by 1, wrapping when decrementing 0
		SOP_DECREMENT_WRAP,
		/// Invert the bits of the stencil buffer
		SOP_INVERT
	};
}

#endif
