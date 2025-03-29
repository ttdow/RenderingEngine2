#pragma once

#include "pch.h"

namespace Engine
{
	/**
	 * @brief Represents a single training example for a neural network.
	 */
	struct TrainingExample
	{
		float input[2]; ///< Two-dimensional input vector.
		float target;   ///< Target output value.
	};

	/**
	 * @brief Simple feed-forward neural network class.
	 * - 2 input neurons.
	 * - 1 hidden layer with 3 neurons.
	 * - 1 hidden layer with 2 neurons.
	 * - 1 output neuron.
	 */
	class SimpleNeuralNetwork
	{
	public:

		/**
		 * @brief Weights connecting input layer (2 neurons) to first hidden layer (3 neurons).
		 * 'input_hidden1_weights[i][j]' is the weight from input neuron 'i' to hidden neuron 'j'.
		 */
		float input_hidden1_weights[2][3];

		/**
		 * @brief Bias values added to each neuron in the first hidden layer.
		 */
		float hidden1_bias[3];

		/**
		 * @brief Weights connecting first hidden layer (3 neurons) to second hidden layer (2 neurons).
		 * @'hidden1_hidden2_weights[i][j]' is the weight from hidden1 neuron 'i' to hidden2 neuron 'j'.
		 */
		float hidden1_hidden2_weights[3][2];

		/**
		 * @brief Bias values added to each neuron in the second hidden layer.
		 */
		float hidden2_bias[2];

		/**
		 * @brief Weights connecting second hidden layer (2 neurons) to output layer (1 neuron).
		 */
		float hidden2_output_weights[2];

		/**
		 * @brief Bias added to the output neuron.
		 */
		float output_bias;

		/**
		 * @brief Constructs and initializes the neural network.
		 * 
		 * Seeds the random number generator and initializes all weights and biases with small
		 * random values using Xavier initialization.
		 */
		SimpleNeuralNetwork()
		{
			// Random seed.
			std::mt19937 rng(std::random_device{}());
			const float xavier = std::sqrt(6.0f / (2.0f + 3.0f));
			std::uniform_real_distribution<float> dist(-xavier, xavier);

			// Initialize weights and biases with small random values.
			for (int i = 0; i < 3; i++)
			{
				hidden1_bias[i] = dist(rng);
				for (int j = 0; j < 2; j++)
				{
					input_hidden1_weights[j][i] = dist(rng);
				}
			}

			for (int i = 0; i < 2; i++)
			{
				hidden2_output_weights[i] = dist(rng);
				hidden2_bias[i] = dist(rng);
				for (int j = 0; j < 3; j++)
				{
					hidden1_hidden2_weights[j][i] = dist(rng);
				}
			}

			output_bias = dist(rng);
		}

