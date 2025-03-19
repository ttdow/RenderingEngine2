#include "pch.h"

#include "Scene.h"

Engine::SceneNode::SceneNode()
{
	this->name = "";
	this->parent = nullptr;
}

Engine::SceneNode::SceneNode(const std::string& name)
{
	this->name = name;
	this->parent = nullptr;
}

Engine::Scene::Scene(const std::string& name)
{
	root = std::make_unique<Engine::SceneNode>(name);

	std::cout << "Scene constructor called.\n";
}

Engine::Scene::~Scene()
{
	// Nothing
	std::cout << "Scene destructor called.\n";
}

Engine::Scene::Scene(const Engine::Scene& other)
{
	// Nothing
	std::cout << "Scene copy constructor called.\n";
}

Engine::Scene& Engine::Scene::operator=(const Scene& other)
{
	// Nothing
	std::cout << "Scene assignment operator called.\n";
	return *this;
}

Engine::Scene::Scene(Engine::Scene&& other) noexcept
{
	// Nothing
	std::cout << "Scene move constructor called.\n";
}

Engine::Scene& Engine::Scene::operator=(Engine::Scene&& other) noexcept
{
	// Nothing
	std::cout << "Scene move assignment operator called.\n";
	return *this;
}