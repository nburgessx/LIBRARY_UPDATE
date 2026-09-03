#pragma once

#include "LAQuantLibOptimizationMethod.h"
#include "LAQuantLibEndCriteria.h"


class LAQuantLibLevenbergMarquardt : public LAQuantLibOptimizationMethod
{
public:
	LAQuantLibLevenbergMarquardt(double epsfcn, double xtol, double gtol);

	virtual ~LAQuantLibLevenbergMarquardt();

	virtual LAQuantLibEndCriteria::Type minimize(LAQuantLibProblem& P, const LAQuantLibEndCriteria& endCriteria);

};
