#pragma once

#include "pch.h"

namespace Engine
{
	class Environment
	{
	public:

		static const int stateSize = 4;
		static const int actionSize = 2;

		/**
		 * @brief Resets the environment to its initial state.
		 */
		std::vector<float> Reset()
		{
			return std::vector<float>(stateSize, 0.0f);
		}

		/**
		 * @brief Steps the environment forward.
		 * @param action The action to take.
		 * @return A tuple containing the next state, the reward, and whether the episode is done.
		 */
		std::tuple<std::vector<float>, float, bool> Step(int action)
		{
			std::vector<float> nextState(stateSize, static_cast<float>(action));

			// Reward is only assigned if action 1 is selected.
			// This should encourage the agent to learn to always
			// select action 1.
			float reward = (action == 1) ? 1.0f : 0.0f;
			bool done = false;

			return { nextState, reward, done };
		}
	};
}