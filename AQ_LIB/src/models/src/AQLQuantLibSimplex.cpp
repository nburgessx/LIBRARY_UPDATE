/*!
	@file AQLQuantLibSimplex.cpp
	@brief a class of optimization method : simplex method
		   using external library
 */

#include <ql/math/optimization/simplex.hpp>
#include "AQLQuantLibSimplex.h"
#include "AQLQuantLibProblem.h"

AQLQuantLibSimplex::AQLQuantLibSimplex(double lambda)
{
	mOptMethod = new QuantLib::Simplex(lambda);
}

AQLQuantLibSimplex::~AQLQuantLibSimplex()
{
	delete mOptMethod;
}

AQLQuantLibEndCriteria::Type
AQLQuantLibSimplex::minimize(AQLQuantLibProblem& P, const AQLQuantLibEndCriteria& endCriteria)
{
	return (AQLQuantLibEndCriteria::Type)mOptMethod->minimize(*P.getProblem(), *endCriteria.getEndCriteria());
}

