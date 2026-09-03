/*!
	@file LAQuantLibEndCriteria.cpp
	@brief a class of end criteria to complete optimization
		   using external library

 */

#include <ql/math/optimization/endcriteria.hpp>
#include "LAQuantLibEndCriteria.h"

LAQuantLibEndCriteria::LAQuantLibEndCriteria(size_t maxIterations,
                                             size_t maxStationaryStateIterations,
                                             double rootEpsilon,
                                             double functionEpsilon,
                                             double gradientNormEpsilon)
{
	mEndCriteria = new QuantLib::EndCriteria(maxIterations,
											 maxStationaryStateIterations,
											 rootEpsilon,
											 functionEpsilon,
											 gradientNormEpsilon);
}

LAQuantLibEndCriteria::~LAQuantLibEndCriteria()
{
	delete mEndCriteria;
}

