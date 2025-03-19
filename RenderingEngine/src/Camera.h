#pragma once

#include "Vector3.h"
#include "Ray.h"
#include "Hittable.h"
#include "Random.h"

namespace Engine
{
	class Camera
	{
	public:

		Camera() = delete;
		Camera(int imageWidth, double aspectRatio);

		Ray GetRay(int x, int y);
		Ray GetJitteredRay(int x, int y);

	private:

		double aspectRatio;
		int imageWidth, imageHeight;
		Vector3 center;
		Vector3 pixel00Location;
		Vector3 pixelDelta_u;
		Vector3 pixelDelta_v;

		void Initialize();
	};
}