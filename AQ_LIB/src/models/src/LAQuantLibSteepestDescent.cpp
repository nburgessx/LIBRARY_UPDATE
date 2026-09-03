/*!
	@file LAQuantLibSteepestDescent.cpp
	@brief a class of optimization method : steepest descent method
		   using external library

 */

#include <ql/math/optimization/steepestdescent.hpp>
#include "LAQuantLibSteepestDescent.h"
#include "LAQuantLibProblem.h"

LAQuantLibSteepestDescent::LAQuantLibSteepestDescent()
{
	mOptMethod = new QuantLib::SteepestDescent();
}

LAQuantLibSteepestDescent::~LAQuantLibSteepestDescent()
{
	delete mOptMethod;
}

LAQuantLibEndCriteria::Type
LAQuantLibSteepestDescent::minimize(LAQuantLibProblem& P, const LAQuantLibEndCriteria& endCriteria)
{
	return (LAQuantLibEndCriteria::Type)mOptMethod->minimize(*P.getProblem(), *endCriteria.getEndCriteria());
}

