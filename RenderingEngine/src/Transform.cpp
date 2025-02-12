#include "pch.h"

#include "Transform.h"

Engine::Transform::Transform()
{
	position = glm::vec3(1);
	rotation = glm::identity<glm::quat>();
	scale = glm::vec3(1);
}