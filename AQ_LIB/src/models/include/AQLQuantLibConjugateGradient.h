#pragma once


#include "AQLQuantLibOptimizationMethod.h"
#include "AQLQuantLibEndCriteria.h"


class AQLQuantLibConjugateGradient : public AQLQuantLibOptimizationMethod
{
	// TODO: this class should be derived from AQLQuantLibLineSearchBasedMethod
	//       because ConjugateGradient class in Q-lib is so.
public:
	AQLQuantLibConjugateGradient();

	virtual ~AQLQuantLibConjugateGradient();

	virtual AQLQuantLibEndCriteria::Type minimize(AQLQuantLibProblem& P, const AQLQuantLibEndCriteria& endCriteria);

};
