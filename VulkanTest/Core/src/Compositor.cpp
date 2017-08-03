#include "..\include\Compositor.h"

namespace Vulkan
{
	Compositor::Compositor(ResourceManager * pResourceMgr, const std::string & name)
		:Resource(pResourceMgr, name)
		,m_pCamera(NULL)
	{
		m_eType = COMPOSITOR;
	}

	Compositor::~Compositor()
	{
	}

	void Compositor::Load()
	{
	}

	void Compositor::UnLoad()
	{
	}

	void Compositor::AddPass(Material * pMaterial, CompositorTextureMode mode, int width, int height, Texture* texture)
	{
		m_listCompositor.push_back(CompositorPass(pMaterial, mode, width, height, texture));
	}

	CompositorPass * Compositor::GetCompositorPass(unsigned int index)
	{
		if (index >= GetPassCount())
		{
			return NULL;
		}

		return &m_listCompositor[index];
	}

}
