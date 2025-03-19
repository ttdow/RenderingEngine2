#include "pch.h"

#include "Vector3.h"

namespace Engine
{
	// Default constructor.
	Vector3::Vector3() : e{ 0.0, 0.0, 0.0 } {}

	Vector3::Vector3(double e0, double e1, double e2) : e{ e0, e1, e2 } {}

	// Copy constructor.
	Vector3::Vector3(const Vector3& other) : e{ other.e[0], other.e[1], other.e[2] } {}
	
	// Assignment operator.
	Vector3& Vector3::operator=(const Vector3& other)
	{
		if (this == &other) return *this;

		e[0] = other.e[0];
		e[1] = other.e[1];
		e[2] = other.e[2];

		return *this;
	}

	// Move constructor.
	Vector3::Vector3(Vector3&& other) noexcept
	{
		e[0] = other.e[0];
		e[1] = other.e[1];
		e[2] = other.e[2];

		other.e[0] = 0.0;
		other.e[1] = 0.0;
		other.e[2] = 0.0;
	}

	// Move assignment operator.
	Vector3& Vector3::operator=(Vector3&& other) noexcept
	{
		if (this == &other) return *this;

		e[0] = other.e[0];
		e[1] = other.e[1];
		e[2] = other.e[2];

		other.e[0] = 0.0;
		other.e[1] = 0.0;
		other.e[2] = 0.0;

		return *this;
	}

	double Vector3::x() const
	{
		return e[0];
	}

	double Vector3::y() const
	{
		return e[1];
	}

	double Vector3::z() const
	{
		return e[2];
	}

	double Vector3::Length2() const
	{
		return e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
	}

	double Vector3::Length() const
	{
		return std::sqrt(Length2());
	}

	double Vector3::Dot(const Vector3& u, const Vector3& v)
	{
		return u.e[0] * v.e[0] + u.e[1] * v.e[1] + u.e[2] * v.e[2];
	}

	Vector3 Vector3::Cross(const Vector3& u, const Vector3& v)
	{
		return Vector3(u.e[1] * v.e[2] - u.e[2] * v.e[1],
			u.e[2] * v.e[0] - u.e[0] * v.e[2],
			u.e[0] * v.e[1] - u.e[1] * v.e[0]);
	}

	Vector3& Vector3::Normalize()
	{
		const double c = 1 / Length();

		e[0] *= c;
		e[1] *= c;
		e[2] *= c;

		return *this;
	}

	Vector3 Vector3::Normalize(const Vector3& v)
	{
		const double c = v.Length();

		return Vector3(v.e[0] / c, v.e[1] / c, v.e[2] / c);
	}

	Vector3 Vector3::Lerp(const Vector3& start, const Vector3& end, double t)
	{
		return ((1.0 - t) * start) + (t * end);
	}

	// Unary operators.
	Vector3 Vector3::operator-() const
	{
		return Vector3(-e[0], -e[1], -e[2]);
	}

	double Vector3::operator[](int i) const
	{
		return e[i];
	}

	double& Vector3::operator[](int i)
	{
		return e[i];
	}

	Vector3& Vector3::operator+=(const Vector3& v)
	{
		e[0] += v.e[0];
		e[1] += v.e[1];
		e[2] += v.e[2];

		return *this;
	}

	Vector3& Vector3::operator*=(double t)
	{
		e[0] *= t;
		e[1] *= t;
		e[2] *= t;
		
		return *this;
	}

	Vector3& Vector3::operator/=(double t)
	{
		return *this *= (1 / t);
	}
}