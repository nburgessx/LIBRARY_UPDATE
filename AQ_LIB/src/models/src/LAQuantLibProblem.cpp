/*!
	@file LAQuantLibProblem.cpp
	@brief a class of problem to be optimized
		   using external library

 */

#include <ql/math/optimization/problem.hpp>
//#include <ql/math/array.hpp>
#include "LAQuantLibProblem.h"
#include "LAQuantLibCostFunction.h"
#include "LAQuantLibConstraint.h"
#include "LAQuantLibProblem.h"

LAQuantLibProblem::LAQuantLibProblem(LAQuantLibCostFunction& costFunction,
                                     LAQuantLibConstraint& constraint,
									 LAQuantLibArray& initialValue)
//									 DoubleVector& initialValue)
{
//	QuantLib::Array initial_value(initialValue);
	mProblem = new QuantLib::Problem(*costFunction.getCostFunction(), *constraint.getConstraint(), *initialValue.getArray());
//	mProblem = new QuantLib::Problem(*costFunction.getCostFunction(), *constraint.getConstraint(), initial_value);
}

LAQuantLibProblem::~LAQuantLibProblem()
{
	delete mProblem;
}

double
LAQuantLibProblem::value(const LAQuantLibArray& x)
//LAQuantLibProblem::value(const DoubleVector& x)
{
//	QuantLib::Array x_(x);
//	return mProblem->value(x_);
	return mProblem->value(*(x.getArray()));
}

LAQuantLibArray
//DoubleVector
LAQuantLibProblem::currentValue(void)
{
	LAQuantLibArray ret(mProblem->currentValue());
//	QuantLib::Array values = mProblem->currentValue();

//	DoubleVector ret(values.size());
//	for(size_t i = 0; i < values.size(); i++)
//		ret[i] = values[i];

	return ret;
}
