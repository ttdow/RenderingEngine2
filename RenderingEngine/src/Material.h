#pragma once

#include "Ray.h"
#include "HitRecord.h"

namespace Engine
{
	class Material
	{
	public:

		virtual ~Material() = default;

		virtual bool Scatter(const Ray& in, const HitRecord& record, Vector3& attenuation, Ray& scattered) const
		{
			return false;
		}
	};
}