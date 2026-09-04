// Cost function for least-square problems
/*! Implements a cost function using the interface provided by
    the LeastSquareProblem class.
*/
///
#ifndef AQLMathLeastSquareHybridHWLVProcessQlib_h
#define AQLMathLeastSquareHybridHWLVProcessQlib_h

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


#include "AQLModelDynamicsCurve.h"
#include "AQLMathHybridHWLVUtil.h"


/*! 
    @brief declaration of struct
*/
struct AQLMathTarget2fit1
{
public:
    AQLMathTarget2fit1(const DoubleArray& x,
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
struct AQLMathTarget2fit2
{
public:
    AQLMathTarget2fit2(const DoubleArray& x,
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
    
        if(n != mY.size()) throw AQLCoreInvalidData("x.size() != y.size() : AQLMathTarget2fit2::AQLMathTarget2fit2", __FILE__, __LINE__);
        if(n != mTarget2fit.size()) throw AQLCoreInvalidData("x.size() != target2fit.size() : AQLMathTarget2fit2::AQLMathTarget2fit2", __FILE__, __LINE__);
        if(n != mWeight.size()) throw AQLCoreInvalidData("x.size() != weight.size() : AQLMathTarget2fit2::AQLMathTarget2fit2", __FILE__, __LINE__);
    
        if(m != mTarget2fit[0].size()) throw AQLCoreInvalidData("y[0].size() != target2fit[0].size() : AQLMathTarget2fit2::AQLMathTarget2fit2", __FILE__, __LINE__);
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
class AQLMathLeastSquareHybridHWLVProcessQlib : public QuantLib::CostFunction
{
public:

    enum mode { Diff, Ratio, Sqrt_Ratio };

    //
    AQLMathLeastSquareHybridHWLVProcessQlib() {}

    AQLMathLeastSquareHybridHWLVProcessQlib(const DoubleArray& fx0,
                                     std::vector<AQLMathHybridHWLVProcessHelper*>& processHelper,
                                     AQLMathTarget2fit2& t2f,
                                     mode mode_ = Diff
                                    );

    // Copy constructor
    AQLMathLeastSquareHybridHWLVProcessQlib( const AQLMathLeastSquareHybridHWLVProcessQlib& rhs );   

    // Destructor
    virtual ~AQLMathLeastSquareHybridHWLVProcessQlib();

    // compute value of the least square function
    virtual QuantLib::Real value(const QuantLib::Array& x) const;

    // compute value of the least square function
    virtual QuantLib::Array values(const QuantLib::Array& x) const;

    //virtual void gradient(Array& grad, const Array& x) const { grad = x; }

    //
    AQLMathLeastSquareHybridHWLVProcessQlib& operator =(const AQLMathLeastSquareHybridHWLVProcessQlib& rhs);
    
 
protected:
    // compute the values of the function to fit
    virtual void getTarget2fit() const;

    // compute the values of the function to fit
	virtual void getFct2fit(const QuantLib::Array& x) const = 0;

    // target to fit
    AQLMathTarget2fit2* mpT2f;
    mutable DoubleArray mTarget2fit;
    mutable DoubleArray mWeight;

    // factors to fit
    mutable DoubleArray mFct2fit;

    //
    DoubleArray mFX0;

    // setUp
    void setUp(const DoubleArray& fx0,
               std::vector<AQLMathHybridHWLVProcessHelper*>& processHelper,
               AQLMathTarget2fit2& t2f,
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
    std::vector<AQLMathHybridHWLVProcessHelper*> mProcessHelper;
    
    //
    mode mMode;


    //
    bool mCloned;
};

/*! 
    @brief declaration of class
*/
class AQLMathLeastSquareHybridHWDDProcessQlib : public AQLMathLeastSquareHybridHWLVProcessQlib
{
public:
    AQLMathLeastSquareHybridHWDDProcessQlib(const DoubleArray& fx0,
                                     std::vector<AQLMathHybridHWDDProcessHelper*>& processHelper,
                                     AQLMathTarget2fit2& t2f,
                                     mode mode_ = Diff
                                    );

    // Copy constructor
    AQLMathLeastSquareHybridHWDDProcessQlib( const AQLMathLeastSquareHybridHWDDProcessQlib& rhs );   

    // Destructor
    virtual ~AQLMathLeastSquareHybridHWDDProcessQlib();
    //
    AQLMathLeastSquareHybridHWDDProcessQlib& operator = (const AQLMathLeastSquareHybridHWDDProcessQlib& rhs);
    
 
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
    mutable std::vector<AQLMathHybridHWDDProcessHelper*> mProcessHelper;

private:
    
    bool mCloned;
};

/*! 
    @brief declaration of class
*/
class AQLMathLeastSquareHybridHWDDProcess1Qlib : public AQLMathLeastSquareHybridHWDDProcessQlib
{
public:
    AQLMathLeastSquareHybridHWDDProcess1Qlib(const DoubleArray& fx0,
                                      std::vector<AQLMathHybridHWDDProcessHelper*>& processHelper,
                                      AQLMathTarget2fit2& vol2fit,
                                      AQLMathTarget2fit2& skew2fit,
                                      DoubleArray weightVolSkew, //{vol_weight, skew_weight}
                                      mode mode_ = Diff
                                     );

    // Copy constructor
    AQLMathLeastSquareHybridHWDDProcess1Qlib( const AQLMathLeastSquareHybridHWDDProcess1Qlib& rhs );   

    // Destructor
    virtual ~AQLMathLeastSquareHybridHWDDProcess1Qlib();

    // compute value of the least square function
    virtual QuantLib::Real value(const QuantLib::Array& x) const;

    // compute value of the least square function
	virtual QuantLib::Array values(const QuantLib::Array& x) const;

    //virtual void gradient(Array& grad, const Array& x) const { grad = x; }

    //
    AQLMathLeastSquareHybridHWDDProcess1Qlib& operator = (const AQLMathLeastSquareHybridHWDDProcess1Qlib& rhs);
    
 
protected:
    // compute the values of the function to fit
    virtual void getTarget2fit() const;

	virtual void getFct2fit(const QuantLib::Array& x) const;

private:
    
    // Target to fit AQLMathTarget2fit1 = {Skew} and AQLMathTarget2fit2 = {Vol} 
    AQLMathTarget2fit2* mpT2f2;
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
class AQLMathLeastSquareHybridHWDDProcess2Qlib : public AQLMathLeastSquareHybridHWDDProcessQlib
{
public:
    AQLMathLeastSquareHybridHWDDProcess2Qlib(const DoubleArray& fx0,
                                      std::vector<AQLMathHybridHWDDProcessHelper*>& processHelper,
                                      AQLMathTarget2fit2& vol2fit,
                                      double ts = 0.,
                                      mode mode_ = Diff
                                     );

    // Copy constructor
    AQLMathLeastSquareHybridHWDDProcess2Qlib( const AQLMathLeastSquareHybridHWDDProcess2Qlib& rhs );   

    // Destructor
    virtual ~AQLMathLeastSquareHybridHWDDProcess2Qlib();

    //
    AQLMathLeastSquareHybridHWDDProcess2Qlib& operator = (const AQLMathLeastSquareHybridHWDDProcess2Qlib& rhs);
    
 
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
class AQLMathLeastSquareHybridHWDDProcess3Qlib : public AQLMathLeastSquareHybridHWDDProcessQlib
{
public:
    AQLMathLeastSquareHybridHWDDProcess3Qlib(const DoubleArray& fx0,
                                      std::vector<AQLMathHybridHWDDProcessHelper*>& processHelper,
                                      AQLMathTarget2fit2& skew2fit,
                                      double ts = 0.,
                                      mode mode_ = Diff
                                     );

    // Copy constructor
    AQLMathLeastSquareHybridHWDDProcess3Qlib( const AQLMathLeastSquareHybridHWDDProcess3Qlib& rhs );   

    // Destructor
    virtual ~AQLMathLeastSquareHybridHWDDProcess3Qlib();

    //
    AQLMathLeastSquareHybridHWDDProcess3Qlib& operator = (const AQLMathLeastSquareHybridHWDDProcess3Qlib& rhs);
    
 
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
class AQLMathBoundaryConstraintHybridHWLVProcess: public QuantLib::Constraint
{
public:
    AQLMathBoundaryConstraintHybridHWLVProcess(double sigmaMax, double sigmaMin, double betaMax, double betaMin, size_t n = 2)
    :
	QuantLib::Constraint(boost::shared_ptr<Constraint::Impl>( new AQLMathBoundaryConstraintHybridHWLVProcess::Impl(sigmaMax, sigmaMin, betaMax, betaMin, n) ) )
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
