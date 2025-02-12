#pragma once

#include "pch.h"

#include "Transform.h"

namespace Engine
{
	class SceneNode
	{
	public:

		std::string name;
		std::vector<SceneNode*> children;
		SceneNode* parent;

		Engine::Transform transform;

		SceneNode() = delete;
		SceneNode(const std::string& name);

	private:

	};

	class Scene
	{
	public:

		std::unique_ptr<SceneNode> root;

	private:

	};
}