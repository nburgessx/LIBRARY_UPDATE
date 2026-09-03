/*!
	@file LAQuantLibConjugateGradient.cpp
	@brief a class of optimization method : conjugate gradient method
		   using external library

 */

#include <ql/math/optimization/conjugategradient.hpp>
#include "LAQuantLibConjugateGradient.h"
#include "LAQuantLibProblem.h"

LAQuantLibConjugateGradient::LAQuantLibConjugateGradient()
{
	mOptMethod = new QuantLib::ConjugateGradient();
}

LAQuantLibConjugateGradient::~LAQuantLibConjugateGradient()
{
	delete mOptMethod;
}

LAQuantLibEndCriteria::Type
LAQuantLibConjugateGradient::minimize(LAQuantLibProblem& P, const LAQuantLibEndCriteria& endCriteria)
{
	return (LAQuantLibEndCriteria::Type)mOptMethod->minimize(*P.getProblem(), *endCriteria.getEndCriteria());
}
