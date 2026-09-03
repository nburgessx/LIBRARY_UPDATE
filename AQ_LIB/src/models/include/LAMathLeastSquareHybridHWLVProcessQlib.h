// Cost function for least-square problems
/*! Implements a cost function using the interface provided by
    the LeastSquareProblem class.
*/
// 2012, AlgoQuantHub.
///
#ifndef LAMathLeastSquareHybridHWLVProcessQlib_h
#define LAMathLeastSquareHybridHWLVProcessQlib_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAMathLeastSquareHybridHWLVProcessQlib.h
//
//  DESCRIPTION :      
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////

#include <AQLCoreUtil.h>
#include <vector>
#include <map>
#include "AQLFunction.h"
#include <limits>

#include <ql/qldefines.hpp>
//#include <ql/auto_link.hpp>
#include <ql/math/array.hpp>
#include <ql/math/optimization/costfunction.hpp>
#include <ql/math/optimization/constraint.hpp>


#include "LAModelDynamicsCurve.h"
#include "LAMathHybridHWLVUtil.h"


/*! 
    @brief declaration of struct
*/
struct LAMathTarget2fit1
{
public:
    LAMathTarget2fit1(const DoubleArray& x,
                const DoubleArray& y,
                const DoubleArray& target2fit
                ) : mX(x), mY(y), mTarget2fit(target2fit) {}
    
    DoubleArray mX;
    DoubleArray mY;
    DoubleArray mTarget2fit;
};

/*! 
    @brief declaration of struct
*/
struct LAMathTarget2fit2
{
public:
    LAMathTarget2fit2(const DoubleArray& x,
                const DoubleMatrix& y,
                const DoubleMatrix& target2fit,
                const DoubleMatrix& weight
               )
    :
    mX(x),
    mY(y),
    mWeight(weight),
    mTarget2fit(target2fit)
    {
        size_t n = mX.size();
        size_t m = mY[0].size();
    
        if(n != mY.size()) throw AQLCoreInvalidData("x.size() != y.size() : LAMathTarget2fit2::LAMathTarget2fit2", __FILE__, __LINE__);
        if(n != mTarget2fit.size()) throw AQLCoreInvalidData("x.size() != target2fit.size() : LAMathTarget2fit2::LAMathTarget2fit2", __FILE__, __LINE__);
        if(n != mWeight.size()) throw AQLCoreInvalidData("x.size() != weight.size() : LAMathTarget2fit2::LAMathTarget2fit2", __FILE__, __LINE__);
    
        if(m != mTarget2fit[0].size()) throw AQLCoreInvalidData("y[0].size() != target2fit[0].size() : LAMathTarget2fit2::LAMathTarget2fit2", __FILE__, __LINE__);
        if(m != mWeight[0].size()) throw AQLCoreInvalidData("y[0].size() != weight[0].size() : Target2fit2::Target2fit2", __FILE__, __LINE__);
    }

    DoubleArray mX;
    DoubleMatrix mY;
    DoubleMatrix mWeight;
    DoubleMatrix mTarget2fit;
};

/*! 
    @brief declaration of class
*/
class LAMathLeastSquareHybridHWLVProcessQlib : public QuantLib::CostFunction
{
public:

    enum mode { Diff, Ratio, Sqrt_Ratio };

    //
    LAMathLeastSquareHybridHWLVProcessQlib() {}

    LAMathLeastSquareHybridHWLVProcessQlib(const DoubleArray& fx0,
                                     std::vector<LAMathHybridHWLVProcessHelper*>& processHelper,
                                     LAMathTarget2fit2& t2f,
                                     mode mode_ = Diff
                                    );

    // Copy constructor
    LAMathLeastSquareHybridHWLVProcessQlib( const LAMathLeastSquareHybridHWLVProcessQlib& rhs );   

    // Destructor
    virtual ~LAMathLeastSquareHybridHWLVProcessQlib();

    // compute value of the least square function
    virtual QuantLib::Real value(const QuantLib::Array& x) const;

    // compute value of the least square function
    virtual QuantLib::Array values(const QuantLib::Array& x) const;

    //virtual void gradient(Array& grad, const Array& x) const { grad = x; }

    //
    LAMathLeastSquareHybridHWLVProcessQlib& operator =(const LAMathLeastSquareHybridHWLVProcessQlib& rhs);
    
 
protected:
    // compute the values of the function to fit
    virtual void getTarget2fit() const;

    // compute the values of the function to fit
	virtual void getFct2fit(const QuantLib::Array& x) const = 0;

