#ifndef _HARDWARE_BUFFER_BASE_H_
#define _HARDWARE_BUFFER_HBAE_H_

#pragma once

namespace Vulkan
{
	class HardwareBuffer
	{
	public:
		HardwareBuffer();
		virtual ~HardwareBuffer();

		virtual void* Lock(unsigned int offset, unsigned int length);
		virtual void* Lock();
		virtual void UnLock();

		virtual void ReadData(unsigned int offset, unsigned int length, void* pDest) = 0;
		virtual void WriteData(unsigned int offset, unsigned int length, void* pSrc) = 0;

		unsigned int GetSizeInBytes() const { return m_u32SizeInBytes; }
		bool IsLocked() { return m_bisLocked; }

	protected:
		virtual void* lockImpl(unsigned int offset, unsigned int length) = 0;
		virtual void unLockImpl() = 0;

		unsigned int m_u32SizeInBytes;
		unsigned int m_u32LockStart;
		unsigned int m_u32LockSize;
		bool m_bisLocked;
	};
}

#endif

