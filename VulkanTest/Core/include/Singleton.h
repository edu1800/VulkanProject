#ifndef _SINGLETON_H_
#define _SINGLETON_H_

#pragma once

namespace Vulkan
{
	template<typename T>
	class Singleton
	{
	private:
		Singleton(const Singleton<T> &);
		Singleton& operator=(const Singleton<T> &);

	protected:
		static T* m_sSingleton;

	public:
		Singleton()
		{
			m_sSingleton = static_cast<T*>(this);
		}

		~Singleton()
		{
			m_sSingleton = 0;
		}

		static T& GetSingleton() { return *m_sSingleton; }
		static T* GetSingletonPtr() { return m_sSingleton; }
	};
}
#endif