    // target to fit
    LAMathTarget2fit2* mpT2f;
    mutable DoubleArray mTarget2fit;
    mutable DoubleArray mWeight;

    // factors to fit
    mutable DoubleArray mFct2fit;

    //
    DoubleArray mFX0;

    // setUp
    void setUp(const DoubleArray& fx0,
               std::vector<LAMathHybridHWLVProcessHelper*>& processHelper,
               LAMathTarget2fit2& t2f,
               mode mode_
              );
   
   size_t searchIdx(double x, const DoubleArray& y) const
   {
	   //size_t i = lower_bound(y.begin(), y.end(), x) - y.begin();
        size_t i = upper_bound(y.begin(), y.end(), x) - y.begin();
        if( i != 0 && i < y.size() )
        {
            i = eq(x, y[i], 1. / 367. ) ? i : --i;
        }
        return i;
   }

private:

    // Hull-White LV Process Helper
    std::vector<LAMathHybridHWLVProcessHelper*> mProcessHelper;
    
    //
    mode mMode;


    //
    bool mCloned;
};

/*! 
    @brief declaration of class
*/
class LAMathLeastSquareHybridHWDDProcessQlib : public LAMathLeastSquareHybridHWLVProcessQlib
{
public:
    LAMathLeastSquareHybridHWDDProcessQlib(const DoubleArray& fx0,
                                     std::vector<LAMathHybridHWDDProcessHelper*>& processHelper,
                                     LAMathTarget2fit2& t2f,
                                     mode mode_ = Diff
                                    );

    // Copy constructor
    LAMathLeastSquareHybridHWDDProcessQlib( const LAMathLeastSquareHybridHWDDProcessQlib& rhs );   

    // Destructor
    virtual ~LAMathLeastSquareHybridHWDDProcessQlib();
    //
    LAMathLeastSquareHybridHWDDProcessQlib& operator = (const LAMathLeastSquareHybridHWDDProcessQlib& rhs);
    
 
protected:
    // compute the values of the function to fit
    virtual void getFct2fit(const QuantLib::Array& x) const = 0;

    void setSigma(size_t idx, double sigma) const
    {
        for(size_t i = 0; i < mProcessHelper.size(); ++i)
        {
            mProcessHelper[i]->setSigma(mProcessHelper[i]->getDDTimeStep()[idx], sigma);
        }
    }

    void setBeta(size_t idx, double beta) const
    {
        for(size_t i = 0; i < mProcessHelper.size(); ++i)
        {
            mProcessHelper[i]->setBeta(mProcessHelper[i]->getDDTimeStep()[idx], beta);
        }
    }

    // Hull-White LV Process Helper
    mutable std::vector<LAMathHybridHWDDProcessHelper*> mProcessHelper;

private:
    
    bool mCloned;
};

/*! 
    @brief declaration of class
*/
class LAMathLeastSquareHybridHWDDProcess1Qlib : public LAMathLeastSquareHybridHWDDProcessQlib
{
public:
    LAMathLeastSquareHybridHWDDProcess1Qlib(const DoubleArray& fx0,
                                      std::vector<LAMathHybridHWDDProcessHelper*>& processHelper,
                                      LAMathTarget2fit2& vol2fit,
                                      LAMathTarget2fit2& skew2fit,
                                      DoubleArray weightVolSkew, //{vol_weight, skew_weight}
                                      mode mode_ = Diff
                                     );

    // Copy constructor
    LAMathLeastSquareHybridHWDDProcess1Qlib( const LAMathLeastSquareHybridHWDDProcess1Qlib& rhs );   

    // Destructor
    virtual ~LAMathLeastSquareHybridHWDDProcess1Qlib();

    // compute value of the least square function
    virtual QuantLib::Real value(const QuantLib::Array& x) const;

    // compute value of the least square function
	virtual QuantLib::Array values(const QuantLib::Array& x) const;

    //virtual void gradient(Array& grad, const Array& x) const { grad = x; }

    //
    LAMathLeastSquareHybridHWDDProcess1Qlib& operator = (const LAMathLeastSquareHybridHWDDProcess1Qlib& rhs);
    
 
protected:
    // compute the values of the function to fit
    virtual void getTarget2fit() const;

	virtual void getFct2fit(const QuantLib::Array& x) const;

private:
    
    // Target to fit LAMathTarget2fit1 = {Skew} and LAMathTarget2fit2 = {Vol} 
    LAMathTarget2fit2* mpT2f2;
    mutable DoubleArray mTarget2fit2;
    DoubleArray mWeightVolSkew;
    
