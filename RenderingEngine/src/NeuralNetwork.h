#pragma once

#include "pch.h"

namespace Engine
{
	class NeuralNetwork
	{
	public:

		NeuralNetwork(const std::vector<int>& layerSizes) : layers(layerSizes)
		{
			if (layers.size() < 2)
			{
				throw std::runtime_error("A neural network must have at least 2 layers (input, output).");
			}

			// Initilize random number generator.
			std::mt19937 rng(std::random_device{}());

			for (size_t i = 0; i < layers.size() - 1; i++)
			{
				const int in = layers[i];
				const int out = layers[i + 1];

				const float xavier = std::sqrt(6.0f / (in + out));

				std::uniform_real_distribution<float> dist(-xavier, +xavier);

				// Weight matrix: out x in
				weights.push_back(std::vector<std::vector<float>>(out, std::vector<float>(in)));
				biases.push_back(std::vector<float>(out));

				for (int j = 0; j < out; j++)
				{
					for (int k = 0; k < in; k++)
					{
						weights.back()[j][k] = dist(rng);
					}

					biases.back()[j] = dist(rng);
				}
			}
		}

		void Train(const std::vector<float>& input, const std::vector<float>& target, float lr)
		{
			std::vector<std::vector<float>> activations;
			std::vector<std::vector<float>> zs;

			// Forward pass.
			activations.push_back(input);
			std::vector<float> activation = input;

			for (size_t l = 0; l < weights.size(); l++)
			{
				std::vector<float> z(layers[l + 1]);
				std::vector<float> a(layers[l + 1]);

				for (int j = 0; j < layers[l + 1]; j++)
				{
					z[j] = biases[l][j];
					for (int i = 0; i < layers[l]; i++)
					{
						z[j] += activation[i] * weights[l][j][i];
					}

					a[j] = (l == weights.size() - 1) ? Linear(z[j]) : ReLU(z[j]);
				}

				zs.push_back(z);
				activations.push_back(a);
				activation = a;
			}

			// Backward pass.
			std::vector<std::vector<float>> deltas(weights.size());

			// Output error.
			int L = weights.size() - 1;
			deltas[L] = std::vector<float>(layers[L + 1]);
			for (int i = 0; i < layers[L + 1]; i++)
			{
				float error = target[i] - activations[L + 1][i];
				deltas[L][i] = error * LinearDerivative(activations[L + 1][i]);
			}

			// Hidden layers.
			for (int l = L - 1; l >= 0; l--)
			{
				deltas[l] = std::vector<float>(layers[l + 1]);
				for (int i = 0; i < layers[l + 1]; i++)
				{
					float sum = 0.0f;
					for (int j = 0; j < layers[l + 2]; j++)
					{
						sum += deltas[l + 1][j] * weights[l + 1][j][i];
					}

					deltas[l][i] = sum * ReLUDerivative(activations[l + 1][i]);
				}
			}

			// Update weights and biases.
			for (size_t l = 0; l < weights.size(); l++)
			{
				for (int j = 0; j < layers[l + 1]; j++)
				{
					for (int i = 0; i < layers[l]; i++)
					{
						weights[l][j][i] += lr * deltas[l][j] * activations[l][i];
					}

					biases[l][j] += lr * deltas[l][j];
				}
			}
		}

		std::vector<float> Predict(const std::vector<float>& input)
		{
			std::vector<float> activation = input;

			for (size_t l = 0; l < weights.size(); l++)
			{
				std::vector<float> next(layers[l + 1]);

				for (int j = 0; j < layers[l + 1]; j++)
				{
					float z = biases[l][j];
					for (int i = 0; i < layers[l]; i++)
					{
						z += activation[i] * weights[l][j][i];
					}

					next[j] = (l == weights.size() - 1) ? Linear(z) : ReLU(z);
				}

				activation = next;
			}

			return activation;
		}

	private:

		std::vector<int> layers;
		std::vector<std::vector<std::vector<float>>> weights;
		std::vector<std::vector<float>> biases;

		float Linear(float x)
		{
			return x;
		}

		float LinearDerivative(float x)
		{
			return 1.0f;
		}

		float Sigmoid(float x)
		{
			return 1.0f / (1.0f + std::exp(-x));
		}

		float SigmoidDerivative(float x)
		{
			return x * (1.0f - x);
		}

		float Tanh(float x)
		{
			return std::tanh(x);
		}

		float TanhDerivative(float x)
		{
			return 1.0f - (x * x);
		}

		float ReLU(float x)
		{
			return x > 0.0f ? x : 0.0f;
		}

		float ReLUDerivative(float x)
		{
			return x > 0.0f ? 1.0f : 0.0f;
		}

		std::vector<float> Softmax(const std::vector<float>& z)
		{
			std::vector<float> result(z.size());
			float maxVal = *std::max_element(z.begin(), z.end());

			float sum = 0.0f;
			for (float val : z)
			{
				sum += std::exp(val - maxVal);
			}

			for (size_t i = 0; i < z.size(); i++)
			{
				result[i] = std::exp(z[i] - maxVal) / sum;
			}

			return result;
		}
	};
}