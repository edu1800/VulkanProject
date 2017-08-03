#ifndef _MESH_LOADER_H_
#define _MESH_LOADER_H_

#pragma once

#include <string>

namespace Vulkan
{
	class GameObject;
	class MeshLoader
	{
	public:
		MeshLoader();
		~MeshLoader();

		static void Init();
		static GameObject* LoadScene(const std::string& name);
	};
}

#endif
