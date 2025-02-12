#include "pch.h"

#include "Scene.h"

Engine::SceneNode::SceneNode(const std::string& name)
{
	this->name = name;
	this->parent = nullptr;
}