#pragma once

#include "Vector3.h"
#include "Ray.h"

namespace Engine
{
	class HitRecord
	{
	public:

		Vector3 p;
		Vector3 N;
		double t;
		bool frontFace;

		void SetFaceNormal(const Ray& ray, const Vector3& outwardNormal)
		{
			// Sets the hit record normal.
			// Note: the parameter 'outwardNormal' is assumed to have unit length.

			frontFace = Vector3::Dot(ray.direction, outwardNormal) < 0;
			N = frontFace ? outwardNormal : -outwardNormal;
		}
	};
}