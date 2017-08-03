#ifndef _SHADER_PARAMETER_H_
#define _SHADER_PARAMETER_H_

#pragma once

#include <map>
#include <string>
#include <vector>
#include "../../ThirdParty/glm/matrix.hpp"

namespace Vulkan
{
	enum GpuConstantType
	{
		GCT_FLOAT1 = 1,
		GCT_FLOAT2 = 2,
		GCT_FLOAT3 = 3,
		GCT_FLOAT4 = 4,
		GCT_SAMPLER1D = 5,
		GCT_SAMPLER2D = 6,
		GCT_SAMPLER3D = 7,
		GCT_SAMPLERCUBE = 8,
		GCT_SAMPLER1DSHADOW = 9,
		GCT_SAMPLER2DSHADOW = 10,
		GCT_SAMPLER2DARRAY = 11,
		GCT_MATRIX_2X2 = 12,
		GCT_MATRIX_2X3 = 13,
		GCT_MATRIX_2X4 = 14,
		GCT_MATRIX_3X2 = 15,
		GCT_MATRIX_3X3 = 16,
		GCT_MATRIX_3X4 = 17,
		GCT_MATRIX_4X2 = 18,
		GCT_MATRIX_4X3 = 19,
		GCT_MATRIX_4X4 = 20,
		GCT_INT1 = 21,
		GCT_INT2 = 22,
		GCT_INT3 = 23,
		GCT_INT4 = 24,
		GCT_UNKNOWN = 99
	};

	struct ShaderParameter
	{
		GpuConstantType eConstType;
		unsigned int physicalIndex;
		unsigned int arraySize;
		unsigned int elementSize;

		bool isFloat() const
		{
			return isFloat(eConstType);
		}

		static bool isFloat(GpuConstantType c)
		{
			switch (c)
			{
			case GCT_INT1:
			case GCT_INT2:
			case GCT_INT3:
			case GCT_INT4:
			case GCT_SAMPLER1D:
			case GCT_SAMPLER2D:
			case GCT_SAMPLER2DARRAY:
			case GCT_SAMPLER3D:
			case GCT_SAMPLERCUBE:
			case GCT_SAMPLER1DSHADOW:
			case GCT_SAMPLER2DSHADOW:
				return false;
			default:
				return true;
			};

		}

		bool isSampler() const
		{
			return isSampler(eConstType);
		}

		static bool isSampler(GpuConstantType c)
		{
			switch (c)
			{
			case GCT_SAMPLER1D:
			case GCT_SAMPLER2D:
			case GCT_SAMPLER2DARRAY:
			case GCT_SAMPLER3D:
			case GCT_SAMPLERCUBE:
			case GCT_SAMPLER1DSHADOW:
			case GCT_SAMPLER2DSHADOW:
				return true;
			default:
				return false;
			};

		}


		/** Get the element size of a given type, including whether to pad the
		elements into multiples of 4 (e.g. SM1 and D3D does, GLSL doesn't)
		*/
		static size_t getElementSize(GpuConstantType ctype, bool padToMultiplesOf4)
		{
			if (padToMultiplesOf4)
			{
				switch (ctype)
				{
				case GCT_FLOAT1:
				case GCT_INT1:
				case GCT_SAMPLER1D:
				case GCT_SAMPLER2D:
				case GCT_SAMPLER2DARRAY:
				case GCT_SAMPLER3D:
				case GCT_SAMPLERCUBE:
				case GCT_SAMPLER1DSHADOW:
				case GCT_SAMPLER2DSHADOW:
				case GCT_FLOAT2:
				case GCT_INT2:
				case GCT_FLOAT3:
				case GCT_INT3:
				case GCT_FLOAT4:
				case GCT_INT4:
					return 4;
				case GCT_MATRIX_2X2:
				case GCT_MATRIX_2X3:
				case GCT_MATRIX_2X4:
					return 8; // 2 float4s
				case GCT_MATRIX_3X2:
				case GCT_MATRIX_3X3:
				case GCT_MATRIX_3X4:
					return 12; // 3 float4s
				case GCT_MATRIX_4X2:
				case GCT_MATRIX_4X3:
				case GCT_MATRIX_4X4:
					return 16; // 4 float4s
				default:
					return 4;
				};
			}
			else
			{
				switch (ctype)
				{
				case GCT_FLOAT1:
				case GCT_INT1:
				case GCT_SAMPLER1D:
				case GCT_SAMPLER2D:
				case GCT_SAMPLER2DARRAY:
				case GCT_SAMPLER3D:
				case GCT_SAMPLERCUBE:
				case GCT_SAMPLER1DSHADOW:
				case GCT_SAMPLER2DSHADOW:
					return 1;
				case GCT_FLOAT2:
				case GCT_INT2:
					return 2;
				case GCT_FLOAT3:
				case GCT_INT3:
					return 3;
				case GCT_FLOAT4:
				case GCT_INT4:
					return 4;
				case GCT_MATRIX_2X2:
					return 4;
				case GCT_MATRIX_2X3:
				case GCT_MATRIX_3X2:
					return 6;
				case GCT_MATRIX_2X4:
				case GCT_MATRIX_4X2:
					return 8;
				case GCT_MATRIX_3X3:
					return 9;
				case GCT_MATRIX_3X4:
				case GCT_MATRIX_4X3:
					return 12;
				case GCT_MATRIX_4X4:
					return 16;
				default:
					return 4;
				};

			}
		}

