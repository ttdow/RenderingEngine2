#pragma once

#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>

#include "Scene.h"

namespace Engine
{
	class AssetLoader
	{
	public:

		static std::unique_ptr<Engine::Scene> LoadGLTF(const std::string& filePath);

	private:

		static bool ParseGLTF();
		static bool LoadExtensions(fastgltf::Asset& gltf);
		static bool LoadScenes(fastgltf::Asset& gltf, std::unique_ptr<Engine::Scene>& scene);
		static bool LoadNodes(fastgltf::Asset& gltf);
	};
}