#pragma once

#include "pch.h"

namespace Engine
{
	class RecurrentNeuralNetwork
	{
	public:

		size_t inputSize, hiddenSize, outputSize;

		std::vector<std::vector<float>> w_x; // Input layer -> Hidden layer weight.
		std::vector<std::vector<float>> w_h; // Hidden layer -> Hidden (recurrent) weight.
		std::vector<std::vector<float>> w_y; // Hidden layer -> Output layer weight
		std::vector<float> b_h;				 // Hidden layer bias.
		std::vector<float> b_y;				 // Output layer bias.
		std::vector<float> h;				 // Hidden (recurrent) states.

		// Histories for backpropagation through time (BPTT).
		std::vector<std::vector<float>> hHistory;
		std::vector<std::vector<float>> xHistory;
		std::vector<std::vector<float>> preActHistory;

		RecurrentNeuralNetwork(size_t inputSize, size_t hiddenSize, size_t outputSize) :
			inputSize(inputSize), hiddenSize(hiddenSize), outputSize(outputSize),
			w_x(hiddenSize, std::vector<float>(inputSize, 0.1f)),
			w_h(hiddenSize, std::vector<float>(hiddenSize, 0.1f)),
			w_y(hiddenSize, std::vector<float>(outputSize, 0.1f)),
			b_h(hiddenSize, 0.0f), b_y(outputSize, 0.0f),
			h(hiddenSize, 0.0f) {}

		std::vector<float> Step(const std::vector<float>& x_t)
		{

			std::vector<float> preAct = VecAdd(MatVecMul(w_x, x_t), MatVecMul(w_h, h));
			preAct = VecAdd(preAct, b_h);
			
			h = ApplyTanh(preAct);

			std::vector<float> y = VecAdd(MatVecMul(w_y, h), b_y);

			xHistory.push_back(x_t);
			hHistory.push_back(h);
			preActHistory.push_back(preAct);

			return y;
		}

		void Backward(const std::vector<std::vector<float>>& targetSeq, float lr = 0.01f)
		{
			// Derivatives of weights.
			std::vector<std::vector<float>> dW_x(hiddenSize, std::vector<float>(inputSize, 0.0f));
			std::vector<std::vector<float>> dW_h(hiddenSize, std::vector<float>(inputSize, 0.0f));
			std::vector<std::vector<float>> dW_y(outputSize, std::vector<float>(inputSize, 0.0f));

			// Derivatives of biases.
			std::vector<float> dB_h(hiddenSize, .0f);
			std::vector<float> dB_y(outputSize, .0f);

			// Derivatives of hidden state.
			std::vector<float> dH_next(hiddenSize, .0f);

			for (int t = targetSeq.size() - 1; t >= 0; t--)
			{
				std::vector<float> y = VecAdd(MatVecMul(w_y, hHistory[t]), b_y);
				std::vector<float> dy = VecSub(y, targetSeq[t]);

				std::vector<std::vector<float>> dWy_t = OuterProduct(dy, hHistory[t]);
				std::vector<float> dBy_t = dy;

				std::vector<float> dH = MatVecMul(Transpose(w_y), dy);
				for (size_t i = 0; i < hiddenSize; i++)
				{
					dH[i] += dH_next[i];
				}

				std::vector<float> dH_raw(VecHadamard(dH, TanhDerivativeVec(preActHistory[t])));

				std::vector<std::vector<float>> dWx_t = OuterProduct(dH_raw, xHistory[t]);
				std::vector<std::vector<float>> dWh_t = OuterProduct(dH_raw, (t > 0 ? hHistory[t - 1] : std::vector<float>(hiddenSize, 0.0f)));

				for (size_t i = 0; i < hiddenSize; i++)
				{
					for (size_t j = 0; j < inputSize; j++)
					{
						dW_x[i][j] += dWx_t[i][j]; // TODO
					}

					for (size_t j = 0; j < hiddenSize; j++)
					{
						dW_h[i][j] += dWh_t[i][j];
					}

					dB_h[i] += dH_raw[i];
				}

				for (size_t i = 0; i < outputSize; i++)
				{
					for (size_t j = 0; j < hiddenSize; j++)
					{
						dW_y[i][j] += dWy_t[i][j];
					}
				}

				for (size_t i = 0; i < outputSize; i++)
				{
					dB_y[i] += dBy_t[i];
				}

				dH_next = MatVecMul(Transpose(w_h), dH_raw);
			}

			ScaleMat(dW_x, lr);
			ScaleMat(dW_h, lr);
			ScaleMat(dW_y, lr);

			ScaleVec(dB_h, lr);
			ScaleVec(dB_y, lr);

			SubMat(w_x, dW_x);
			SubMat(w_h, dW_h);
			SubMat(w_y, dW_y);

			for (size_t i = 0; i < hiddenSize; i++)
			{
				b_h[i] -= dB_h[i];
			}

			for (size_t i = 0; i < outputSize; i++)
			{
				b_y[i] -= dB_y[i];
			}

			ResetHistory();
		}

