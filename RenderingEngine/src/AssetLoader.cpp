#include "pch.h"

#include "AssetLoader.h"

std::unique_ptr<Engine::Scene> Engine::AssetLoader::LoadGLTF(const std::string& filePath)
{
	std::cout << "Loading glTF file: " << filePath << std::endl;
	

	std::unique_ptr<Engine::Scene> scene = std::make_unique<Engine::Scene>("Root");

	// Save the system file path.
	std::filesystem::path path = filePath;

	// Read file from disk.
	auto gltfFile = fastgltf::GltfDataBuffer::FromPath(path);
	if (!gltfFile)
	{
		throw std::runtime_error("Failed to load glTF file!");
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
	LoadScenes(gltf, scene);
	LoadNodes(gltf);

	return scene;
}

bool Engine::AssetLoader::ParseGLTF()
{
	return true;
}

bool Engine::AssetLoader::LoadExtensions(fastgltf::Asset& gltf)
{
	return true;
}

bool Engine::AssetLoader::LoadScenes(fastgltf::Asset& gltf, std::unique_ptr<Engine::Scene>& sceneGraph)
{
	// Get number of scenes in glTF file.
	const uint32_t nScenes = gltf.scenes.size();
	if (nScenes <= 0)
	{
		throw std::runtime_error("There are no scenes in the loaded glTF file!");
	}
	
	// Allocate memory for top-level scene nodes.
	sceneGraph->root->children.resize(nScenes);

	std::cout << "Scenes: " << nScenes << std::endl;

	for (uint32_t i = 0; i < nScenes; i++)
	{
		// Get a reference to the current scene in the glTF file.
		fastgltf::Scene& scene = gltf.scenes[i];

		// Create a new scene graph node.
		sceneGraph->root->children[i] = new Engine::SceneNode(scene.name.c_str());
		Engine::SceneNode& newNode = *sceneGraph->root->children[i];

		// Allocate memory for the node's children.
		newNode.children.reserve(scene.nodeIndices.size());

		std::cout << scene.name << std::endl;
		std::cout << newNode.name << std::endl;
		
		//newNode.name = scene.name;
		//newNode.children.reserve(scene.nodeIndices.size());

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