#pragma once


#include "LAQuantLibOptimizationMethod.h"
#include "LAQuantLibEndCriteria.h"


class LAQuantLibConjugateGradient : public LAQuantLibOptimizationMethod
{
	// TODO: this class should be derived from LAQuantLibLineSearchBasedMethod
	//       because ConjugateGradient class in Q-lib is so.
public:
	LAQuantLibConjugateGradient();

	virtual ~LAQuantLibConjugateGradient();

	virtual LAQuantLibEndCriteria::Type minimize(LAQuantLibProblem& P, const LAQuantLibEndCriteria& endCriteria);

};
