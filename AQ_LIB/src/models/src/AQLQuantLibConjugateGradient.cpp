/*!
	@file AQLQuantLibConjugateGradient.cpp
	@brief a class of optimization method : conjugate gradient method
		   using external library
 */

#include <ql/math/optimization/conjugategradient.hpp>
#include "AQLQuantLibConjugateGradient.h"
#include "AQLQuantLibProblem.h"

AQLQuantLibConjugateGradient::AQLQuantLibConjugateGradient()
{
	mOptMethod = new QuantLib::ConjugateGradient();
}

AQLQuantLibConjugateGradient::~AQLQuantLibConjugateGradient()
{
	delete mOptMethod;
}

AQLQuantLibEndCriteria::Type
AQLQuantLibConjugateGradient::minimize(AQLQuantLibProblem& P, const AQLQuantLibEndCriteria& endCriteria)
{
	return (AQLQuantLibEndCriteria::Type)mOptMethod->minimize(*P.getProblem(), *endCriteria.getEndCriteria());
}
