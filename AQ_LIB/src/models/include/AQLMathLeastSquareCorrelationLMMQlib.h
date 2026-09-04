/*! @file AQLMathLeastSquareCorrelationLMMQlib.h 

	@brief Cost function for least-square problems with QuantLib
	Implements a cost function using the interface provided by
    the LeastSquareProblem class.
*/

#ifndef AQLMathLeastSquareCorrelationLMMQlib_h
#define AQLMathLeastSquareCorrelationLMMQlib_h

#include <ql/math/optimization/costfunction.hpp>
#include <ql/math/array.hpp>
#include "AQLQuantLibDisposable.h"
#include "AQLMathCorrelationLMMDiscAngle.h"

class AQLMathLeastSquareCorrelationLMMQlib : public QuantLib::CostFunction
{
public:
    // Default constructor
    explicit AQLMathLeastSquareCorrelationLMMQlib(AQLMathCorrelationLMMDiscAngle& cor_);

    // Copy constructor
    AQLMathLeastSquareCorrelationLMMQlib(const AQLMathLeastSquareCorrelationLMMQlib& rhs);

    // Destructor
    ~AQLMathLeastSquareCorrelationLMMQlib();

    AQLMathLeastSquareCorrelationLMMQlib* clone() const { return new AQLMathLeastSquareCorrelationLMMQlib(*this); }

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
    AQLMathLeastSquareCorrelationLMMQlib& operator =(const AQLMathLeastSquareCorrelationLMMQlib& rhs);


private:

    // target correlation to fit
    DoubleMatrix mCorTarget;

    // target correlation size
    size_t mn; 

    //
    mutable AQLMathCorrelationLMMDiscAngle* mCor;

    // rank of factor_loading
    size_t mNoOfFactors;

    // factors to fit
    /* This Matrix is made up by cor->Get(0.0, i, j). */
    mutable DoubleMatrix mfct2fit;

    bool mCloned;
};

#endif

