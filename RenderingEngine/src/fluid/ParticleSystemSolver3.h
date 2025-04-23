#pragma once

#include "ParticleSystemData3.h"

namespace Engine
{
	class ParticleSystemSolver3
	{
	public:

		//ParticleSystemSolver3();
		//virtual ~ParticleSystemSolver3();

		//void OnAdvanceTimeStep(double timeIntervalInSeconds) override;
		//virtual void AccumulateForces(double timeStepInSeconds);
		//void ResolveCollision();

	private:

		std::shared_ptr<ParticleSystemData3> particleSystemData;

		//void BeginAdvanceTimeStep();
		//void EndAdvanceTimeStep();
		//void TimeIntegration(double timeIntervalInSeconds);
	};
}