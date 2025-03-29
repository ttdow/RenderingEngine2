#pragma once

#include "NeuralNetwork.h"

namespace Engine
{
	struct Experience
	{
		std::vector<float> state;
		int action;
		float reward;
		std::vector<float> nextState;
		bool done;
	};

	class ReplayBuffer
	{
	public:

		ReplayBuffer(size_t capacity) : capacity(capacity) {}

		void Add(const Experience& exp)
		{
			if (buffer.size() < capacity)
			{
				buffer.push_back(exp);
			}
			else
			{
				buffer[index] = exp;
			}

			index = (index + 1) % capacity;
		}

		bool CanSample(size_t batchSize) const
		{
			return buffer.size() >= batchSize;
		}

		std::vector<Experience> Sample(size_t batchSize)
		{
			std::vector<Experience> batch;
			batch.reserve(batchSize);

			std::uniform_int_distribution<size_t> dist(0, buffer.size() - 1);
			for (size_t i = 0; i < batchSize; i++)
			{
				batch.push_back(buffer[dist(rng)]);
			}

			return batch;
		}

	private:

		std::vector<Experience> buffer;
		size_t capacity;
		size_t index = 0;
		std::mt19937 rng{ static_cast<unsigned>(std::time(nullptr)) };
	};


	class DQNAgent
	{
	public:

		DQNAgent(int stateSize, int actionSize) : stateSize(stateSize), actionSize(actionSize), qNet({ stateSize, 32, 32, actionSize })
		{
			// Nothing.
		}

		int Act(const std::vector<float>& state, float epsilon)
		{
			if (((float)rand() / RAND_MAX) < epsilon)
			{
				int action = rand() % actionSize;

				return action;
			}

			std::vector<float> qValues = qNet.Predict(state);

			return std::distance(qValues.begin(), std::max_element(qValues.begin(), qValues.end()));
		}

		void Train(const std::vector<float>& state, int action, float reward, const std::vector<float>& nextState, bool done, float gamma, float lr)
		{
			std::vector<float> target = qNet.Predict(state);
			std::vector<float> nextQ = qNet.Predict(nextState);

			float maxNetQ = *std::max_element(nextQ.begin(), nextQ.end());
			target[action] = reward + (done ? 0.0f : gamma * maxNetQ);

			qNet.Train(state, target, lr);
		}

	private:

		int stateSize;
		int actionSize;
		NeuralNetwork qNet;
	};
}