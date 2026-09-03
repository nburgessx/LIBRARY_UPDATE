// Cost function for least-square problems
/*! Implements a cost function using the interface provided by
    the LeastSquareProblem class.
*/

#ifndef AQLMathLeastSquareSwaptionCapFloorQlib_h
#define AQLMathLeastSquareSwaptionCapFloorQlib_h

#include <ql/math/optimization/costfunction.hpp>
#include <ql/math/array.hpp>
#include "AQLCoreTemplateType.h"

class AQLMathLeastSquareSwaptionVolQlib;
class AQLMathLeastSquareCapFloorQlib;

class AQLMathLeastSquareSwaptionCapFloorQlib : public QuantLib::CostFunction
{
public:
    // Default constructor
    explicit AQLMathLeastSquareSwaptionCapFloorQlib(AQLMathLeastSquareSwaptionVolQlib& swaption_vol_,
												 AQLMathLeastSquareCapFloorQlib& capfloor_,
												 const DoubleVector& weight_ = DoubleVector(2, 1.0)
			);
   

    // Copy constructor
    AQLMathLeastSquareSwaptionCapFloorQlib( const AQLMathLeastSquareSwaptionCapFloorQlib& rhs );   

    // Destructor
    virtual ~AQLMathLeastSquareSwaptionCapFloorQlib();

    virtual AQLMathLeastSquareSwaptionCapFloorQlib* clone() const;

    // compute value of the least square function
    virtual double value(const QuantLib::Array& x) const;

    // compute value of the least square function
    virtual QuantLib::Array values(const QuantLib::Array& x) const;

    //
    virtual AQLMathLeastSquareSwaptionCapFloorQlib& operator =(const AQLMathLeastSquareSwaptionCapFloorQlib& rhs);
    
    //
    virtual void setWeight( const DoubleVector& weight_) { mWeight = weight_; }
 
protected:
    
private:

    // factors to fit
    mutable DoubleMatrix mfct2fit;

    // weight
    DoubleVector mWeight;

    //
    AQLMathLeastSquareCapFloorQlib* mCapfloor;

    AQLMathLeastSquareSwaptionVolQlib* mSwaptionVol;

    //
    bool mCloned;

};

#endif
