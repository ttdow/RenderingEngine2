#pragma once

#include "Vector3.h"
#include "Ray.h"
#include "HitRecord.h"
#include "Interval.h"

namespace Engine
{
	class Hittable
	{
	public:

		virtual ~Hittable() = default;
		virtual bool Hit(const Ray& ray, Interval ray_t, HitRecord& record) const = 0;
	};
}