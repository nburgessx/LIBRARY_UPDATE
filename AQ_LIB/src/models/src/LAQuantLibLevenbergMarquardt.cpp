/*!
	@file LAQuantLibLevenbergMarquardt.cpp
	@brief a class of optimization method : Levenberg Marquardt method
		   using external library

 */

#include <ql/math/optimization/levenbergmarquardt.hpp>
#include "LAQuantLibLevenbergMarquardt.h"
#include "LAQuantLibProblem.h"

LAQuantLibLevenbergMarquardt::LAQuantLibLevenbergMarquardt(double epsfcn, double xtol, double gtol)
{
	mOptMethod = new QuantLib::LevenbergMarquardt(epsfcn, xtol, gtol);
}

LAQuantLibLevenbergMarquardt::~LAQuantLibLevenbergMarquardt()
{
	delete mOptMethod;
}

LAQuantLibEndCriteria::Type
LAQuantLibLevenbergMarquardt::minimize(LAQuantLibProblem& P, const LAQuantLibEndCriteria& endCriteria)
{
	return (LAQuantLibEndCriteria::Type)mOptMethod->minimize(*P.getProblem(), *endCriteria.getEndCriteria());
}

