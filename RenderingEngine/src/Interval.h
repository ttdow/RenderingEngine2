#pragma once

#include "pch.h"

namespace Engine
{
	class Interval
	{
	public:
		double min, max;

		//Interval() : min(1000), max(-1000) {}
		//Interval(double min, double max) : min(min), max(max) {}

		double Size() const
		{
			return max - min;
		}

		bool Contains(double x) const
		{
			return min <= x && x <= max;
		}

		bool Surrounds(double x) const
		{
			return min < x && x < max;
		}

		double Clamp(double x) const
		{
			if (x < min) return min;
			if (x > max) return max;

			return x;
		}

		//static const Interval empty, universe;
	};
}