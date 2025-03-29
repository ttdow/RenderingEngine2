#include "pch.h"

#include "CppUnitTest.h"
#include <src/Vector3.h>
#include <src/Ray.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Microsoft::VisualStudio::CppUnitTestFramework
{
	template<>
	std::wstring ToString<Engine::Vector3>(const Engine::Vector3& v)
	{
		std::wostringstream ss;
		ss << L"Vector3(" << v.x << L", " << v.y << L", " << v.z << L")";
		return ss.str();
	}
}

namespace RenderingEngineTests
{
	using namespace Engine;

	TEST_CLASS(RenderingEngineTests)
	{
	public:
		
		TEST_METHOD(TestMethod1)
		{
			Vector3 origin(0, 0, 0);
			Vector3 direction(0, 0, 1);

			Ray ray(origin, direction);

			Assert::AreEqual(ray.origin, origin);
			Assert::AreEqual(ray.direction, direction);
		}
	};

	TEST_CLASS(Vector3Tests)
	{
	public:

		void AssertVectorNear(const Vector3& a, const Vector3& b)
		{
			Assert::IsTrue(std::abs(a.x - b.x) < std::numeric_limits<double>::epsilon());
			Assert::IsTrue(std::abs(a.y - b.y) < std::numeric_limits<double>::epsilon());
			Assert::IsTrue(std::abs(a.z - b.z) < std::numeric_limits<double>::epsilon());
		}

		TEST_METHOD(DefaultConstructor_ShouldInitToZero)
		{
			Vector3 v;

			Assert::AreEqual(0.0, v.x);
			Assert::AreEqual(0.0, v.y);
			Assert::AreEqual(0.0, v.z);
		}

		TEST_METHOD(ParameterConstructor_ShouldInitCorrectly)
		{
			Vector3 v(1.0, 2.0, 3.0);

			Assert::AreEqual(1.0, v.x);
			Assert::AreEqual(2.0, v.y);
			Assert::AreEqual(3.0, v.z);
		}

		TEST_METHOD(CopyConstructor_CopiesValues)
		{
			Vector3 original(1.0, 2.0, 3.0);
			Vector3 copy(original);

			Assert::AreEqual(original, copy);
		}

		TEST_METHOD(EqualityOperator_IdenticalVectors_ReturnsTrue)
		{
			Vector3 a(1.0, 2.0, 3.0);
			Vector3 b(1.0, 2.0, 3.0);

			Assert::IsTrue(a == b);
		}

		TEST_METHOD(EqualityOperator_DifferentVectors_ReturnsFalse)
		{
			Vector3 a(1.0, 2.0, 3.0);
			Vector3 b(1.0, 2.0, 4.0);

			Assert::IsFalse(a == b);
		}

		TEST_METHOD(SubscriptOperator_ReadAccess)
		{
			Vector3 v(1.1, 2.2, 3.3);

			Assert::AreEqual(1.1, v[0]);
			Assert::AreEqual(2.2, v[1]);
			Assert::AreEqual(3.3, v[2]);
		}

		TEST_METHOD(SubscriptOperator_WriteAccess)
		{
			Vector3 v;

			v[0] = 4.4;

			Assert::AreEqual(4.4, v[0]);
		}

		TEST_METHOD(OperatorPlusEquals_AddsValues)
		{
			Vector3 a(1.0, 2.0, 3.0);
			Vector3 b(4.0, 5.0, 6.0);

			a += b;

			Assert::AreEqual(Vector3(5.0, 7.0, 9.0), a);
		}

		TEST_METHOD(LengthSquared_Correct)
		{
			Vector3 v(3.0, 4.0, 0.0);
			
			Assert::AreEqual(25.0, v.Length2());
		}

		TEST_METHOD(Length_Correct)
		{
			Vector3 v(3.0, 4.0, 0.0);
		
			Assert::AreEqual(5.0, v.Length());
		}

		TEST_METHOD(Normalize_MakesLengthOnes)
		{
			Vector3 v(3.0, 0.0, 4.0);

			v.Normalize();
			double length = v.Length();

			Assert::IsTrue(std::abs(length - 1.0) < std::numeric_limits<double>::epsilon());
		}
	};
}