// Cost function for least-square problems
/*! Implements a cost function using the interface provided by
    the LeastSquareProblem class.
*/

#ifndef AQLMathLeastSquareSwaptionCapFloor_h
#define AQLMathLeastSquareSwaptionCapFloor_h

#include "AQLQuantLibCostFunction.h"
#include "AQLCoreTemplateType.h"

class AQLMathLeastSquareSwaptionVol;
class AQLMathLeastSquareCapFloor;

class AQLMathLeastSquareSwaptionCapFloor : public AQLQuantLibCostFunction
{
public:
    // Default constructor
    explicit AQLMathLeastSquareSwaptionCapFloor(AQLMathLeastSquareSwaptionVol& swaption_vol_,
											 AQLMathLeastSquareCapFloor& capfloor_,
											 const DoubleVector& weight_ = DoubleVector(2, 1.0)
			);


    // Copy constructor
    AQLMathLeastSquareSwaptionCapFloor( const AQLMathLeastSquareSwaptionCapFloor& rhs );

    // Destructor
    virtual ~AQLMathLeastSquareSwaptionCapFloor();

    virtual AQLMathLeastSquareSwaptionCapFloor* clone() const;

    //
    virtual AQLMathLeastSquareSwaptionCapFloor& operator =(const AQLMathLeastSquareSwaptionCapFloor& rhs);

    //
    virtual void setWeight( const DoubleVector& weight_);

};

#endif
