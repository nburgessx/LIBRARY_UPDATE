#pragma once

#include "LAQuantLibOptimizationMethod.h"
#include "LAQuantLibEndCriteria.h"

class LAQuantLibSimplex : public LAQuantLibOptimizationMethod
{
	// TODO: This class should be derived from LAQuantLibLineSearchBasedMethod because Simplex class is
public:
	LAQuantLibSimplex(double lambda);

	virtual ~LAQuantLibSimplex();

	virtual LAQuantLibEndCriteria::Type minimize(LAQuantLibProblem& P, const LAQuantLibEndCriteria& endCriteria);

};