		/**
		 * @brief Tains the network with a single data point using backpropagation.
		 * @param input A 2-element input vector.
		 * @param target The expected output for the given input.
		 * @param lr Learning rate used to update the weights and biases.
		 */
		void Train(const std::vector<float>& input, float target, float lr)
		{
			// ----------------------------------
			// Forward pass.
			// ----------------------------------

			// Input layer -> Hidden layer 1.
			float hidden1[3];
			for (int i = 0; i < 3; i++)
			{
				// Compute the activation of each hidden neuron.
				hidden1[i] = Tanh(
					input[0] * input_hidden1_weights[0][i] +
					input[1] * input_hidden1_weights[1][i] +
					hidden1_bias[i]
				);
			}

			// Hidden layer 1 -> Hidden layer 2.
			float hidden2[2];
			for (int i = 0; i < 2; i++)
			{
				hidden2[i] = Tanh(
					hidden1[0] * hidden1_hidden2_weights[0][i] +
					hidden1[1] * hidden1_hidden2_weights[1][i] +
					hidden1[2] * hidden1_hidden2_weights[2][i] +
					hidden2_bias[i]
				);
			}

			// Hidden Layer 2 -> Output layer.
			float output = Sigmoid(
				hidden2[0] * hidden2_output_weights[0] +
				hidden2[1] * hidden2_output_weights[1] +
				output_bias
			);

			// Compute error (expected - actual).
			float error = target - output;

			// ----------------------------------
			// Backpropagation.
			// ----------------------------------

			// Output layer error.
			float d_output = error * SigmoidDerivative(output);

			// Hidden layer 2 error.
			float d_hidden2[2];
			for (int i = 0; i < 2; i++)
			{
				d_hidden2[i] = d_output * hidden2_output_weights[i] * TanhDerivative(hidden2[i]);
			}

			// Hidden layer 1 error.
			float d_hidden1[3];
			for (int i = 0; i < 3; i++)
			{
				d_hidden1[i] = 0.0f;
				for (int j = 0; j < 2; j++)
				{
					d_hidden1[i] += d_hidden2[j] * hidden1_hidden2_weights[i][j];
				}

				d_hidden1[i] *= TanhDerivative(hidden1[i]);
			}

			// ----------------------------------
			// Update weights and bias.
			// ----------------------------------

			// Hidden layer 2 -> Output layer weights.
			for (int i = 0; i < 2; i++)
			{
				hidden2_output_weights[i] += lr * d_output * hidden2[i];
			}

			// Output layer bias.
			output_bias += lr * d_output;

			// Hidden layer 1 -> Hidden layer 2 weights.
			for (int i = 0; i < 3; i++)
			{
				for (int j = 0; j < 2; j++)
				{
					hidden1_hidden2_weights[i][j] += lr * d_hidden2[j] * hidden1[i];
				}
			}

			// Hidden layer 2 bias.
			for (int i = 0; i < 2; i++)
			{
				hidden2_bias[i] += lr * d_hidden2[i];
			}

			// Input layer -> Hidden layer 1 weights.
			for (int i = 0; i < 2; i++)
			{
				for (int j = 0; j < 3; j++)
				{
					input_hidden1_weights[i][j] += lr * d_hidden1[j] * input[i];
				}
			}

			// Hidden layer 1 bias.
			for (int i = 0; i < 3; i++)
			{
				hidden1_bias[i] += lr * d_hidden1[i];
			}
		}

		/**
		 * @brief Predicts the output for a given input using a forward pass.
		 * @param input A 2-element input vector.
		 * @return The predicted output value from the neural network.
		 */
		// Predict the output for a given input.
		float Predict(const std::vector<float>& input)
		{
			// Input layer -> Hidden layer 1.
			float hidden1[3];
			for (int i = 0; i < 3; i++)
			{
				// Compute the activation of each hidden neuron.
				hidden1[i] = Tanh(
					input[0] * input_hidden1_weights[0][i] +
					input[1] * input_hidden1_weights[1][i] +
					hidden1_bias[i]
				);
			}

			// Hidden layer 1 -> Hidden layer 2.
			float hidden2[2];
			for (int i = 0; i < 2; i++)
			{
				hidden2[i] = Tanh(
					hidden1[0] * hidden1_hidden2_weights[0][i] +
					hidden1[1] * hidden1_hidden2_weights[1][i] +
					hidden1[2] * hidden1_hidden2_weights[2][i] +
					hidden2_bias[i]
				);
			}

			// Hidden layer 2 -> Output layer.
			float output = Sigmoid(
				hidden2[0] * hidden2_output_weights[0] +
				hidden2[1] * hidden2_output_weights[1] +
				output_bias
			);

			return output;
		}

	private:

		/**
		 * @brief Applies the sigmoid activation function.
		 * @param x Input value.
		 * @return Sigmoid of x.
		 */
		float Sigmoid(float x)
		{
			return 1.0f / (1.0f + std::exp(-x));
		}

		/**
		 * @brief Derivative of the sigmoid activation function.
		 * Assumes the input x is already sigmoid(x).
		 * 
		 * @param x Output of the sigmoid function.
		 * @return Derivative value.
		 */
		float SigmoidDerivative(float x)
		{
			return x * (1.0f - x); // Assume x is already Sigmoid(x).
		}

		/**
		 * @brief Applies the hyperbolic tangent activation function.
		 * @param x Input value.
		 * @return Tanh of x.
		 */
		float Tanh(float x)
		{
			return std::tanh(x);
		}

		/**
		 * @brief Derivative of the hyperbolic tangent activation function.
		 * Assumes the input x is already tanh(x).
		 * 
		 * @param x Output of the tanh function.
		 * @return Derivative value.
		 */
		float TanhDerivative(float x)
		{
			return 1.0f - (x * x);
		}
	};
}