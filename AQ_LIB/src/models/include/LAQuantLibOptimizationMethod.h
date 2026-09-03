#pragma once

#include <ql/math/optimization/endcriteria.hpp>
#include <ql/math/optimization/method.hpp>
#include "LAQuantLibEndCriteria.h"

class LAQuantLibProblem;


class LAQuantLibOptimizationMethod
{
public:
	LAQuantLibOptimizationMethod();

	virtual ~LAQuantLibOptimizationMethod();

	virtual LAQuantLibEndCriteria::Type minimize(LAQuantLibProblem& P, const LAQuantLibEndCriteria& endCriteria) = 0;

protected:
	QuantLib::OptimizationMethod *mOptMethod;
};
