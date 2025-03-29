#pragma once

#include <string>

namespace Engine
{
	class Vector2
	{
	public:

		union
		{
			struct
			{
				double x;
				double y;
			};

			double data[2];
		};

		// Constructors.
		constexpr Vector2() : x(0.0), y(0.0) {};
		constexpr Vector2(double x, double y) : x(x), y(y) {};
		constexpr Vector2(const Vector2& other) : x(other.x), y(other.y) {};
		//Vector2& operator=(const Vector2& other);
		//Vector2(Vector2&& other) noexcept;
		//Vector2& operator=(Vector2&& other) noexcept;

		//double operator[](int i) const;
		//double& operator[](int i);

		constexpr Vector2 operator-() const { return Vector2(-x, -y); }
		constexpr Vector2 operator+() const { return *this; }

		Vector2 operator+(const Vector2& other) const
		{
			return Vector2(x + other.x, y + other.y);
		}
	};

	inline std::ostream& operator<<(std::ostream& os, const Vector2& v)
	{
		os << "(" << v.x << ", " << v.y << ")";
		return os;
	}
}