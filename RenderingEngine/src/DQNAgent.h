#pragma once

#include "NeuralNetwork.h"

#include "LossFunction.h"

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

		DQNAgent(int stateSize, int actionSize, LossFunction* loss, size_t replayCapacity = 10000) : 
			stateSize(stateSize), 
			actionSize(actionSize), 
			qNet({ stateSize, 32, 32, actionSize }, loss),
			targetNet({ stateSize, 32, 32, actionSize}, loss),
			replayBuffer(replayCapacity),
			rng(static_cast<unsigned>(std::time(nullptr)))
		{
			// Initialize target network.
			targetNet = qNet;
		}

		int Act(const std::vector<float>& state, float epsilon)
		{
			std::uniform_real_distribution<float> floatDist(0.0f, 1.0f);
			std::uniform_int_distribution<int> actionDist(0, actionSize - 1);

			if (floatDist(rng) < epsilon)
			{
				int action = actionDist(rng);

				return action;
			}

			std::vector<float> qValues = qNet.Predict(state);

			return std::distance(qValues.begin(), std::max_element(qValues.begin(), qValues.end()));
		}

		void Step(const std::vector<float>& state, int action, float reward, const std::vector<float>& nextState, bool done)
		{
			replayBuffer.Add({ state, action, reward, nextState, done });
		}

		void TrainBatch(float gamma, float lr, size_t batchSize)
		{
			if (!replayBuffer.CanSample(batchSize))
			{
				return;
			}

			std::vector<Experience> batch = replayBuffer.Sample(batchSize);

			for (const Experience& exp : batch)
			{
				std::vector<float> target = qNet.Predict(exp.state);
				std::vector<float> nextQ = targetNet.Predict(exp.nextState);

				float maxQ = *std::max_element(nextQ.begin(), nextQ.end());
				target[exp.action] = exp.reward + (exp.done ? 0.0f : gamma * maxQ);

				qNet.Train(exp.state, target, lr);
			}

		}

		void Train(const std::vector<float>& state, int action, float reward, const std::vector<float>& nextState, bool done, float gamma, float lr)
		{
			std::vector<float> target = qNet.Predict(state);
			std::vector<float> nextQ = qNet.Predict(nextState);

			float maxNetQ = *std::max_element(nextQ.begin(), nextQ.end());
			target[action] = reward + (done ? 0.0f : gamma * maxNetQ);

			qNet.Train(state, target, lr);
		}

		void UpdateTargetNetwork()
		{
			targetNet = qNet;
		}

		void SoftUpdateTarget(float tau)
		{
			// TODO.
			//targetNet.SoftUpdateFrom(qNet, tau);
		}

	private:

		int stateSize;
		int actionSize;

		NeuralNetwork qNet;
		NeuralNetwork targetNet;

		ReplayBuffer replayBuffer;
		
		std::mt19937 rng;
	};
}