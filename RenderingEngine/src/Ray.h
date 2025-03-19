#pragma once

#include "Vector3.h"

namespace Engine
{
	class Ray
	{
	public:

		Vector3 origin;
		Vector3 direction;

		Ray() {}
		Ray(const Vector3& origin, const Vector3& direction) : origin(origin), direction(direction) {}

		Vector3 At(double t) const
		{
			return origin + (t * direction);
		}
	};
}