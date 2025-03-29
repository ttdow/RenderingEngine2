#pragma once

#include "Material.h"
#include "Ray.h"
#include "HitRecord.h"
#include "Vector3.h"

namespace Engine
{
	class Lambertian : public Material
	{
	public:

		Lambertian(const Vector3& albedo) : albedo(albedo) {}

		bool Scatter(const Ray& in, const HitRecord& record, Vector3& attenuation, Ray& scattered) const override
		{
			return true;
		}

	private:

		Vector3 albedo;

	};
}