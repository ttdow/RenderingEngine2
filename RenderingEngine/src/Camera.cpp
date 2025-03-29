#include "pch.h"

#include "Camera.h"

namespace Engine
{
	Camera::Camera(int imageWidth, double aspectRatio) : imageWidth(imageWidth), aspectRatio(aspectRatio) 
	{
		Initialize();
	}

	Ray Camera::GetRay(int x, int y)
	{
		Vector3 pixelCenter = pixel00Location + (x * pixelDelta_u) + (y * pixelDelta_v);
		Vector3 rayDirection = (pixelCenter - center);
		
		return Ray(center, rayDirection);
	}

	Ray Camera::GetJitteredRay(int x, int y)
	{
		Vector3 offset = Vector3(Random::Get()->RandomDouble() - 0.5, Random::Get()->RandomDouble() - 0.5, 0.0);

		Vector3 target = pixel00Location + 
			(double(x) + offset.x) * pixelDelta_u + 
			(double(y) + offset.y) * pixelDelta_v;

		return Ray(center, Vector3::Normalize(target - center));
	}

	void Camera::Initialize()
	{
		imageHeight = int(imageWidth / aspectRatio);
		imageHeight = (imageHeight < 1) ? 1 : imageHeight;

		center = Vector3(0.0, 0.0, 0.0);

		// Determine viewport dimensions.
		const double focalLength = 1.0;
		const double viewportHeight = 2.0;
		const double viewportWidth = viewportHeight * (double(imageWidth) / imageHeight);

		// Calculate the vectors across the horizontal and down the vertical viewport edges.
		const Vector3 viewport_u(viewportWidth, 0.0, 0.0);
		const Vector3 viewport_v(0.0, -viewportHeight, 0.0);

		// Calculate the horizontal and vertical delta vectors from pixel to pixel.
		pixelDelta_u = viewport_u / imageWidth;
		pixelDelta_v = viewport_v / imageHeight;

		// Calculate the location of the upper left pixel.
		const Vector3 viewportUpperLeft = center - Vector3(0.0, 0.0, focalLength) - (viewport_u / 2) - (viewport_v / 2);
		pixel00Location = viewportUpperLeft + 0.5 * (pixelDelta_u + pixelDelta_v);
	}
}