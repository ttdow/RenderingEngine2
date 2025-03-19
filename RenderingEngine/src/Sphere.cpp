#include "pch.h"

#include "Sphere.h"

namespace Engine
{
	Sphere::Sphere(const Vector3& center, double radius)
	{
		this->center = center;
		this->radius = std::fmax(0.0f, radius);
	}

	bool Sphere::Hit(const Ray& ray, Interval ray_t, HitRecord& record) const
	{
		const Vector3 oc = center - ray.origin;
		const double a = ray.direction.Length2();
		const double h = Vector3::Dot(ray.direction, oc);
		const double c = oc.Length2() - radius * radius;

		const double d = h * h - a * c;
		if (d < 0)
		{
			return false;
		}

		const double sqrtd = std::sqrt(d);

		// Find the nearest root that lies in the acceptable range.
		double root = (h - sqrtd) / a;
		if (!ray_t.Surrounds(root))
		{
			root = (h + sqrtd) / a;
			if (!ray_t.Surrounds(root))
			{
				return false;
			}
		}

		record.t = root;
		record.p = ray.At(record.t);
		Vector3 outwardNormal = (record.p - center) / radius;
		record.SetFaceNormal(ray, outwardNormal);

		return true;
	}
}