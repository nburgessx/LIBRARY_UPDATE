// Cost function for least-square problems
/*! Implements a cost function using the interface provided by
    the LeastSquareProblem class.
*/

#ifndef AQLMathLeastSquareBlackDDQlib_h
#define AQLMathLeastSquareBlackDDQlib_h

//#include <config.h>

#include <ql/qldefines.hpp>
//#include <ql/auto_link.hpp>
#include <ql/math/array.hpp>
#include <ql/math/optimization/costfunction.hpp>
#include <ql/math/optimization/constraint.hpp>

#include "AQLCoreTemplateType.h"
#include "AQLMathAnalyticalFormula.h"
#include "AQLModelDynamicsCurve.h"

//class CostFunction;

class AQLMathLeastSquareBlackDDQlib : public QuantLib::CostFunction
{
public:
    // Default constructor
    AQLMathLeastSquareBlackDDQlib(double s0,
                        AQLRatesPathElementCurve& curve0_d,
                        AQLRatesPathElementCurve& curve0_f,
                        const DoubleArray& t,
                        const DoubleMatrix& strike,
                        const DoubleMatrix& prem,
                        const DoubleMatrix& weight,
                        const vector<IntArray>& callPutFlg,
                        bool isCalibVol = true,
                        bool isCalibSkew = true
                        );

    // Copy constructor
    AQLMathLeastSquareBlackDDQlib(const AQLMathLeastSquareBlackDDQlib& rhs);   

    // Destructor
    virtual ~AQLMathLeastSquareBlackDDQlib();

    virtual AQLMathLeastSquareBlackDDQlib* clone() const { return new AQLMathLeastSquareBlackDDQlib(*this); }

    // compute value of the least square function
    virtual QuantLib::Real value(const QuantLib::Array& x) const;

    // compute value of the least square function
    virtual QuantLib::Array values(const QuantLib::Array& x) const;

	// compute the values of black volatility
    virtual DoubleArray getBlackVol(const DoubleArray& x) const;
    //virtual void gradient(QuantLib::Array& grad, const QuantLib::Array& x) const { grad = x; }

    //
    AQLMathLeastSquareBlackDDQlib& operator =(const AQLMathLeastSquareBlackDDQlib& rhs);
    
 
protected:

    DoubleArray mT;       // T
    DoubleMatrix mStrike; // Strike
    DoubleMatrix mPrem;   // Premium
	DoubleArray mWeight;  // Weight
    vector<IntArray> mCallPutFlg; // CallPutFlag

    bool mIsCalibVol;    // isCalibVol
    bool mIsCalibSkew;   // isCalibSkew

    mutable bool mInit; // initFlg
    mutable DoubleArray mMktSkew;  // MktSkew
    mutable DoubleArray mMktSigma; // MktSigma

    DoubleArray mFX0;  // FX0
    AQLRatesPathElementCurve* mCurve0_d; // curve0

    mutable DoubleArray mFct2Fit; // factors to fit

private:
    
    // compute the values of the function to fit
    virtual void getTarget2Fit() const;

    // compute the values of the function to fit
    virtual void getFct2Fit(const QuantLib::Array& x) const;

    // target correlation to fit
    mutable DoubleArray mTarget2Fit;
    
    bool mCloned; // clonedflg  
};

//
//----------------------------------------------------------------------------------------
//

//
class AQLMathLeastSquareBlackDDVolQlib : public AQLMathLeastSquareBlackDDQlib
{
public:
    // Default constructor
    AQLMathLeastSquareBlackDDVolQlib(double s0,
                            AQLRatesPathElementCurve& curve0_d,
                            AQLRatesPathElementCurve& curve0_f,
                            const DoubleArray& t,
                            const DoubleMatrix& strike,
                            const DoubleMatrix& prem,
                            const DoubleMatrix& weight,
                            const vector<IntVector>& callPutFlg
                           );

    // Copy constructor
    AQLMathLeastSquareBlackDDVolQlib( const AQLMathLeastSquareBlackDDVolQlib& rhs );

