/*!
	@file AQLQuantLibLevenbergMarquardt.cpp
	@brief a class of optimization method : Levenberg Marquardt method
		   using external library

 */

#include <ql/math/optimization/levenbergmarquardt.hpp>
#include "AQLQuantLibLevenbergMarquardt.h"
#include "AQLQuantLibProblem.h"

AQLQuantLibLevenbergMarquardt::AQLQuantLibLevenbergMarquardt(double epsfcn, double xtol, double gtol)
{
	mOptMethod = new QuantLib::LevenbergMarquardt(epsfcn, xtol, gtol);
}

AQLQuantLibLevenbergMarquardt::~AQLQuantLibLevenbergMarquardt()
{
	delete mOptMethod;
}

AQLQuantLibEndCriteria::Type
AQLQuantLibLevenbergMarquardt::minimize(AQLQuantLibProblem& P, const AQLQuantLibEndCriteria& endCriteria)
{
	return (AQLQuantLibEndCriteria::Type)mOptMethod->minimize(*P.getProblem(), *endCriteria.getEndCriteria());
}