		ShaderParameter()
			:eConstType(GCT_UNKNOWN)
			,arraySize(1)
			,elementSize(0)
			,physicalIndex(std::numeric_limits<unsigned int>::max())
		{

		}
	};

	typedef std::vector<float> ShaderConstants;

	template<typename T>
	class ShaderParametersTemplate
	{
	public:
		typedef std::map<T, ShaderParameter> ShaderParameterMap;
		ShaderParametersTemplate();
		virtual ~ShaderParametersTemplate();

		void AddParamter(const typename std::remove_reference<T>::type key, GpuConstantType eConstType, unsigned int arraySize);
		void RemoveParamter(const typename std::remove_reference<T>::type&  key);
		void RemoveAll();
		bool ContainParameter(const typename std::remove_reference<T>::type&  key);
		size_t GetParameterCount() { return m_mapShaderParamter.size(); }
		const ShaderParameter& GetShaderParameter(const typename std::remove_reference<T>::type&  key);
		const ShaderParameterMap& GetAllShaderParameters() const { return m_mapShaderParamter; }

		void SetParameter(const typename std::remove_reference<T>::type&  key, float value);
		void SetParameter(const typename std::remove_reference<T>::type&  key, int value);
		void SetParameter(const typename std::remove_reference<T>::type&  key, const glm::vec3& value);
		void SetParameter(const typename std::remove_reference<T>::type&  key, const glm::vec4& value);
		void SetParameter(const typename std::remove_reference<T>::type&  key, const glm::mat4x4& value);
		void SetParameter(const typename std::remove_reference<T>::type&  key, const glm::mat4x4* value, unsigned int num);
		void SetParameter(const typename std::remove_reference<T>::type&  key, const float* val, unsigned int size);
		void SetParameter(const typename std::remove_reference<T>::type&  key, const int* val, unsigned int size);

		const ShaderConstants& GetShaderConstants() { return m_listConstants; }
		void SetIsPadding(bool padding) { m_bIsPadding = padding; }
	private:
		ShaderParameterMap m_mapShaderParamter;
		ShaderConstants m_listConstants;
		bool m_bIsPadding;
	};

	class ShaderParameters : public ShaderParametersTemplate<std::string>
	{
	public:
		ShaderParameters();
		virtual ~ShaderParameters();
	};

	class ShaderParametersInt : public ShaderParametersTemplate<int>
	{
	public:
		ShaderParametersInt();
		virtual ~ShaderParametersInt();
	};

	template<typename T>
	inline ShaderParametersTemplate<T>::ShaderParametersTemplate()
		:m_bIsPadding(true)
	{
	}

	template<typename T>
	inline ShaderParametersTemplate<T>::~ShaderParametersTemplate()
	{
	}
	template<typename T>
	inline void ShaderParametersTemplate<T>::AddParamter(const typename std::remove_reference<T>::type key, GpuConstantType eConstType, unsigned int arraySize)
	{
		if (m_mapShaderParamter.find(key) != m_mapShaderParamter.end())
		{
			return;
		}

		ShaderParameter param;
		param.eConstType = eConstType;
		param.arraySize = arraySize;
		param.elementSize = ShaderParameter::getElementSize(eConstType, m_bIsPadding);
		param.physicalIndex = m_listConstants.size();
		m_listConstants.resize(m_listConstants.size() + param.arraySize * param.elementSize);
		m_mapShaderParamter[key] = param;
	}

