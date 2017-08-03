#ifndef _MESH_RENDERER_H_
#define _MESH_RENDERER_H_

#pragma once

#include "CommonUtil.h"
#include "Component.h"

namespace Vulkan
{
	class Mesh;
	class Material;
	class MeshRenderer : public Component
	{
	public:
		MeshRenderer();
		virtual ~MeshRenderer();

		void SetMesh(Mesh* pMesh) { m_pMesh = pMesh; }
		Mesh* GetMesh() const { return m_pMesh; }

		void AddMaterial(Material* pMaterial) { m_ListMaterial.push_back(pMaterial); }
		void SetMaterial(Material* pMaterial, int index);
		unsigned int GetMaterialCount() { return m_ListMaterial.size(); }
		Material* GetMaterial(int index);

		GET_SET_DECLARE(CastShadow, bool, m_bCastShadow)
	protected:
		Mesh* m_pMesh;
		std::vector<Material*> m_ListMaterial;
		bool m_bCastShadow;
	};
}

#endif
