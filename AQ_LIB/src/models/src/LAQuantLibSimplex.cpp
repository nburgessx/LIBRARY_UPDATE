/*!
	@file LAQuantLibSimplex.cpp
	@brief a class of optimization method : simplex method
		   using external library

 */

#include <ql/math/optimization/simplex.hpp>
#include "LAQuantLibSimplex.h"
#include "LAQuantLibProblem.h"

LAQuantLibSimplex::LAQuantLibSimplex(double lambda)
{
	mOptMethod = new QuantLib::Simplex(lambda);
}

LAQuantLibSimplex::~LAQuantLibSimplex()
{
	delete mOptMethod;
}

LAQuantLibEndCriteria::Type
LAQuantLibSimplex::minimize(LAQuantLibProblem& P, const LAQuantLibEndCriteria& endCriteria)
{
	return (LAQuantLibEndCriteria::Type)mOptMethod->minimize(*P.getProblem(), *endCriteria.getEndCriteria());
}