    // Destructor
    virtual ~AQLMathLeastSquareBlackDDVolQlib();

    virtual AQLMathLeastSquareBlackDDVolQlib* clone() const { return new AQLMathLeastSquareBlackDDVolQlib(*this); }

    //
    AQLMathLeastSquareBlackDDVolQlib& operator =(const AQLMathLeastSquareBlackDDVolQlib& rhs);
    
 
protected:
private:
    
    // compute the values of the function to fit
    virtual void getFct2Fit(const QuantLib::Array& x) const;

    bool mCloned; // clonedflg
};

//
//----------------------------------------------------------------------------------------
//

//
class AQLMathLeastSquareBlackDDSkewQlib : public AQLMathLeastSquareBlackDDQlib
{
public:
    // Default constructor
    AQLMathLeastSquareBlackDDSkewQlib(double s0,
                             AQLRatesPathElementCurve& curve0_d,
                             AQLRatesPathElementCurve& curve0_f,
                             const DoubleArray& t,
                             const DoubleMatrix& strike,
                             const DoubleMatrix& prem,
                             const DoubleMatrix& weight,
                             const DoubleArray& impVol,
                             const vector<IntVector>& callPutFlg
                            );

    // Copy constructor
    AQLMathLeastSquareBlackDDSkewQlib( const AQLMathLeastSquareBlackDDSkewQlib& rhs );

    // Destructor
    virtual ~AQLMathLeastSquareBlackDDSkewQlib();

    //
    virtual AQLMathLeastSquareBlackDDSkewQlib* clone() const { return new AQLMathLeastSquareBlackDDSkewQlib(*this); }

    //
    AQLMathLeastSquareBlackDDSkewQlib& operator =(const AQLMathLeastSquareBlackDDSkewQlib& rhs);
    
    //
    void setImpVol(const DoubleArray& impVol);

protected:
private:
    // compute the values of the function to fit
    virtual void getFct2Fit(const QuantLib::Array& x) const;

    bool mCloned; // clonedflg
};

//
//----------------------------------------------------------------------------------------
//

// %Constraint of Mkt Skew and Vol
class AQLMathBoundaryConstraintMktSkewVolQLib : public QuantLib::Constraint
{
public:
    AQLMathBoundaryConstraintMktSkewVolQLib(double skewMax, double skewMin, double volMax, double volMin, size_t n = 2)
    :
    QuantLib::Constraint(boost::shared_ptr<Constraint::Impl>( new AQLMathBoundaryConstraintMktSkewVolQLib::Impl(skewMax, skewMin, volMax, volMin, n) ) )
    {
    }

private:
    class Impl : public Constraint::Impl
    {
    public:
        Impl(double skewMax, double skewMin, double volMax, double volMin, size_t n)
        :
        mSkewMax(skewMax),
        mSkewMin(skewMin),
        mVolMax(volMax),
        mVolMin(volMin),
        mSize(n)
        {
        }
        
        bool test(const QuantLib::Array& params) const
        {
            if( params.size() != mSize) throw AQLCoreInvalidData("params.size() != mSize : AQLMathBoundaryConstraintMktSkewVolQLib::Impl::test", __FILE__, __LINE__);            
            if( mSize % 2 != 0 ) throw AQLCoreInvalidData(" mSize % 2 != 0 : AQLMathBoundaryConstraintMktSkewVolQLib::Impl::test", __FILE__, __LINE__);

            for(size_t i = 0; i < static_cast<size_t>(mSize / 2); ++i)
            {
                if(params[i] < mVolMin ) return false;
                if(params[i] > mVolMax ) return false;
            }
            
            for(size_t i = static_cast<size_t>(mSize / 2); i < mSize; ++i)
            {
                if(params[i] < mSkewMin ) return false;
                if(params[i] > mSkewMax) return false;
            }
            return true;
        }

    private:
        QuantLib::Real mSkewMax, mSkewMin, mVolMax, mVolMin;
        size_t mSize;
    };
};
#endif