		void ResetHistory()
		{
			xHistory.clear();
			hHistory.clear();
			preActHistory.clear();
		}

		void ResetState()
		{
			std::fill(h.begin(), h.end(), 0.0f);
			ResetHistory();
		}

		static void PrintVec(const std::vector<float>& v)
		{
			std::cout << "[ ";
			for (float x : v)
			{
				std::cout << x << " ";
			}

			std::cout << "]";
		}

	private:

		std::vector<float> MatVecMul(const std::vector<std::vector<float>>& mat, const std::vector<float>& vec)
		{
			std::vector<float> result(mat.size(), 0.0f);
			for (size_t i = 0; i < mat.size(); i++)
			{
				for (size_t j = 0; j < vec.size(); j++)
				{
					result[i] += mat[i][j] * vec[i];
				}
			}

			return result;
		}

		std::vector<std::vector<float>> Transpose(const std::vector<std::vector<float>>& mat)
		{
			size_t rows = mat.size();
			size_t cols = mat[0].size();
			std::vector<std::vector<float>> result(cols, std::vector<float>(rows));
			for (size_t i = 0; i < rows; i++)
			{
				for (size_t j = 0; j < cols; j++)
				{
					result[j][i] = mat[i][j];
				}
			}

			return result;
		}

		std::vector<std::vector<float>> OuterProduct(const std::vector<float>& a, const std::vector<float>& b)
		{
			std::vector<std::vector<float>> result(a.size(), std::vector<float>(b.size()));
			for (size_t i = 0; i < a.size(); i++)
			{
				for (size_t j = 0; j < b.size(); j++)
				{
					result[i][j] = a[i] * b[j];
				}
			}

			return result;
		}

		std::vector<float> VecAdd(const std::vector<float>& a, const std::vector<float>& b)
		{
			std::vector<float> result(a.size());
			for (size_t i = 0; i < a.size(); i++)
			{
				result[i] = a[i] * b[i];
			}

			return result;
		}

		std::vector<float> VecSub(const std::vector<float>& a, const std::vector<float>& b)
		{
			std::vector<float> result(a.size());
			for (size_t i = 0; i < a.size(); i++)
			{
				result[i] = a[i] - b[i];
			}

			return result;
		}

		std::vector<float> VecHadamard(const std::vector<float>& a, const std::vector<float>& b)
		{
			std::vector<float> result(a.size());
			for (size_t i = 0; i < a.size(); i++)
			{
				result[i] = a[i] * b[i];
			}

			return result;
		}

		void ScaleVec(std::vector<float>& vec, float scalar)
		{
			for (float& v : vec)
			{
				v *= scalar;
			}
		}

		void ScaleMat(std::vector<std::vector<float>>& mat, float scalar)
		{
			for (std::vector<float>& row : mat)
			{
				ScaleVec(row, scalar);
			}
		}

		void SubMat(std::vector<std::vector<float>>& a, const std::vector<std::vector<float>>& b)
		{
			for (size_t i = 0; i < a.size(); i++)
			{
				for (size_t j = 0; j < a[0].size(); j++)
				{
					a[i][j] -= b[i][j];
				}
			}
		}

		std::vector<float> ApplyTanh(const std::vector<float>& v)
		{
			std::vector<float> result(v.size());
			for (size_t i = 0; i < v.size(); i++)
			{
				result[i] = TanhActivation(v[i]);
			}

			return result;
		}

		inline float TanhActivation(float x)
		{
			return std::tanh(x);
		}

		inline float TanhDerivative(float x)
		{
			float t = std::tanh(x);

			return 1.0f - t * t;
		}

		std::vector<float> TanhVec(const std::vector<float>& v)
		{
			std::vector<float> result(v.size());
			for (size_t i = 0; i < v.size(); i++)
			{
				result[i] = TanhActivation(v[i]);
			}

			return result;
		}

		std::vector<float> TanhDerivativeVec(const std::vector<float>& v)
		{
			std::vector<float> result(v.size());
			for (size_t i = 0; i < v.size(); i++)
			{
				result[i] = TanhDerivative(v[i]);
			}

			return result;
		}
	};
}