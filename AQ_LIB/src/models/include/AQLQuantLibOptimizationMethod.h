#pragma once

#include <ql/math/optimization/endcriteria.hpp>
#include <ql/math/optimization/method.hpp>
#include "AQLQuantLibEndCriteria.h"

class AQLQuantLibProblem;


class AQLQuantLibOptimizationMethod
{
public:
	AQLQuantLibOptimizationMethod();

	virtual ~AQLQuantLibOptimizationMethod();

	virtual AQLQuantLibEndCriteria::Type minimize(AQLQuantLibProblem& P, const AQLQuantLibEndCriteria& endCriteria) = 0;

protected:
	QuantLib::OptimizationMethod *mOptMethod;
};
