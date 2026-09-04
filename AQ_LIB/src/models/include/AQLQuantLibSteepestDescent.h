#pragma once

#include "AQLQuantLibOptimizationMethod.h"
#include "AQLQuantLibEndCriteria.h"


class AQLQuantLibSteepestDescent : public AQLQuantLibOptimizationMethod
{
	// TODO: This class should be derived from AQLQuantLibLineSearchBasedMethod is
public:
	AQLQuantLibSteepestDescent();

	virtual ~AQLQuantLibSteepestDescent();

	virtual AQLQuantLibEndCriteria::Type minimize(AQLQuantLibProblem& P, const AQLQuantLibEndCriteria& endCriteria);

};
