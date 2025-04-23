#pragma once

#include "pch.h"

namespace Engine
{
	class LossFunction
	{
	public:

		virtual float Compute(const std::vector<float>& target, const std::vector<float>& output) const = 0;
		virtual std::vector<float> Derivative(const std::vector<float>& target, const std::vector<float>& output) const = 0;
		virtual ~LossFunction() = default;
	};

	class MSELoss : public LossFunction
	{
	public:

		float Compute(const std::vector<float>& target, const std::vector<float>& output) const override
		{
			float sum = 0.0f;
			for (size_t i = 0; i < target.size(); i++)
			{
				float diff = target[i] - output[i];
				sum += diff * diff;
			}

			return sum / static_cast<float>(target.size());
		}

		std::vector<float> Derivative(const std::vector<float>& target, const std::vector<float>& output) const override
		{
			std::vector<float> grad(target.size());
			for (size_t i = 0; i < target.size(); i++)
			{
				grad[i] = (target[i] - output[i]);
			}

			return grad;
		}
	};
}