#pragma once

#include "Vector3.h"
#include "Ray.h"
#include "Hittable.h"

namespace Engine
{
	class Sphere : public Hittable
	{
	public:

		Vector3 center;
		double radius;

		Sphere(const Vector3& center, double radius);

		bool Hit(const Ray& ray, Interval ray_t, HitRecord& record) const override;
	};
}