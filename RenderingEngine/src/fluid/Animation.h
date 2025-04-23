#pragma once

#include "pch.h"

namespace Engine
{
	struct Frame
	{
		unsigned int index = 0;
		double timeIntervalInSeconds = 1.0 / 60.0;

		double TimeInSeconds() const
		{
			return index * timeIntervalInSeconds;
		}

		void Advance()
		{
			index++;
		}

		void Advance(unsigned int delta)
		{
			index += delta;
		}
	};

	class Animation
	{
	public:

		void Update(const Frame& frame)
		{
			// Some pre-processing here.

			OnUpdate(frame);

			// Some post-processing here.
		}

	protected:
		virtual void OnUpdate(const Frame& frame) = 0;
	};

	class SineAnimation : public Animation
	{
	public:

		double x = 0.0;

	protected:

		void OnUpdate(const Frame& frame) override
		{
			x = std::sin(frame.TimeInSeconds());
		}
	};
}