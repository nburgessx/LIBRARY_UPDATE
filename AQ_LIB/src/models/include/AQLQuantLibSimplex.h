#pragma once

#include "AQLQuantLibOptimizationMethod.h"
#include "AQLQuantLibEndCriteria.h"

class AQLQuantLibSimplex : public AQLQuantLibOptimizationMethod
{
	// TODO: This class should be derived from LAQuantLibLineSearchBasedMethod because Simplex class is
public:
	AQLQuantLibSimplex(double lambda);

	virtual ~AQLQuantLibSimplex();

	virtual AQLQuantLibEndCriteria::Type minimize(AQLQuantLibProblem& P, const AQLQuantLibEndCriteria& endCriteria);

};
