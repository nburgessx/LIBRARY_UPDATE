#pragma once

#include "AQLCoreTemplateType.h"
#include "AQLQuantLibArray.h"
#include "AQLQuantLibConstraint.h"
#include <ql/math/optimization/problem.hpp>


class AQLQuantLibCostFunction;
class AQLQuantLibConstraint;


class AQLQuantLibProblem
{
public:
	AQLQuantLibProblem(AQLQuantLibCostFunction& costFunction,
					  AQLQuantLibConstraint& constraint,
					  AQLQuantLibArray& initialValue);
//					  DoubleVector& initialValue);

	~AQLQuantLibProblem();

	QuantLib::Problem* getProblem(void) { return mProblem; }

	// call cost function computation and increment evaluation counter
	double value(const AQLQuantLibArray& x);
//	double value(const DoubleVector& x);

	AQLQuantLibArray currentValue(void);
//	DoubleVector currentValue(void);

private:
	QuantLib::Problem *mProblem;
};
