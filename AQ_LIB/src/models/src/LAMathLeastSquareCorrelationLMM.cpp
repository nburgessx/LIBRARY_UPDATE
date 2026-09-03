/*! @file LAMathLeastSquareCorrelationLMM.cpp
  	@brief Cost function for least-square problems
		Implements a cost function using the interface provided by
		the LeastSquareProblem class.
 */

#include "LAMathLeastSquareCorrelationLMM.h"
#include "LAQuantLibCostFunction.h"

LAMathLeastSquareCorrelationLMM::LAMathLeastSquareCorrelationLMM(LAMathCorrelationLMMDiscAngle& cor_)
{
	mCostFunction = std::make_shared<LAMathLeastSquareCorrelationLMMQlib>(cor_);
}

LAMathLeastSquareCorrelationLMM::LAMathLeastSquareCorrelationLMM( const LAMathLeastSquareCorrelationLMM& rhs )
{
	mCostFunction = std::make_shared<LAMathLeastSquareCorrelationLMMQlib>(*(dynamic_pointer_cast<LAMathLeastSquareCorrelationLMMQlib>(rhs.mCostFunction)));
}

LAMathLeastSquareCorrelationLMM::~LAMathLeastSquareCorrelationLMM()
{
}

LAMathLeastSquareCorrelationLMM& LAMathLeastSquareCorrelationLMM::operator =(const LAMathLeastSquareCorrelationLMM& rhs)
{
    if ( this != &rhs )
    {
		*mCostFunction = *rhs.mCostFunction;
    }
    return *this;
}

