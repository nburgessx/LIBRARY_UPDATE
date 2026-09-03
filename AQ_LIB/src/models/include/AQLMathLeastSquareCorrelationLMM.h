/*! @file AQLMathLeastSquareCorrelationLMM.h 

	@brief Cost function for least-square problems
	Implements a cost function using the interface provided by
    the LeastSquareProblem class.
*/

#ifndef AQLMathLeastSquareCorrelationLMM_h
#define AQLMathLeastSquareCorrelationLMM_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLMathLeastSquareCorrelationLMM.h
//
//  SYNOPSIS    :       AQLMathLeastSquareCorrelationLMM
//  DESCRIPTION :       Cost function for least-square problems
//
//  VERSION     :
////X///////////////////X///////////////////////////////X///////////////////

#include "AQLQuantLibCostFunction.h"
#include "AQLQuantLibDisposable.h"
#include "AQLMathCorrelationLMMDiscAngle.h"
#include "AQLMathLeastSquareCorrelationLMMQlib.h"

class AQLMathLeastSquareCorrelationLMM : public AQLQuantLibCostFunction
{
public:
    // Default constructor
    explicit AQLMathLeastSquareCorrelationLMM( AQLMathCorrelationLMMDiscAngle& cor_);

    // Copy constructor
    AQLMathLeastSquareCorrelationLMM( const AQLMathLeastSquareCorrelationLMM& rhs );

    // Destructor
    virtual ~AQLMathLeastSquareCorrelationLMM();

    virtual AQLMathLeastSquareCorrelationLMM* clone() const { return new AQLMathLeastSquareCorrelationLMM(*this); }

    //
    AQLMathLeastSquareCorrelationLMM& operator =(const AQLMathLeastSquareCorrelationLMM& rhs);

};

#endif

