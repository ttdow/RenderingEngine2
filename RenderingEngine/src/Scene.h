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

		SceneNode();
		SceneNode(const std::string& name);

	private:

	};

	class Scene
	{
	public:

		std::unique_ptr<SceneNode> root;

		Scene() = delete;						  // Default constructor
		Scene(const std::string& name);			  // Constructor
		~Scene();								  // Destructor
		Scene(const Scene& other);				  // Copy constructor
		Scene& operator=(const Scene& other);	  // Copy assignment operator
		Scene(Scene&& other) noexcept;			  // Move constructor
		Scene& operator=(Scene&& other) noexcept; // Move assignment operator

	private:

	};
}