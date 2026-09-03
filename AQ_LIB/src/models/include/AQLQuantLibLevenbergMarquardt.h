#pragma once

#include "AQLQuantLibOptimizationMethod.h"
#include "AQLQuantLibEndCriteria.h"


class AQLQuantLibLevenbergMarquardt : public AQLQuantLibOptimizationMethod
{
public:
	AQLQuantLibLevenbergMarquardt(double epsfcn, double xtol, double gtol);

	virtual ~AQLQuantLibLevenbergMarquardt();

	virtual AQLQuantLibEndCriteria::Type minimize(AQLQuantLibProblem& P, const AQLQuantLibEndCriteria& endCriteria);

};
