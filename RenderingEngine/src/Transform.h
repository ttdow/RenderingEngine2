#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

namespace Engine
{
	class Transform
	{
	public:

		glm::vec3 position;
		glm::quat rotation;
		glm::vec3 scale;

		Transform();

	private:

	};
}