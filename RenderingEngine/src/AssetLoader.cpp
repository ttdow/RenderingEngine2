#include "pch.h"

#include "AssetLoader.h"

bool Engine::AssetLoader::LoadGLTF(const std::string& filePath)
{
	std::cout << "Loading glTF file: " << filePath << std::endl;

	// Save the system file path.
	std::filesystem::path path = filePath;

	// Read file from disk.
	auto gltfFile = fastgltf::GltfDataBuffer::FromPath(path);
	if (!gltfFile)
	{
		std::cout << "Failed to load glTF file!\n";

		return false;
	}

	fastgltf::Asset gltf;

	constexpr auto gltfOptions = fastgltf::Options::DontRequireValidAssetMember |
		fastgltf::Options::AllowDouble |
		fastgltf::Options::LoadExternalBuffers;

	fastgltf::Parser parser{};

	// Determine the glTF file type so it can be parsed as such.
	fastgltf::GltfType type = fastgltf::determineGltfFileType(gltfFile.get());

	if (type == fastgltf::GltfType::GLB)
	{
		auto load = parser.loadGltfBinary(gltfFile.get(), path.parent_path(), gltfOptions);
		if (load)
		{
			gltf = std::move(load.get());
		}
	}

	LoadExtensions(gltf);
	LoadScenes(gltf);
	LoadNodes(gltf);

	return true;
}

bool Engine::AssetLoader::ParseGLTF()
{
	return true;
}

bool Engine::AssetLoader::LoadExtensions(fastgltf::Asset& gltf)
{
	//if (gltf.extensionsRequired.size() > 0)
	//{
		std::cout << "Extensions required: " << gltf.extensionsRequired.size() << std::endl;
	//}

	//if (gltf.extensionsUsed.size() > 0)
	//{
		std::cout << "Extensions used: " << gltf.extensionsUsed.size() << std::endl;
	//}

		std::cout << "Lights: " << gltf.lights.size() << std::endl;

	return true;
}

bool Engine::AssetLoader::LoadScenes(fastgltf::Asset& gltf)
{
	std::cout << "Scenes: " << gltf.scenes.size() << std::endl;

	for (uint32_t i = 0; i < gltf.scenes.size(); i++)
	{
		fastgltf::Scene& scene = gltf.scenes[i];

		std::cout << "  Scene[" << i << "]: " << scene.name << std::endl;
		std::cout << "    Nodes: " << scene.nodeIndices.size() << std::endl;
		std::cout << "    Node Indices: [";

		for (uint32_t j = 0; j < scene.nodeIndices.size(); j++)
		{
			if (j > 0)
			{
				std::cout << ", ";
			}

			std::cout << scene.nodeIndices[j];
		}

		std::cout << "]\n";
	}

	if (gltf.defaultScene.has_value())
	{
		std::cout << "Default scene index: " << gltf.defaultScene.value() << std::endl;
	}

	return true;
}

bool Engine::AssetLoader::LoadNodes(fastgltf::Asset& gltf)
{
	fastgltf::iterateSceneNodes(gltf, 0, fastgltf::math::fmat4x4(),
		[&](fastgltf::Node& node, fastgltf::math::fmat4x4 matrix)
		{
			if (node.meshIndex.has_value())
			{
				std::cout << node.meshIndex.value() << '\n';
			}
			if (node.cameraIndex.has_value())
			{
				std::cout << node.cameraIndex.value() << '\n';
			}
			if (node.lightIndex.has_value())
			{
				std::cout << "where\n";
			}
		});

	std::cout << "Nodes: " << gltf.nodes.size() << std::endl;

	for (uint32_t i = 0; i < gltf.nodes.size(); i++)
	{
		fastgltf::Node& node = gltf.nodes[i];

		std::cout << "  Node[" << i << "]: " << node.name << std::endl;
		std::cout << "    Children: " << node.children.size() << std::endl;

		auto transform = node.transform;
		if (std::holds_alternative<fastgltf::TRS>(transform))
		{

		}
		else if (std::holds_alternative<fastgltf::math::fmat4x4>(transform))
		{

		}
		else
		{
			throw std::runtime_error("Unexpected node transform type!");
		}

		if (node.meshIndex.has_value())
		{
			std::cout << "    Mesh Index: " << node.meshIndex.value() << std::endl;
		}
		
		if (node.cameraIndex.has_value())
		{
			std::cout << "    Camera Index: " << node.cameraIndex.value() << std::endl;
		}

		if (node.lightIndex.has_value())
		{
			std::cout << "    Light Index: " << node.lightIndex.value() << std::endl;
		}

		if (node.skinIndex.has_value())
		{
			std::cout << "    Skin Index: " << node.skinIndex.value() << std::endl;
		}
	}

	return true;
}