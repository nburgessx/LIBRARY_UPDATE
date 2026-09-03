#pragma once

#include "AQLCoreTemplateType.h"
#include "LAQuantLibArray.h"
#include "LAQuantLibConstraint.h"
#include <ql/math/optimization/problem.hpp>


class LAQuantLibCostFunction;
class LAQuantLibConstraint;


class LAQuantLibProblem
{
public:
	LAQuantLibProblem(LAQuantLibCostFunction& costFunction,
					  LAQuantLibConstraint& constraint,
					  LAQuantLibArray& initialValue);
//					  DoubleVector& initialValue);

	~LAQuantLibProblem();

	QuantLib::Problem* getProblem(void) { return mProblem; }

	// call cost function computation and increment evaluation counter
	double value(const LAQuantLibArray& x);
//	double value(const DoubleVector& x);

	LAQuantLibArray currentValue(void);
//	DoubleVector currentValue(void);

private:
	QuantLib::Problem *mProblem;
};