	template<typename T>
	inline void ShaderParametersTemplate<T>::RemoveParamter(const typename std::remove_reference<T>::type & key)
	{
		ShaderParameterMap::iterator itr = m_mapShaderParamter.find(key);
		if (itr != m_mapShaderParamter.end())
		{
			ShaderParameter& param = itr->second;
			unsigned int numElems = param.arraySize * param.elementSize;

			ShaderParameterMap::iterator sItr = m_mapShaderParamter.begin();
			ShaderParameterMap::iterator sItrEnd = m_mapShaderParamter.end();
			for (; sItr != sItrEnd; ++sItr)
			{
				ShaderParameter& otherParam = sItr->second;
				if (otherParam.physicalIndex > param.physicalIndex)
				{
					otherParam.physicalIndex -= numElems;
				}
			}

			ShaderConstants::iterator cItr = m_listConstants.begin();
			std::advance(cItr, param.physicalIndex);
			ShaderConstants::iterator cItrEnd = cItr;
			std::advance(cItrEnd, numElems);
			m_listConstants.erase(cItr, cItrEnd);
			m_mapShaderParamter.erase(key);
		}
	}
	template<typename T>
	inline void ShaderParametersTemplate<T>::RemoveAll()
	{
		m_listConstants.clear();
		m_mapShaderParamter.clear();
	}
	template<typename T>
	inline bool ShaderParametersTemplate<T>::ContainParameter(const typename std::remove_reference<T>::type & key)
	{
		return m_mapShaderParamter.find(key) != m_mapShaderParamter.end();
	}
	template<typename T>
	inline const ShaderParameter & ShaderParametersTemplate<T>::GetShaderParameter(const typename std::remove_reference<T>::type & key)
	{
		return m_mapShaderParamter[key];
	}
	template<typename T>
	inline void ShaderParametersTemplate<T>::SetParameter(const typename std::remove_reference<T>::type & key, float value)
	{
		SetParameter(key, &value, 1);
	}
	template<typename T>
	inline void ShaderParametersTemplate<T>::SetParameter(const typename std::remove_reference<T>::type & key, int value)
	{
		SetParameter(key, &value, 1);
	}
	template<typename T>
	inline void ShaderParametersTemplate<T>::SetParameter(const typename std::remove_reference<T>::type & key, const glm::vec3 & value)
	{
		SetParameter(key, &value[0], 3);
	}
	template<typename T>
	inline void ShaderParametersTemplate<T>::SetParameter(const typename std::remove_reference<T>::type & key, const glm::vec4 & value)
	{
		SetParameter(key, &value[0], 4);
	}
	template<typename T>
	inline void ShaderParametersTemplate<T>::SetParameter(const typename std::remove_reference<T>::type & key, const glm::mat4x4 & value)
	{
		SetParameter(key, &value[0][0], 16);
	}
	template<typename T>
	inline void ShaderParametersTemplate<T>::SetParameter(const typename std::remove_reference<T>::type & key, const glm::mat4x4 * value, unsigned int num)
	{
		SetParameter(key, &((*value)[0][0]), 16 * num);
	}
	template<typename T>
	inline void ShaderParametersTemplate<T>::SetParameter(const typename std::remove_reference<T>::type & key, const float * val, unsigned int size)
	{
		ShaderParameterMap::iterator itr = m_mapShaderParamter.find(key);
		if (itr != m_mapShaderParamter.end())
		{
			ShaderParameter& param = itr->second;
			unsigned int numElems = param.arraySize * param.elementSize;
			unsigned int num = size > numElems ? numElems : size;
			memcpy(&m_listConstants[param.physicalIndex], val, sizeof(float) * num);
		}
	}
	template<typename T>
	inline void ShaderParametersTemplate<T>::SetParameter(const typename std::remove_reference<T>::type & key, const int * val, unsigned int size)
	{
		ShaderParameterMap::iterator itr = m_mapShaderParamter.find(key);
		if (itr != m_mapShaderParamter.end())
		{
			ShaderParameter& param = itr->second;
			unsigned int numElems = param.arraySize * param.elementSize;
			unsigned int num = size > numElems ? numElems : size;
			memcpy(&m_listConstants[param.physicalIndex], val, sizeof(int) * num);
		}
	}
}

#endif
