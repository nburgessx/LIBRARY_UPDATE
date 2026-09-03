// Cost function for least-square problems
/*! Implements a cost function using the interface provided by
    the LeastSquareProblem class.
*/

#ifndef LAMathLeastSquareSwaptionCapFloorQlib_h
#define LAMathLeastSquareSwaptionCapFloorQlib_h

#include <ql/math/optimization/costfunction.hpp>
#include <ql/math/array.hpp>
#include "AQLCoreTemplateType.h"

class LAMathLeastSquareSwaptionVolQlib;
class LAMathLeastSquareCapFloorQlib;

class LAMathLeastSquareSwaptionCapFloorQlib : public QuantLib::CostFunction
{
public:
    // Default constructor
    explicit LAMathLeastSquareSwaptionCapFloorQlib(LAMathLeastSquareSwaptionVolQlib& swaption_vol_,
												 LAMathLeastSquareCapFloorQlib& capfloor_,
												 const DoubleVector& weight_ = DoubleVector(2, 1.0)
			);
   

    // Copy constructor
    LAMathLeastSquareSwaptionCapFloorQlib( const LAMathLeastSquareSwaptionCapFloorQlib& rhs );   

    // Destructor
    virtual ~LAMathLeastSquareSwaptionCapFloorQlib();

    virtual LAMathLeastSquareSwaptionCapFloorQlib* clone() const;

    // compute value of the least square function
    virtual double value(const QuantLib::Array& x) const;

    // compute value of the least square function
    virtual QuantLib::Array values(const QuantLib::Array& x) const;

    //
    virtual LAMathLeastSquareSwaptionCapFloorQlib& operator =(const LAMathLeastSquareSwaptionCapFloorQlib& rhs);
    
    //
    virtual void setWeight( const DoubleVector& weight_) { mWeight = weight_; }
 
protected:
    
private:

    // factors to fit
    mutable DoubleMatrix mfct2fit;

    // weight
    DoubleVector mWeight;

    //
    LAMathLeastSquareCapFloorQlib* mCapfloor;

    LAMathLeastSquareSwaptionVolQlib* mSwaptionVol;

    //
    bool mCloned;

};

#endif
