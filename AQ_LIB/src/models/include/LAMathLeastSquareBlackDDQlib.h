// Cost function for least-square problems
/*! Implements a cost function using the interface provided by
    the LeastSquareProblem class.
*/

#ifndef LAMathLeastSquareBlackDDQlib_h
#define LAMathLeastSquareBlackDDQlib_h

//#include <config.h>

#include <ql/qldefines.hpp>
//#include <ql/auto_link.hpp>
#include <ql/math/array.hpp>
#include <ql/math/optimization/costfunction.hpp>
#include <ql/math/optimization/constraint.hpp>

#include "LACoreTemplateType.h"
#include "LAMathAnalyticalFormula.h"
#include "LAModelDynamicsCurve.h"

//class CostFunction;

class LAMathLeastSquareBlackDDQlib : public QuantLib::CostFunction
{
public:
    // Default constructor
    LAMathLeastSquareBlackDDQlib(double s0,
                        LARatesPathElementCurve& curve0_d,
                        LARatesPathElementCurve& curve0_f,
                        const DoubleArray& t,
                        const DoubleMatrix& strike,
                        const DoubleMatrix& prem,
                        const DoubleMatrix& weight,
                        const vector<IntArray>& callPutFlg,
                        bool isCalibVol = true,
                        bool isCalibSkew = true
                        );

    // Copy constructor
    LAMathLeastSquareBlackDDQlib(const LAMathLeastSquareBlackDDQlib& rhs);   

    // Destructor
    virtual ~LAMathLeastSquareBlackDDQlib();

    virtual LAMathLeastSquareBlackDDQlib* clone() const { return new LAMathLeastSquareBlackDDQlib(*this); }

    // compute value of the least square function
    virtual QuantLib::Real value(const QuantLib::Array& x) const;

    // compute value of the least square function
    virtual QuantLib::Array values(const QuantLib::Array& x) const;

	// compute the values of black volatility
    virtual DoubleArray getBlackVol(const DoubleArray& x) const;
    //virtual void gradient(QuantLib::Array& grad, const QuantLib::Array& x) const { grad = x; }

    //
    LAMathLeastSquareBlackDDQlib& operator =(const LAMathLeastSquareBlackDDQlib& rhs);
    
 
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
    LARatesPathElementCurve* mCurve0_d; // curve0

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
class LAMathLeastSquareBlackDDVolQlib : public LAMathLeastSquareBlackDDQlib
{
public:
    // Default constructor
    LAMathLeastSquareBlackDDVolQlib(double s0,
                            LARatesPathElementCurve& curve0_d,
                            LARatesPathElementCurve& curve0_f,
                            const DoubleArray& t,
                            const DoubleMatrix& strike,
                            const DoubleMatrix& prem,
                            const DoubleMatrix& weight,
                            const vector<IntVector>& callPutFlg
                           );

    // Copy constructor
    LAMathLeastSquareBlackDDVolQlib( const LAMathLeastSquareBlackDDVolQlib& rhs );

    // Destructor
    virtual ~LAMathLeastSquareBlackDDVolQlib();

    virtual LAMathLeastSquareBlackDDVolQlib* clone() const { return new LAMathLeastSquareBlackDDVolQlib(*this); }

    //
    LAMathLeastSquareBlackDDVolQlib& operator =(const LAMathLeastSquareBlackDDVolQlib& rhs);
    
 
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
class LAMathLeastSquareBlackDDSkewQlib : public LAMathLeastSquareBlackDDQlib
{
public:
    // Default constructor
    LAMathLeastSquareBlackDDSkewQlib(double s0,
                             LARatesPathElementCurve& curve0_d,
                             LARatesPathElementCurve& curve0_f,
                             const DoubleArray& t,
                             const DoubleMatrix& strike,
                             const DoubleMatrix& prem,
                             const DoubleMatrix& weight,
                             const DoubleArray& impVol,
                             const vector<IntVector>& callPutFlg
                            );

    // Copy constructor
    LAMathLeastSquareBlackDDSkewQlib( const LAMathLeastSquareBlackDDSkewQlib& rhs );

    // Destructor
    virtual ~LAMathLeastSquareBlackDDSkewQlib();

    //
    virtual LAMathLeastSquareBlackDDSkewQlib* clone() const { return new LAMathLeastSquareBlackDDSkewQlib(*this); }

    //
    LAMathLeastSquareBlackDDSkewQlib& operator =(const LAMathLeastSquareBlackDDSkewQlib& rhs);
    
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
class LAMathBoundaryConstraintMktSkewVolQLib : public QuantLib::Constraint
{
public:
    LAMathBoundaryConstraintMktSkewVolQLib(double skewMax, double skewMin, double volMax, double volMin, size_t n = 2)
    :
    QuantLib::Constraint(boost::shared_ptr<Constraint::Impl>( new LAMathBoundaryConstraintMktSkewVolQLib::Impl(skewMax, skewMin, volMax, volMin, n) ) )
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
            if( params.size() != mSize) throw LACoreInvalidData("params.size() != mSize : LAMathBoundaryConstraintMktSkewVolQLib::Impl::test", __FILE__, __LINE__);            
            if( mSize % 2 != 0 ) throw LACoreInvalidData(" mSize % 2 != 0 : LAMathBoundaryConstraintMktSkewVolQLib::Impl::test", __FILE__, __LINE__);

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
