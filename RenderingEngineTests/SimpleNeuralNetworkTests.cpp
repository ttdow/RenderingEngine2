#include "pch.h"

#include "CppUnitTest.h"

#include <src/SimpleNeuralNetwork.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace RenderingEngineTests
{
	TEST_CLASS(SimpleNeuralNetworkTests)
	{
	public:

		Engine::SimpleNeuralNetwork nn;

		TEST_METHOD(WeightsAreInitialized)
		{
			for (int i = 0; i < 2; i++)
			{
				for (int j = 0; j < 3; j++)
				{
					Assert::AreNotEqual(0.0f, nn.input_hidden1_weights[i][j]);
				}
			}

			for (int i = 0; i < 3; i++)
			{
				Assert::AreNotEqual(0.0f, nn.hidden1_bias[i]);
			}

			for (int i = 0; i < 3; i++)
			{
				for (int j = 0; j < 2; j++)
				{
					Assert::AreNotEqual(0.0f, nn.hidden1_hidden2_weights[i][j]);
				}
			}

			for (int i = 0; i < 2; i++)
			{
				Assert::AreNotEqual(0.0f, nn.hidden2_output_weights[i]);
				Assert::AreNotEqual(0.0f, nn.hidden2_bias[i]);
			}

			Assert::AreNotEqual(0.0f, nn.output_bias);
		}

		TEST_METHOD(SigmoidRange)
		{
			float result = nn.Predict({ 0.0f, 0.0f }); // Will run sigmoid function at the end.

			Assert::IsTrue(result > 0.0f);
			Assert::IsTrue(result < 1.0f);
		}

		/*
		TEST_METHOD(SigmoidIsCorrect)
		{
			float x = 0.0f;
			float expected = 0.5f;

			float e = std::numeric_limits<float>::epsilon();

			Assert::IsTrue(std::abs(expected - nn.Sigmoid(x)) < e);
		}
		*/

		TEST_METHOD(LearnsAND)
		{
			std::vector<Engine::TrainingExample> data =
			{
				{ {0.0f, 0.0f}, 0.0f },
				{ {0.0f, 1.0f}, 0.0f },
				{ {1.0f, 0.0f}, 0.0f },
				{ {1.0f, 1.0f}, 1.0f }
			};

			for (int epoch = 0; epoch < 1000000; epoch++)
			{
				for (const auto& ex : data)
				{
					nn.Train({ ex.input[0], ex.input[1] }, ex.target, 0.1f);
				}
			}

			Assert::IsTrue(nn.Predict({ 0.0f, 0.0f }) < 0.5f);
			Assert::IsTrue(nn.Predict({ 0.0f, 1.0f }) < 0.5f);
			Assert::IsTrue(nn.Predict({ 1.0f, 0.0f }) < 0.5f);
			Assert::IsTrue(nn.Predict({ 1.0f, 1.0f }) > 0.5f);
		}
	};
}