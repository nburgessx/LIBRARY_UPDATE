/*!
	@file AQLQuantLibSteepestDescent.cpp
	@brief a class of optimization method : steepest descent method
		   using external library

 */

#include <ql/math/optimization/steepestdescent.hpp>
#include "AQLQuantLibSteepestDescent.h"
#include "AQLQuantLibProblem.h"

AQLQuantLibSteepestDescent::AQLQuantLibSteepestDescent()
{
	mOptMethod = new QuantLib::SteepestDescent();
}

AQLQuantLibSteepestDescent::~AQLQuantLibSteepestDescent()
{
	delete mOptMethod;
}

AQLQuantLibEndCriteria::Type
AQLQuantLibSteepestDescent::minimize(AQLQuantLibProblem& P, const AQLQuantLibEndCriteria& endCriteria)
{
	return (AQLQuantLibEndCriteria::Type)mOptMethod->minimize(*P.getProblem(), *endCriteria.getEndCriteria());
}

