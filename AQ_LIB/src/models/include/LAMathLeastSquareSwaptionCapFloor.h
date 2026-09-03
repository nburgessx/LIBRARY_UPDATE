// Cost function for least-square problems
/*! Implements a cost function using the interface provided by
    the LeastSquareProblem class.
*/

#ifndef LAMathLeastSquareSwaptionCapFloor_h
#define LAMathLeastSquareSwaptionCapFloor_h

#include "LAQuantLibCostFunction.h"
#include "AQLCoreTemplateType.h"

class LAMathLeastSquareSwaptionVol;
class LAMathLeastSquareCapFloor;

class LAMathLeastSquareSwaptionCapFloor : public LAQuantLibCostFunction
{
public:
    // Default constructor
    explicit LAMathLeastSquareSwaptionCapFloor(LAMathLeastSquareSwaptionVol& swaption_vol_,
											 LAMathLeastSquareCapFloor& capfloor_,
											 const DoubleVector& weight_ = DoubleVector(2, 1.0)
			);


    // Copy constructor
    LAMathLeastSquareSwaptionCapFloor( const LAMathLeastSquareSwaptionCapFloor& rhs );

    // Destructor
    virtual ~LAMathLeastSquareSwaptionCapFloor();

    virtual LAMathLeastSquareSwaptionCapFloor* clone() const;

    //
    virtual LAMathLeastSquareSwaptionCapFloor& operator =(const LAMathLeastSquareSwaptionCapFloor& rhs);

    //
    virtual void setWeight( const DoubleVector& weight_);

};

#endif
