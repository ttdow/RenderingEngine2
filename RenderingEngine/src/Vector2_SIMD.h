#pragma once

#include <emmintrin.h>
#include <string>

namespace Engine
{
	class Vector2_SIMD
	{
	public:

		__m128d vec;

		// Constructors.
		Vector2_SIMD() : vec(_mm_setzero_pd()) {};
		Vector2_SIMD(double x, double y) : vec(_mm_set_pd(y, x)) {};
		explicit Vector2_SIMD(__m128d vec) : vec(vec) {};

		// Accessors.
		double x() const { return _mm_cvtsd_f64(vec); }
		double y() const { return _mm_cvtsd_f64(_mm_unpackhi_pd(vec, vec)); }

		void SetX(double x) { vec = _mm_move_sd(vec, _mm_set_sd(x)); }
		void SetY(double y) 
		{
			__m128d yVec = _mm_set_sd(y);
			vec = _mm_move_sd(_mm_unpacklo_pd(yVec, vec), vec);
		}

		inline Vector2_SIMD operator+(const Vector2_SIMD& other) const
		{
			return Vector2_SIMD(_mm_add_pd(vec, other.vec));
		}

		friend std::ostream& operator<<(std::ostream& os, const Vector2_SIMD& v)
		{
			os << "(" << v.x() << ", " << v.y() << ")";
			return os;
		}
	};
}