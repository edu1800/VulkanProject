#include "../include/MeshRenderer.h"
#include "../include/MaterialManager.h"

namespace Vulkan
{
	MeshRenderer::MeshRenderer()
		:Component()
		,m_pMesh(NULL)
		,m_bCastShadow(false)
	{
		AddMaterial(MaterialManager::GetSingleton().GetDefaultMaterial());
	}

	MeshRenderer::~MeshRenderer()
	{
	}

	void MeshRenderer::SetMaterial(Material* pMaterial, int index)
	{
		if (index < 0 || index >= GetMaterialCount())
		{
			return;
		}

		m_ListMaterial[index] = pMaterial;
	}

	Material * MeshRenderer::GetMaterial(int index)
	{
		if (index < 0 || index >= GetMaterialCount())
		{
			return NULL;
		}

		return m_ListMaterial[index];
	}
}