#pragma once

#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>

namespace Engine
{
	class AssetLoader
	{
	public:

		static bool LoadGLTF(const std::string& filePath);

	private:

		static bool ParseGLTF();
		static bool LoadExtensions(fastgltf::Asset& gltf);
		static bool LoadScenes(fastgltf::Asset& gltf);
		static bool LoadNodes(fastgltf::Asset& gltf);

	};
}