#ifndef _COMPOSITOR_MANAGER_H_
#define _COMPOSITOR_MANAGER_H_

#pragma once

#include "ResourceManager.h"
#include "Singleton.h"
#include "CommonUtil.h"

namespace Vulkan
{
	class CompositorManager : public ResourceManager, public Singleton<CompositorManager>
	{
	public:
		CompositorManager();
		virtual ~CompositorManager();

		SINGLETON_DECLARE(CompositorManager)
	protected:
		virtual Resource* createImpl(const std::string& name);
	};
}

#endif
