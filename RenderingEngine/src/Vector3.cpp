#include "pch.h"

#include "Vector3.h"

namespace Engine
{
	// Assignment operator.
	Vector3& Vector3::operator=(const Vector3& other)
	{
		if (this == &other) return *this;

		x = other.x;
		y = other.y;
		z = other.z;

		return *this;
	}

	// Move constructor.
	Vector3::Vector3(Vector3&& other) noexcept
	{
		x = other.x;
		y = other.y;
		z = other.z;

		other.x = 0.0;
		other.y = 0.0;
		other.z = 0.0;
	}

	// Move assignment operator.
	Vector3& Vector3::operator=(Vector3&& other) noexcept
	{
		if (this == &other) return *this;

		x = other.x;
		y = other.y;
		z = other.z;

		other.x = 0.0;
		other.y = 0.0;
		other.z = 0.0;

		return *this;
	}

	double Vector3::Length2() const
	{
		return x * x + y * y + z * z;
	}

	double Vector3::Length() const
	{
		return std::sqrt(Length2());
	}

	double Vector3::Dot(const Vector3& u, const Vector3& v)
	{
		return u.x * v.x + u.y * v.y + u.z * v.z;
	}

	Vector3 Vector3::Cross(const Vector3& u, const Vector3& v)
	{
		return Vector3(u.y * v.z - u.z * v.y,
			u.z * v.x - u.x * v.z,
			u.x * v.y - u.y * v.x);
	}

	Vector3& Vector3::Normalize()
	{
		const double c = 1 / Length();

		x *= c;
		y *= c;
		z *= c;

		return *this;
	}

	Vector3 Vector3::Normalize(const Vector3& v)
	{
		const double c = v.Length();

		return Vector3(v.x / c, v.y / c, v.z / c);
	}

	Vector3 Vector3::Lerp(const Vector3& start, const Vector3& end, double t)
	{
		return ((1.0 - t) * start) + (t * end);
	}

	Vector3& Vector3::Clamp(double min, double max)
	{
		x = std::clamp(x, min, max);
		y = std::clamp(y, min, max);
		z = std::clamp(z, min, max);

		return *this;
	}

	Vector3 Vector3::Clamp(const Vector3& v, double min, double max)
	{
		return Vector3(std::clamp(v.x, min, max),
			std::clamp(v.y, min, max),
			std::clamp(v.z, min, max));
	}

	std::string Vector3::ToString(int precision) const
	{
		std::ostringstream oss;
		oss << std::fixed << std::setprecision(precision) << x << ' ' << y << ' ' << z;
		return oss.str();
	}

	// Unary operators.

	double Vector3::operator[](int i) const
	{
		return data[i];
	}

	double& Vector3::operator[](int i)
	{
		return data[i];
	}

	Vector3& Vector3::operator+=(const Vector3& v)
	{
		x += v.x;
		y += v.y;
		z += v.z;

		return *this;
	}

	Vector3& Vector3::operator*=(double t)
	{
		x *= t;
		y *= t;
		z *= t;
		
		return *this;
	}

	Vector3& Vector3::operator/=(double t)
	{
		return *this *= (1 / t);
	}

	bool Vector3::operator==(const Vector3& v) const
	{
		return x == v.x && y == v.y && z == v.z;
	}
}