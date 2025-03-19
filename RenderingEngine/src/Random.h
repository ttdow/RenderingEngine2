#pragma once

#include "pch.h"
#include "Vector3.h"

namespace Engine
{
	class Random
	{
	private:

		static Random* instance;

		std::mt19937 generator;
		std::uniform_real_distribution<double> uniformDistribution;

		Random() : generator(std::chrono::system_clock::now().time_since_epoch().count()), uniformDistribution(0.0f, 1.0f) {}
		~Random() {}

	public:

		Random(const Random&) = delete;
		Random operator=(const Random&) = delete;

		static Random* Get()
		{
			if (instance == nullptr)
			{
				instance = new Random();
			}

			return instance;
		}

		double RandomDouble()
		{
			return uniformDistribution(generator);
		}

		double RandomDouble(double min, double max)
		{
			return min + (max - min) * RandomDouble();
		}

		Vector3 RandomVector3()
		{
			return Vector3(RandomDouble(), RandomDouble(), RandomDouble());
		}

		Vector3 RandomVector3(double min, double max)
		{
			return Vector3(RandomDouble(min, max), RandomDouble(min, max), RandomDouble(min, max));
		}

		Vector3 RandomUnitVector3()
		{
			while (true)
			{
				Vector3 p = RandomVector3(-1.0, 1.0);
				double lensq = p.Length2();
				if (1e-160 < lensq && lensq <= 1)
				{
					return p / std::sqrt(lensq);
				}
			}
		}

		Vector3 RandomOnUnitHemisphere(const Vector3& N)
		{
			Vector3 onUnitSphere = RandomUnitVector3();
			if (Vector3::Dot(onUnitSphere, N) > 0.0)
			{
				return onUnitSphere;
			}
			else
			{
				return -onUnitSphere;
			}
		}
	};
}