#include "../include/CompositorManager.h"
#include "../include/Compositor.h"

namespace Vulkan
{
	SINGLETON_IMPL(CompositorManager)

	CompositorManager::CompositorManager()
		:ResourceManager()
		, Singleton<CompositorManager>()
	{
	}

	CompositorManager::~CompositorManager()
	{
	}

	Resource* CompositorManager::createImpl(const std::string& name)
	{
		return new Compositor(this, name);
	}
}