    // Weight between Mkt Vol and Skew
    mutable DoubleArray mWeight2;
    mutable DoubleArray mFct2fit2;
    
    bool mCloned;
};

/*! 
    @brief declaration of class
*/
class LAMathLeastSquareHybridHWDDProcess2Qlib : public LAMathLeastSquareHybridHWDDProcessQlib
{
public:
    LAMathLeastSquareHybridHWDDProcess2Qlib(const DoubleArray& fx0,
                                      std::vector<LAMathHybridHWDDProcessHelper*>& processHelper,
                                      LAMathTarget2fit2& vol2fit,
                                      double ts = 0.,
                                      mode mode_ = Diff
                                     );

    // Copy constructor
    LAMathLeastSquareHybridHWDDProcess2Qlib( const LAMathLeastSquareHybridHWDDProcess2Qlib& rhs );   

    // Destructor
    virtual ~LAMathLeastSquareHybridHWDDProcess2Qlib();

    //
    LAMathLeastSquareHybridHWDDProcess2Qlib& operator = (const LAMathLeastSquareHybridHWDDProcess2Qlib& rhs);
    
 
protected:

    // compute the values of the function to fit
    virtual void getFct2fit(const QuantLib::Array& x) const;

private:
            
    double mTs;
    bool mCloned;
};

/*! 
    @brief declaration of class
*/
class LAMathLeastSquareHybridHWDDProcess3Qlib : public LAMathLeastSquareHybridHWDDProcessQlib
{
public:
    LAMathLeastSquareHybridHWDDProcess3Qlib(const DoubleArray& fx0,
                                      std::vector<LAMathHybridHWDDProcessHelper*>& processHelper,
                                      LAMathTarget2fit2& skew2fit,
                                      double ts = 0.,
                                      mode mode_ = Diff
                                     );

    // Copy constructor
    LAMathLeastSquareHybridHWDDProcess3Qlib( const LAMathLeastSquareHybridHWDDProcess3Qlib& rhs );   

    // Destructor
    virtual ~LAMathLeastSquareHybridHWDDProcess3Qlib();

    //
    LAMathLeastSquareHybridHWDDProcess3Qlib& operator = (const LAMathLeastSquareHybridHWDDProcess3Qlib& rhs);
    
 
protected:
    // compute the values of the function to fit
    virtual void getFct2fit(const QuantLib::Array& x) const;

    // method to overload to compute grad_f, the first derivative of
    //  the cost function with respect to x
    //virtual void gradient(Array& grad, const Array& x) const;

private:
    
    double mTs;
    bool mCloned;
};

/*! 
    @brief declaration of class
*/

// %Constraint of Mkt Skew and Vol
class LAMathBoundaryConstraintHybridHWLVProcess: public QuantLib::Constraint
{
public:
    LAMathBoundaryConstraintHybridHWLVProcess(double sigmaMax, double sigmaMin, double betaMax, double betaMin, size_t n = 2)
    :
	QuantLib::Constraint(boost::shared_ptr<Constraint::Impl>( new LAMathBoundaryConstraintHybridHWLVProcess::Impl(sigmaMax, sigmaMin, betaMax, betaMin, n) ) )
    {
    }

private:
    class Impl : public QuantLib::Constraint::Impl
    {
    public:
        Impl(double sigmaMax, double sigmaMin, double betaMax, double betaMin, size_t n_)
        :
        mSigmaMax(sigmaMax),
        mSigmaMin(sigmaMin),
        mBetaMax(betaMax),
        mBetaMin(betaMin),
        mN(n_)
        {
        }
        
        bool test(const QuantLib::Array& params) const
        {
            if( params.size() != mN) throw AQLCoreInvalidData("params.size() != n : BoundaryConstraint_MktSkewVol::Impl::test", __FILE__, __LINE__);            
            if( mN % 2 != 0 ) throw AQLCoreInvalidData(" n % 2 != 0 : BoundaryConstraint_MktSkewVol::Impl::test", __FILE__, __LINE__);

            for(size_t i = 0; i < static_cast<size_t>(mN / 2); ++i)
            {
                if(params[i] < mSigmaMin ) return false;
                if(params[i] > mSigmaMax ) return false;
            }
            
            for(size_t i = static_cast<size_t>(mN / 2); i < mN; ++i)
            {
                if(params[i] < mBetaMin ) return false;
                if(params[i] > mBetaMax) return false;
            }
            return true;
        }

    private:
        QuantLib::Real mSigmaMax, mSigmaMin, mBetaMax, mBetaMin;
        size_t mN;
    };
};

#endif
