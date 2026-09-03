/*! @file AQLMathLeastSquareCorrelationLMM.cpp
  	@brief Cost function for least-square problems
		Implements a cost function using the interface provided by
		the LeastSquareProblem class.
 */

#include "AQLMathLeastSquareCorrelationLMM.h"
#include "AQLQuantLibCostFunction.h"

AQLMathLeastSquareCorrelationLMM::AQLMathLeastSquareCorrelationLMM(AQLMathCorrelationLMMDiscAngle& cor_)
{
	mCostFunction = std::make_shared<AQLMathLeastSquareCorrelationLMMQlib>(cor_);
}

AQLMathLeastSquareCorrelationLMM::AQLMathLeastSquareCorrelationLMM( const AQLMathLeastSquareCorrelationLMM& rhs )
{
	mCostFunction = std::make_shared<AQLMathLeastSquareCorrelationLMMQlib>(*(dynamic_pointer_cast<AQLMathLeastSquareCorrelationLMMQlib>(rhs.mCostFunction)));
}

AQLMathLeastSquareCorrelationLMM::~AQLMathLeastSquareCorrelationLMM()
{
}

AQLMathLeastSquareCorrelationLMM& AQLMathLeastSquareCorrelationLMM::operator =(const AQLMathLeastSquareCorrelationLMM& rhs)
{
    if ( this != &rhs )
    {
		*mCostFunction = *rhs.mCostFunction;
    }
    return *this;
}

