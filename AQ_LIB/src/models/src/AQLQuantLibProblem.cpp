/*!
	@file AQLQuantLibProblem.cpp
	@brief a class of problem to be optimized
		   using external library
 */

#include <ql/math/optimization/problem.hpp>
//#include <ql/math/array.hpp>
#include "AQLQuantLibProblem.h"
#include "AQLQuantLibCostFunction.h"
#include "AQLQuantLibConstraint.h"
#include "AQLQuantLibProblem.h"

AQLQuantLibProblem::AQLQuantLibProblem(AQLQuantLibCostFunction& costFunction,
                                     AQLQuantLibConstraint& constraint,
									 AQLQuantLibArray& initialValue)
//									 DoubleVector& initialValue)
{
//	QuantLib::Array initial_value(initialValue);
	mProblem = new QuantLib::Problem(*costFunction.getCostFunction(), *constraint.getConstraint(), *initialValue.getArray());
//	mProblem = new QuantLib::Problem(*costFunction.getCostFunction(), *constraint.getConstraint(), initial_value);
}

AQLQuantLibProblem::~AQLQuantLibProblem()
{
	delete mProblem;
}

double
AQLQuantLibProblem::value(const AQLQuantLibArray& x)
//AQLQuantLibProblem::value(const DoubleVector& x)
{
//	QuantLib::Array x_(x);
//	return mProblem->value(x_);
	return mProblem->value(*(x.getArray()));
}

AQLQuantLibArray
//DoubleVector
AQLQuantLibProblem::currentValue(void)
{
	AQLQuantLibArray ret(mProblem->currentValue());
//	QuantLib::Array values = mProblem->currentValue();

//	DoubleVector ret(values.size());
//	for(size_t i = 0; i < values.size(); i++)
//		ret[i] = values[i];

	return ret;
}
