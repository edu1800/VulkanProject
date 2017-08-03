#ifndef _COMMON_UTIL_H_
#define _COMMON_UTIL_H_

#include <string>

#pragma once

#define VEC_DELETE_POINTER(vec)				\
	for (size_t i = 0; i < vec.size(); i++) \
	{										\
		if (vec[i] != NULL)					\
			delete vec[i];					\
	}										\
	vec.clear();

#define SAFE_DELETE(pointer)	\
	if (pointer != NULL)		\
		delete pointer;			\
	pointer = NULL;

#define SINGLETON_DECLARE(class_name)		\
	static class_name* GetSingletonPtr();	\
	static class_name& GetSingleton();

#define SINGLETON_IMPL(class_name)									\
	template<> class_name* Singleton<class_name>::m_sSingleton = 0; \
    class_name* class_name::GetSingletonPtr()						\
	{																\
		return m_sSingleton;										\
	}																\
	class_name& class_name::GetSingleton()							\
	{																\
		return *m_sSingleton;										\
	}

#define GET_SET_DECLARE(name, type, arg)			\
	void Set##name##(type _value) { arg = _value; }	\
	type Get##name##() const { return arg; }

#define GET_SET_DECLARE_STRUCT(name, type, arg)				\
	void Set##name##(const type& _value) { arg = _value; }	\
	const type& Get##name##() const { return arg; }

class CommonUtil
{
public:
	CommonUtil() {}
	~CommonUtil() {}

	static std::string GetFileExtension(const std::string& str);
	static unsigned GetHashCode(const char* str);
private:

};

#endif
