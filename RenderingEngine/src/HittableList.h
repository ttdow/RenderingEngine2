#pragma once

#include "Hittable.h"
#include "Ray.h"
#include "HitRecord.h"
#include "Interval.h"

namespace Engine
{
	class HittableList : public Hittable
	{
	public:

		std::vector<std::shared_ptr<Hittable>> objects;

		HittableList() {}
		HittableList(std::shared_ptr<Hittable> object) { Add(object); }

		void Clear() { objects.clear(); }

		void Add(std::shared_ptr<Hittable> object)
		{
			objects.push_back(object);
		}

		bool Hit(const Ray& ray, Interval ray_t, HitRecord& record) const override
		{
			HitRecord tempRecord;
			bool hitAnything = false;

			auto closestSoFar = ray_t.max;

			for (const auto& object : objects)
			{
				if (object->Hit(ray, Interval(ray_t.min, closestSoFar), tempRecord))
				{
					hitAnything = true;
					closestSoFar = tempRecord.t;
					record = tempRecord;
				}
			}

			return hitAnything;
		}
	};
}