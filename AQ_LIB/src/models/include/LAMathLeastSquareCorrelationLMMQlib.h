/*! @file LAMathLeastSquareCorrelationLMMQlib.h 

	@brief Cost function for least-square problems with QuantLib
	Implements a cost function using the interface provided by
    the LeastSquareProblem class.
*/

#ifndef LAMathLeastSquareCorrelationLMMQlib_h
#define LAMathLeastSquareCorrelationLMMQlib_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAMathLeastSquareCorrelationLMMQlib.h
//
//  SYNOPSIS    :       LAMathLeastSquareCorrelationLMMQlib
//  DESCRIPTION :       Cost function for least-square problems with QuantLib
//
//  VERSION     :
////X///////////////////X///////////////////////////////X///////////////////

#include <ql/math/optimization/costfunction.hpp>
#include <ql/math/array.hpp>
#include "LAQuantLibDisposable.h"
#include "LAMathCorrelationLMMDiscAngle.h"

class LAMathLeastSquareCorrelationLMMQlib : public QuantLib::CostFunction
{
public:
    // Default constructor
    explicit LAMathLeastSquareCorrelationLMMQlib(LAMathCorrelationLMMDiscAngle& cor_);

    // Copy constructor
    LAMathLeastSquareCorrelationLMMQlib(const LAMathLeastSquareCorrelationLMMQlib& rhs);

    // Destructor
    ~LAMathLeastSquareCorrelationLMMQlib();

    LAMathLeastSquareCorrelationLMMQlib* clone() const { return new LAMathLeastSquareCorrelationLMMQlib(*this); }

    // compute value of the least square function
    virtual double value(const QuantLib::Array& x) const;

    // compute value of the least square function
    virtual QuantLib::Array values(const QuantLib::Array& x) const;

    //virtual void gradient(DoubleVector& grad, const QuantLib::Array& x) const { grad = x; }

    // compute vector of derivatives of the least square function
    virtual void gradient(QuantLib::Array& grad_f, const QuantLib::Array& x) const;

    // compute the values of the function to fit
    virtual void getfct2fit(const QuantLib::Array& x) const;

    //
    LAMathLeastSquareCorrelationLMMQlib& operator =(const LAMathLeastSquareCorrelationLMMQlib& rhs);


private:

    // target correlation to fit
    DoubleMatrix mCorTarget;

    // target correlation size
    size_t mn; 

    //
    mutable LAMathCorrelationLMMDiscAngle* mCor;

    // rank of factor_loading
    size_t mNoOfFactors;

    // factors to fit
    /* This Matrix is made up by cor->Get(0.0, i, j). */
    mutable DoubleMatrix mfct2fit;

    bool mCloned;
};

#endif

