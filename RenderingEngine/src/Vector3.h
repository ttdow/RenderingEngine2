#pragma once

#include "pch.h"

namespace Engine
{
	/**
	 * @class Vector3
	 * @brief A simple 3D vector class for representing and manipulating 3D vectors.
	 * 
	 * Supports common vector operations such as addition, scaling, normalization, 
	 * dot and cross products, and interpolation. Uses a union for both named 
	 * access ('x', 'y', 'z') and array-style access via 'data[3]'.
	 */
	class Vector3
	{
	public:

		/// Union to allow both named and indexed access to components.
		union
		{
			struct
			{
				double x; ///< X component
				double y; ///< Y component
				double z; ///< Z component
			};

			double data[3]; ///< Array-style access to components
		};

		/** @brief Default constructor. Initializes all components to zero. */
		constexpr Vector3() : x(0.0), y(0.0), z(0.0) {};

		/**
		 * @brief Parameterized constructor.
		 * @param x The x-component of the vector.
		 * @param y The y-component of the vector.
		 * @param z The z-component of the vector.
		 */
		constexpr Vector3(double x, double y, double z) : x(x), y(y), z(z) {};

		/**
		 * @brief Copy constructor.
		 * @param other The vector to copy.
		 */
		constexpr Vector3(const Vector3& other) : x(other.x), y(other.y), z(other.z) {};

		/**
		 * @brief Assignment operator.
		 * @param other The vector to copy.
		 * @return A reference to the copied vector.
		 */
		Vector3& operator=(const Vector3& other);

		/**
		 * @brief Move constructor.
		 * @param other The vector to move.
		 */
		Vector3(Vector3&& other) noexcept;

		/**
		 * @brief Move assignment operator.
		 * @param other The vector to move.
		 * @return A reference to the moved vector.
		 */
		Vector3& operator=(Vector3&& other) noexcept;

		/**
		 * @brief Returns a vector with all components set to zero.
		 * @return The zero vector.
		 */
		static constexpr Vector3 Zero() { return Vector3(0.0, 0.0, 0.0); }

		/**
		 * @brief Returns a vector with the y-component set to 1.0.
		 * @return The up vector.
		 */
		static constexpr Vector3 Up() { return Vector3(0.0, 1.0, 0.0); }

		/**
		 * @brief Returns a vector with the x-component set to 1.0.
		 * @return The right vector.
		 */
		static constexpr Vector3 Right() { return Vector3(1.0, 0.0, 0.0); }

		/**
		 * @brief Returns a vector with the z-component set to 1.0.
		 * @return The forward vector.
		 */
		static constexpr Vector3 Forward() { return Vector3(0.0, 0.0, 1.0); }

		/** @brief Unary minus operator. Returns the negation of the vector. */
		constexpr Vector3 operator-() const { return Vector3(-x, -y, -z); }

		/** 
		 * @brief Unary plus operator. Returns a copy of the vector.
		 * 
		 * This is a no-op that returns the same vector unchanged.
		 * Useful for consistency in generic code or expressions involving both unary plus and minus.
		 * 
		 * @return A copy of the vector.
		 */
		constexpr Vector3 operator+() const { return *this; };

		/**
		 * @brief Read-only access to a component by index. 
		 * @param i Index (0 = x, 1 = y, 2 = z).
		 * @return Value of the component.
		 */
		double operator[](int i) const;

		/**
		 * @brief Mutable access to a component by index.
		 * @param i Index (0 = x, 1 = y, 2 = z).
		 * @return Reference to the component.
		 */
		double& operator[](int i);

		/**
		 * @brief Adds another vector to this vector.
		 * @param v Vector to add.
		 * @return Reference to this vector.
		 */
		Vector3& operator+=(const Vector3& v);

		/**
		 * @brief Multiplies this vector by a scalar.
		 * @param t Scalar multiplier.
		 * @return Reference to this vector.
		 */
		Vector3& operator*=(double t);

		/**
		 * @brief Divides this vector by a scalar.
		 * @param t Scalar divisor.
		 * @return Reference to this vector.
		 */
		Vector3& operator/=(double t);

		/**
		 * @brief Checks for equality with another vector.
		 * @param v Vector to compare.
		 * @return True if all components are equal, false otherwise.
		 */
		bool operator==(const Vector3& v) const;

		/** @brief Returns the squared length (magnitude) of the vector. */
		double Length2() const;

		/** @brief Returns the length (magnitude) of the vector. */
		double Length() const;

		/**
		 * @brief Normalizes this vector in place.
		 * @return Reference to this vector.
		 */
		Vector3& Normalize();

