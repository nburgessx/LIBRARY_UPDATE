#pragma once

#include "LAQuantLibOptimizationMethod.h"
#include "LAQuantLibEndCriteria.h"


class LAQuantLibSteepestDescent : public LAQuantLibOptimizationMethod
{
	// TODO: This class should be derived from LAQuantLibLineSearchBasedMethod is
public:
	LAQuantLibSteepestDescent();

	virtual ~LAQuantLibSteepestDescent();

	virtual LAQuantLibEndCriteria::Type minimize(LAQuantLibProblem& P, const LAQuantLibEndCriteria& endCriteria);

};
