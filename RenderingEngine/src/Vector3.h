#pragma once

#include "pch.h"

namespace Engine
{
	class Vector3
	{
	public:

		double e[3];

		Vector3();
		Vector3(double e0, double e1, double e2);
		Vector3(const Vector3& other);
		Vector3& operator=(const Vector3& other);
		Vector3(Vector3&& other) noexcept;
		Vector3& operator=(Vector3&& other) noexcept;

		double x() const;
		double y() const;
		double z() const;

		Vector3 operator-() const;
		double operator[](int i) const;
		double& operator[](int i);
		Vector3& operator+=(const Vector3& v);
		Vector3& operator*=(double t);
		Vector3& operator/=(double t);

		double Length2() const;
		double Length() const;
		Vector3& Normalize();

		static double Dot(const Vector3& u, const Vector3& v);
		static Vector3 Cross(const Vector3& u, const Vector3& v);
		static Vector3 Normalize(const Vector3& v);
		static Vector3 Lerp(const Vector3& u, const Vector3& v, double t);
	};

	// Binary operators.
	inline std::ostream& operator<<(std::ostream& out, const Vector3& v)
	{
		return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2];
	}

	inline Vector3 operator+(const Vector3& u, const Vector3& v)
	{
		return Vector3(u.e[0] + v.e[0], u.e[1] + v.e[1], u.e[2] + v.e[2]);
	}

	inline Vector3 operator+(double t, const Vector3& v)
	{
		return Vector3(v.e[0] + t, v.e[1] + t, v.e[2] + t);
	}

	inline Vector3 operator+(const Vector3& v, double t)
	{
		return t + v;
	}

	inline Vector3 operator-(const Vector3& u, const Vector3& v)
	{
		return Vector3(u.e[0] - v.e[0], u.e[1] - v.e[1], u.e[2] - v.e[2]);
	}

	inline Vector3 operator-(const Vector3& v, double t)
	{
		return Vector3(v.e[0] - t, v.e[1] - t, v.e[2] - t);
	}

	inline Vector3 operator*(const Vector3& u, const Vector3& v)
	{
		return Vector3(u.e[0] * v.e[0], u.e[1] * v.e[1], u.e[2] * v.e[2]);
	}

	inline Vector3 operator*(double t, const Vector3& v)
	{
		return Vector3(v.e[0] * t, v.e[1] * t, v.e[2] * t);
	}

	inline Vector3 operator*(const Vector3& v, double t)
	{
		return t * v;
	}

	inline Vector3 operator/(const Vector3& v, double t)
	{
		return (1 / t) * v;
	}
}