		/**
		 * @brief Computes the dot product of two vectors.
		 * @param u First vector.
		 * @param v Second vector.
		 * @return The dot product.
		 */
		static double Dot(const Vector3& u, const Vector3& v);

		/**
		 * @brief Computes the cross product of two vectors.
		 * @param u First vector.
		 * @param v Second vector.
		 * @return The resulting vector.
		 */
		static Vector3 Cross(const Vector3& u, const Vector3& v);

		/**
		 * @brief Returns a normalized copy of the input vector.
		 * @param v Vector to normalize.
		 * @return The normalized vector.
		 */
		static Vector3 Normalize(const Vector3& v);

		/**
		 * @brief Linearly interpolates between two vectors.
		 * @param u Start vector.
		 * @param v End vector.
		 * @param t Interpolation factor (0.0 = u, 1.0 = v).
		 * @return The interpolated vector.
		 */
		static Vector3 Lerp(const Vector3& u, const Vector3& v, double t);

		/**
		 * @brief Clamps the components of this vector to a specified range.
		 * @param min The minimum value.
		 * @param max The maximum value.
		 * @return Reference to this vector.
		 */
		Vector3& Clamp(double min, double max);

		/**
		 * @brief Clamps the components of a vector to a specified range.
		 * @param v The vector to clamp.
		 * @param min The minimum value.
		 * @param max The maximum value.
		 * @return The clamped vector.
		 */
		static Vector3 Clamp(const Vector3& v, double min, double max);

		/**
		 * @brief Converts the vector to a string representation.
		 * @param precision The number of decimal places to include.
		 * @return The string representation of the vector.
		 */
		std::string ToString(int precision = 3) const;
	};

	// *************************
	// Binary operators.
	// *************************

	/**
	 * @brief Streams a vector to an output stream.
	 * @param out The output stream.
	 * @param v The vector to print.
	 * @return Reference to the output stream.
	 */
	inline std::ostream& operator<<(std::ostream& out, const Vector3& v)
	{
		return out << v.x << ' ' << v.y << ' ' << v.z;
	}

	/**
	 * @brief Adds two vectors.
	 * @param u First vector.
	 * @param v Second vector.
	 * @return Resulting vector.
	 */
	inline Vector3 operator+(const Vector3& u, const Vector3& v)
	{
		return Vector3(u.x + v.x, u.y + v.y, u.z + v.z);
	}

	/**
	 * @brief Adds a scalar to all components of the vector.
	 * @param t Scalar to add.
	 * @param v Vector.
	 * @return Resulting vector.
	 */
	inline Vector3 operator+(double t, const Vector3& v)
	{
		return Vector3(v.x + t, v.y + t, v.z + t);
	}

	/** @copydoc operator+(double, const Vector3&) */
	inline Vector3 operator+(const Vector3& v, double t)
	{
		return t + v;
	}

	/**
	 * @brief Subtracts one vector from another.
	 * @param u First vector.
	 * @param v Vector to subtract.
	 * @return Resulting vector.
	 */
	inline Vector3 operator-(const Vector3& u, const Vector3& v)
	{
		return Vector3(u.x - v.x, u.y - v.y, u.z - v.z);
	}

	/**
	 * @brief Subtracts a scalar from all components of the vector.
	 * @param v Vector.
	 * @param t Scalar to subtract.
	 * @return Resulting vector.
	 */
	inline Vector3 operator-(const Vector3& v, double t)
	{
		return Vector3(v.x - t, v.y - t, v.z - t);
	}

	/**
	 * @brief Component-wise multiplication of two vectors.
	 * @param u First vector.
	 * @param v Second vector.
	 * @return Resulting vector.
	 */
	inline Vector3 operator*(const Vector3& u, const Vector3& v)
	{
		return Vector3(u.x * v.x, u.y * v.y, u.z * v.z);
	}

	/**
	 * @brief Multiplies all components of a vector by a scalar.
	 * @param t Scalar multiplier.
	 * @param v Vector.
	 * @return Resulting vector.
	 */
	inline Vector3 operator*(double t, const Vector3& v)
	{
		return Vector3(v.x * t, v.y * t, v.z * t);
	}

	/** @copydoc operator*(double, const Vector3&) */
	inline Vector3 operator*(const Vector3& v, double t)
	{
		return t * v;
	}

	/**
	 * @brief Divides all components of a vector by a scalar.
	 * @param v Vector.
	 * @param t Scalar divisor.
	 * @return Resulting vector.
	 */
	inline Vector3 operator/(const Vector3& v, double t)
	{
		return (1 / t) * v;
	}
}