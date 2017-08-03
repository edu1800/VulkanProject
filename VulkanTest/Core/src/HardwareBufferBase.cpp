#include "../include/HardwareBufferBase.h"
#include <string>

namespace Vulkan
{
	HardwareBuffer::HardwareBuffer()
		: m_u32LockSize(0)
		, m_u32LockStart(0)
		, m_u32SizeInBytes(0)
		, m_bisLocked(false)
	{
	}

	HardwareBuffer::~HardwareBuffer()
	{
	}

	void * HardwareBuffer::Lock(unsigned int offset, unsigned int length)
	{
		void* ret = NULL;
		if (offset + length > m_u32SizeInBytes)
		{
			//error
			return NULL;
		}
		else
		{
			ret = lockImpl(offset, length);
			m_bisLocked = true;
		}

		m_u32LockStart = offset;
		m_u32LockSize = length;

		return ret;
	}

	void * HardwareBuffer::Lock()
	{
		return Lock(0, m_u32SizeInBytes);
	}

	void HardwareBuffer::UnLock()
	{
		if (!IsLocked())
		{
			return;
		}

		unLockImpl();
		m_bisLocked = false;
	}
}