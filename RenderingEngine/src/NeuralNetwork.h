#pragma once

#include "pch.h"

#include "LossFunction.h"

namespace Engine
{
	/**
	 * @class NeuralNetwork
	 * @brief A simple feedforward neural network with backpropagation training.
	 */
	class NeuralNetwork
	{
	public:

		/**
		 * @brief Constructs a NeuralNetwork with the given layer sizes.
		 * 
		 * @param layerSizes A vector of integers representing the number of neurons in each layer.
		 * @throws std::runtime_error if fewer than 2 layers are specified.
		 */
		NeuralNetwork(const std::vector<int>& layerSizes, LossFunction* loss) : layers(layerSizes), lossFunction(loss)
		{
			if (layers.size() < 2)
			{
				throw std::runtime_error("A neural network must have at least 2 layers (input, output).");
			}

			// Initilize random number generator for weights and biases.
			std::mt19937 rng(std::random_device{}());

			// Initialize the weights and biases for each layer (except the input layer).
			for (size_t i = 0; i < layers.size() - 1; i++)
			{
				const int in = layers[i];		// Number of neurons in the current layer.
				const int out = layers[i + 1];	// Number of neurons in the next layer.

				// Xavier initialization scaling factor to improve convergence.
				const float xavier = std::sqrt(6.0f / (in + out));
				std::uniform_real_distribution<float> dist(-xavier, +xavier);

				// Create a weight matrix of shape [out][in].
				weights.push_back(std::vector<std::vector<float>>(out, std::vector<float>(in)));
				biases.push_back(std::vector<float>(out));

				// Randomly initialize each weight and bias.
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

		/**
		 * @brief Trains the neural network on a single example using stochastic gradient descent.
		 * @param input The input vector.
		 * @param target The target output vector.
		 * @param lr The learning rate.
		 */
		void Train(const std::vector<float>& input, const std::vector<float>& target, float lr)
		{
			std::vector<std::vector<float>> activations; // Stores activations of each layer.
			std::vector<std::vector<float>> zs;			 // Stores weighted inputs (z-values) of each layer.

			// Forward pass.
			activations.push_back(input); // Input layer activation.
			std::vector<float> activation = input; 

			for (size_t l = 0; l < weights.size(); l++)
			{
				std::vector<float> z(layers[l + 1]); // Pre-activation values.
				std::vector<float> a(layers[l + 1]); // Post-activation values.

				for (int j = 0; j < layers[l + 1]; j++)
				{
					z[j] = biases[l][j]; // Start with bias.
					for (int i = 0; i < layers[l]; i++)
					{
						z[j] += activation[i] * weights[l][j][i]; // Weighted sum.
					}

					// Use ReLU for hidden layers, linear for output layer.
					a[j] = (l == weights.size() - 1) ? Linear(z[j]) : ReLU(z[j]);
				}

				zs.push_back(z);
				activations.push_back(a);
				activation = a; // Carry forward to the next layer.
			}

			// Backward pass.
			std::vector<std::vector<float>> deltas(weights.size());

			// Compute output layer error.
			int L = weights.size() - 1;
			deltas[L] = std::vector<float>(layers[L + 1]);

			std::vector<float> errors = lossFunction->Derivative(target, activations[L + 1]);
			for (int i = 0; i < errors.size(); i++)
			{
				deltas[L][i] = errors[i] * LinearDerivative(activations[L + 1][i]);
			}
			
			/*
			for (int i = 0; i < layers[L + 1]; i++)
			{
				// 0.5 * (y - a) ^ 2 -> (y - a) (derivative of MSE loss function)
				float error = target[i] - activations[L + 1][i]; // Difference from target.
				deltas[L][i] = error * LinearDerivative(activations[L + 1][i]); // Derivative of Loss.
			}
			*/

			// Propagate error backwards through hidden layers.
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

					deltas[l][i] = sum * ReLUDerivative(activations[l + 1][i]); // Gradient through ReLU.
				}
			}

			// Update weights and biases using gradients.
			for (size_t l = 0; l < weights.size(); l++)
			{
				for (int j = 0; j < layers[l + 1]; j++)
				{
					for (int i = 0; i < layers[l]; i++)
					{
						weights[l][j][i] += lr * deltas[l][j] * activations[l][i]; // Gradient descent step.
					}

					biases[l][j] += lr * deltas[l][j]; // Update bias.
				}
			}
		}

		/**
		 * @brief Predicts the output of the network given an input.
		 * @param input The input vector.
		 * @return std::vector<float> The output vector after forward propagation.
		 */
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
						z += activation[i] * weights[l][j][i]; // WEighted sum.
					}

					next[j] = (l == weights.size() - 1) ? Linear(z) : ReLU(z); // Activation.
				}

				activation = next; // Move to next layer.
			}

			return activation;
		}

	private:

		std::vector<int> layers; ///< Layer sizes of the neural network.
		std::vector<std::vector<std::vector<float>>> weights; ///< Weight matrices between layers.
		std::vector<std::vector<float>> biases; ///< Bias vectors for each layer.

		LossFunction* lossFunction;

		/**
		 * @brief Identity activation function.
		 * @param x Input value.
		 * @return float Output value.
		 */
		float Linear(float x)
		{
			return x;
		}

		/**
		 * @brief Derivative of the identity function.
		 * @param x Input value.
		 * @return float Derivative value.
		 */
		float LinearDerivative(float x)
		{
			return 1.0f;
		}

		/**
		 * @brief Sigmoid activation function.
		 * @param x Input value.
		 * @return float Output value.
		 */
		float Sigmoid(float x)
		{
			return 1.0f / (1.0f + std::exp(-x));
		}

		/**
		 * @brief Derivative of the sigmoid function.
		 * @param x Output value from sigmoid.
		 * @return float Derivative value.
		 */
		float SigmoidDerivative(float x)
		{
			return x * (1.0f - x);
		}

		/**
		 * @brief Hyperbolic tangent activation function.
		 * @param x Input value.
		 * @return float Output value.
		 */
		float Tanh(float x)
		{
			return std::tanh(x);
		}

		/**
		 * @brief Derivative of te hyperbolic tangent function.
		 * @param x Output value from tanh.
		 * @return float Derivative value.
		 */
		float TanhDerivative(float x)
		{
			return 1.0f - (x * x);
		}

		/**
		 * @brief Rectified Linear Unit (ReLU) activation function.
		 * @param x Input value.
		 * @return float Output value.
		 */
		float ReLU(float x)
		{
			return x > 0.0f ? x : 0.0f;
		}

		/**
		 * @brief Derivative of the ReLU function.
		 * @param x Input value.
		 * @return float Derivative value.
		 */
		float ReLUDerivative(float x)
		{
			return x > 0.0f ? 1.0f : 0.0f;
		}

		/**
		 * @brief Softmax activation function.
		 * @param z Input vector.
		 * @return std::vector<float> Softmax-normalized vector.
		 */
		std::vector<float> Softmax(const std::vector<float>& z)
		{
			std::vector<float> result(z.size());
			float maxVal = *std::max_element(z.begin(), z.end()); // For numerical stability.

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