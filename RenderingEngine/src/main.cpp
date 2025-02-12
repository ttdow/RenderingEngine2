#include "pch.h"

#include "AssetLoader.h"
#include "Scene.h"

int main(int argc, char* argv[])
{
	Engine::Scene scene;
	scene.root = std::make_unique<Engine::SceneNode>();
	scene.root->

	bool loaded = Engine::AssetLoader::LoadGLTF("./res/box_face.glb");
	if (loaded)
	{
		return EXIT_SUCCESS;
	}
	else
	{
		return EXIT_FAILURE;
	}
}