/*! @file LAMathLeastSquareCorrelationLMM.h 

	@brief Cost function for least-square problems
	Implements a cost function using the interface provided by
    the LeastSquareProblem class.
*/

#ifndef LAMathLeastSquareCorrelationLMM_h
#define LAMathLeastSquareCorrelationLMM_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAMathLeastSquareCorrelationLMM.h
//
//  SYNOPSIS    :       LAMathLeastSquareCorrelationLMM
//  DESCRIPTION :       Cost function for least-square problems
//
//  VERSION     :
////X///////////////////X///////////////////////////////X///////////////////

#include "LAQuantLibCostFunction.h"
#include "LAQuantLibDisposable.h"
#include "LAMathCorrelationLMMDiscAngle.h"
#include "LAMathLeastSquareCorrelationLMMQlib.h"

class LAMathLeastSquareCorrelationLMM : public LAQuantLibCostFunction
{
public:
    // Default constructor
    explicit LAMathLeastSquareCorrelationLMM( LAMathCorrelationLMMDiscAngle& cor_);

    // Copy constructor
    LAMathLeastSquareCorrelationLMM( const LAMathLeastSquareCorrelationLMM& rhs );

    // Destructor
    virtual ~LAMathLeastSquareCorrelationLMM();

    virtual LAMathLeastSquareCorrelationLMM* clone() const { return new LAMathLeastSquareCorrelationLMM(*this); }

    //
    LAMathLeastSquareCorrelationLMM& operator =(const LAMathLeastSquareCorrelationLMM& rhs);

};

#endif

