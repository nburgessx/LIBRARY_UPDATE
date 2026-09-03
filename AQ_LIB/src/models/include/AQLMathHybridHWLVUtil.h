// Cost function for least-square problems
/*! Implements a cost function using the interface provided by
    the LeastSquareProblem class.
*/
//  2012, AlgoQuantHub.
#ifndef AQLMathHybridHWLVUtil_h
#define AQLMathHybridHWLVUtil_h


#include <AQLCoreUtil.h>
#include <vector>
#include <map>
#include "AQLFunction.h"
#include <limits>

//#ifdef __GNUC__
//#include <ext/unordered_map>
//using namespace __gnu_cxx;
//#else
//#include <unordered_map>
//using namespace stdext;
//#endif

//#include "AQLMathAnalyticalFormula.h"
#include "AQLModelDynamicsCurve.h"
#include "AQLBasic.h"
using namespace std;

//===================== Struct Declare AQLMathHullWhiteParams==================================
/*! 

*/
struct AQLMathHullWhiteParams
{
    public:
        AQLMathHullWhiteParams( const DoubleArray& t,
                         const DoubleArray& a,
                         const DoubleArray& sigma
                       )
        :
        mT(t),
        mA(a),
        mSigma(sigma)
        {}

        DoubleArray mT;
        DoubleArray mA;
        DoubleArray mSigma;    
};

//===================== Struct Declare AQLMathCorrelationParams==================================
/*! 

*/
struct AQLMathCorrelationParams
{
	public:
		AQLMathCorrelationParams(const DoubleArray& t,
						  const std::vector<DoubleMatrix>& rho
						 )
        
		:
		mT(t),
		mRho(rho)
		{}

		DoubleArray mT;
		std::vector<DoubleMatrix> mRho;    
};

//===================== Struct Declare AQLMathDisplacedDiffusionParams==================================
/*! 

*/
struct AQLMathDisplacedDiffusionParams
{
   public:
       AQLMathDisplacedDiffusionParams( const DoubleArray& t,
                                 const DoubleArray& sigma,
                                 const DoubleArray& beta
                               )
       :
       mT(t),
       mSigma(sigma),
       mBeta(beta)
       {}

       DoubleArray mT;
       DoubleArray mSigma;
       DoubleArray mBeta;
};

//===================== Class Declare AQLMathHybridHWLVProcessHelper==================================
/*! 

*/
class AQLMathHybridHWLVProcessHelper 
{
   public:
       //
       AQLMathHybridHWLVProcessHelper(double T,
                                   double fx0,
                                   AQLMathHullWhiteParams& hwParams_d,
                                   AQLMathHullWhiteParams& hwParams_f,
                                   AQLMathCorrelationParams& corParams,
                                   const DoubleArray& integralTimeSteps,
                                   size_t numInt = 1
                                  );
       
       //
       AQLMathHybridHWLVProcessHelper( const AQLMathHybridHWLVProcessHelper& rhs );
        
       //
       virtual ~AQLMathHybridHWLVProcessHelper() {}

       // Model Volatility
       virtual double calcVar() const { return calcIntLamSQ(mT); }
	   double calcSigma() const { return AQLMath::sqrt(calcVar() / mT); }

       // Discount Bond Price volatility
       virtual double calcVol_Pd(double t) const { return calcVol_Pd(t, mT); }
       virtual double calcVol_Pf(double t) const { return calcVol_Pf(t, mT); }

       virtual AQLMathHybridHWLVProcessHelper* clone() const = 0;

   protected:

       // SetUp Maturity
       void setT(double T) const;

       // Discount Bond Price volatility Helper
       virtual double calcVol_Pd(double t, double T) const;
       virtual double calcVol_Pf(double t, double T) const;
       virtual double calcE_d(double t) const;
       virtual double calcInvE_d(double t) const;
       virtual double calcE_f(double t) const;
       virtual double calcInvE_f(double t) const;

       // Model Volatility Helper
       double calcLambda(double t) const;
       double calcLambdaSQ(double t) const;
       double calcIntLamSQ(double t) const;

       // LV
       virtual double calcGamma(double t) const = 0;
       virtual double calcGamma(double t, double x) const = 0;
       
       // Maturity
       mutable double mT;

       // Forward FX
       double mFX0;
      
       // Integral Information
       size_t mNumInt;
       DoubleArray mIntegralTimeSteps;
 
       // corrilation between r_d, r_f and S
       AQLMathCorrelationParams* mpCorParams;

       // Hull-White Model parameters
       AQLMathHullWhiteParams* mpHWParams_d;
       AQLMathHullWhiteParams* mpHWParams_f;
       
       // Clear Cache
       virtual void clearCache1() const;
       virtual void clearCache2() const;
       virtual void clearCache() const
       {
            mCacheE_d.clear();
            mIsCacheE_d.clear();
            
            mCacheVol_Pd.clear();
            mIsCacheVol_Pd.clear();

            mCacheE_f.clear();
            mIsCacheE_f.clear();

            mCacheVol_Pf.clear();
            mIsCacheVol_Pf.clear();

            mCacheIntLamSQ.clear();
            mIsCacheIntLamSQ.clear();
       }

       size_t searchIdx(double x, const DoubleArray& y) const
       {
		   size_t i = std::upper_bound(y.begin(), y.end(), x) - y.begin();
            if( i != 0 && i < y.size() ) i = eq(x, y[i], 1. / 367. ) ? i : --i;
            return i;
       }

       //
       mutable map<double, map<double, double> > mCacheVol_Pd;
       mutable map<double, map<double, bool> > mIsCacheVol_Pd;
    
       mutable map<double, map<double, double> > mCacheVol_Pf;
       mutable map<double, map<double, bool> > mIsCacheVol_Pf;

private:

       // Cache
        AQLMathFunction<AQLMathHybridHWLVProcessHelper> mIntInvE_d;
        mutable map<double, double> mCacheE_d;
        mutable map<double, bool> mIsCacheE_d;

        AQLMathFunction<AQLMathHybridHWLVProcessHelper> mIntInvE_f;
        mutable map<double, double> mCacheE_f;
        mutable map<double, bool> mIsCacheE_f;

        AQLMathFunction<AQLMathHybridHWLVProcessHelper> mIntLamSQ;
        mutable map<double, map<double, double> > mCacheIntLamSQ;
        mutable map<double, map<double, bool> > mIsCacheIntLamSQ;
};

//===================== Class Declare AQLMathHybridHWDDProcessHelper==================================
/*! 

*/
class AQLMathHybridHWDDProcessHelper : public AQLMathHybridHWLVProcessHelper
{
    public:
        //
        AQLMathHybridHWDDProcessHelper(double T,
                                    double fx0,
                                    AQLMathHullWhiteParams& hwParams_d,
                                    AQLMathHullWhiteParams& hwParams_f,
                                    AQLMathDisplacedDiffusionParams& ddParams,
                                    AQLMathCorrelationParams& corParams,
                                    const DoubleArray& integralTimeSteps,
                                    size_t numInt = 1
                                   );

        //
        AQLMathHybridHWDDProcessHelper( const AQLMathHybridHWDDProcessHelper& rhs);

        //
        virtual ~AQLMathHybridHWDDProcessHelper() {}

        //
        virtual AQLMathHybridHWDDProcessHelper* clone() const { return new AQLMathHybridHWDDProcessHelper(*this); }

        //
        virtual double calcSkew() const { return 0.; } // under construction

        //
        virtual double calcCDF(double ts, double te) const { return 0.; } // under construction

        //
        void setSigma(double t, double sigma)
        {
//clearCache();
            clearCache1();
            //size_t t_idx = upper_bound(mpDDParams->T.begin(), mpDDParams->T.end(), t) - mpDDParams->T.begin();
            //if( t_idx != 0 && t_idx < mpDDParams->T.size() ) t_idx = eq(t, mpDDParams->T[t_idx], 1. / 367. ) ? t_idx : --t_idx;
            size_t t_idx = searchIdx(t, mpDDParams->mT);
            mpDDParams->mSigma[t_idx] = sigma;    
        }

        void setBeta(double t, double beta)
        {
//clearCache();
            clearCache1();
            //size_t t_idx = upper_bound(mpDDParams->T.begin(), mpDDParams->T.end(), t) - mpDDParams->T.begin();
            //if( t_idx != 0 && t_idx < mpDDParams->T.size() ) t_idx = eq(t, mpDDParams->T[t_idx], 1. / 367. ) ? t_idx : --t_idx;
            size_t t_idx = searchIdx(t, mpDDParams->mT);

            mpDDParams->mBeta[t_idx] = beta;            
        }

        DoubleArray getDDTimeStep()
        {
            return mpDDParams->mT;
        }

    protected:

        //
        // LV
        virtual double calcGamma(double t, double x ) const;
        virtual double calcGamma(double t) const { return calcGamma(t, mFX0); }

        // DD params
        AQLMathDisplacedDiffusionParams* mpDDParams;

    private:
};

//===================== Class Declare AQLMathHybridHWDDMPProcessHelper==================================
/*! 

*/

   // Markovian Projection of AQLMathHybridHWDDProcessHelper
class AQLMathHybridHWDDMPProcessHelper : public AQLMathHybridHWDDProcessHelper
{
   public:
       AQLMathHybridHWDDMPProcessHelper(double T,
                                      double fx0,
                                      AQLMathHullWhiteParams& hwParams_d,
                                      AQLMathHullWhiteParams& hwParams_f,
                                      AQLMathDisplacedDiffusionParams& ddParams,
                                      AQLMathCorrelationParams& corParams,
                                      const DoubleArray& integralTimeSteps,
                                      size_t numInt = 1
                                     )
       :
       AQLMathHybridHWDDProcessHelper(T, fx0, hwParams_d, hwParams_f, ddParams, corParams, integralTimeSteps, numInt)
       {
            mIntU1Helper.SetFunc( *this, &AQLMathHybridHWDDMPProcessHelper::calcU1helper );
            mIntSkewHelper.SetFunc( *this, &AQLMathHybridHWDDMPProcessHelper::calcSkewHelper );
            mIntCDFHelper.SetFunc( *this, &AQLMathHybridHWDDMPProcessHelper::calcCDFHelper );
       }
       
       AQLMathHybridHWDDMPProcessHelper(const AQLMathHybridHWDDMPProcessHelper& rhs)
       :
       AQLMathHybridHWDDProcessHelper(rhs),
       m_t(rhs.m_t)
       {
            mIntU1Helper.SetFunc( *this, &AQLMathHybridHWDDMPProcessHelper::calcU1helper );
            mIntSkewHelper.SetFunc( *this, &AQLMathHybridHWDDMPProcessHelper::calcSkewHelper );
            mIntCDFHelper.SetFunc( *this, &AQLMathHybridHWDDMPProcessHelper::calcCDFHelper );
       }
       
       //
       virtual ~AQLMathHybridHWDDMPProcessHelper() {}

       //
       virtual AQLMathHybridHWDDMPProcessHelper* Clone() const { return new AQLMathHybridHWDDMPProcessHelper(*this); }

       //
       double calcVar() const { return calcIntLamSQ(mT); }
       
       //
       virtual double calcSkew() const;
       virtual double calcSkewHelper(double t) const;

       virtual double calcCDF(double ts, double te) const;
       virtual double calcCDFHelper(double t) const;
       AQLMathFunction<AQLMathHybridHWDDMPProcessHelper> mIntCDFHelper;

   protected:

       // SetUp Spot
       virtual void set_t(double t) const;
       
       // Markovian representation of gamma
       virtual double calcGamma(double t, double x) const;
       virtual double calcGamma(double t) const { return calcGamma(t, mFX0); }

       //
       virtual double calcWeight(double t) const;

       //
       virtual double calcU1(double t) const;
       virtual double calcU2(double t) const;

       //
       virtual double calcR(double t) const { return calcU1(t) / calcU2(t); }

       // spot
       mutable double m_t;

       virtual void clearCache2() const
       {
           mCacheSkew.clear();
           mIsCacheSkew.clear();
       }

       virtual void clearCache() const
       {
           AQLMathHybridHWLVProcessHelper::clearCache();
           
           mCacheSkew.clear();
           mIsCacheSkew.clear();

           mCacheWeight.clear();
           mIsCacheWeight.clear();

           mCacheU1.clear();
           mIsCacheU1.clear();
       }

       mutable map<double, double> mCacheWeight;
       mutable map<double, bool> mIsCacheWeight;

   private:
       virtual double calcU1helper(double s) const;


       // Skew
       AQLMathFunction<AQLMathHybridHWDDMPProcessHelper> mIntSkewHelper;
       mutable map<double, map<double, double> > mCacheSkew;
       mutable map<double, map<double, bool> > mIsCacheSkew;

       //
       AQLMathFunction<AQLMathHybridHWDDMPProcessHelper> mIntU1Helper;
       mutable map<double, double> mCacheU1;
       mutable map<double, bool> mIsCacheU1;
};

//===================== Class Declare AQLMathHybridHWDDProcessHelper==================================
/*! 

*/
   // Markovian Projection of AQLMathHybridHWDDProcessHelper
class AQLMathHybridHWDDMPProcessHelper2 : public AQLMathHybridHWDDMPProcessHelper
{
   public:

       //
       AQLMathHybridHWDDMPProcessHelper2(double T,
                                       double fx0,
                                       AQLMathHullWhiteParams& hwParams_d,
                                       AQLMathHullWhiteParams& hwParams_f,
                                       AQLMathDisplacedDiffusionParams& ddParams,
                                       AQLMathCorrelationParams& corParams,
                                       const DoubleArray& integralTimeSteps,
                                       size_t numInt = 1
                                      )
       :
       AQLMathHybridHWDDMPProcessHelper(T, fx0, hwParams_d, hwParams_f, ddParams, corParams, integralTimeSteps, numInt)
       {
           mIntU3Helper.SetFunc( *this, &AQLMathHybridHWDDMPProcessHelper2::calcU3Helper );
           mIntC1Helper.SetFunc( *this, &AQLMathHybridHWDDMPProcessHelper2::calcC1Helper );
           mIntLamSQApproximated.SetFunc(*this, &AQLMathHybridHWDDMPProcessHelper2::calcLamSQApproximated);  
       }

       //
       AQLMathHybridHWDDMPProcessHelper2(const AQLMathHybridHWDDMPProcessHelper2& rhs)
       :
       AQLMathHybridHWDDMPProcessHelper(rhs)
       {
       }
       
       //
       virtual ~AQLMathHybridHWDDMPProcessHelper2() {}

       //
       virtual AQLMathHybridHWDDMPProcessHelper2* Clone() const { return new AQLMathHybridHWDDMPProcessHelper2(*this); }
       
   protected:
       // SetUp Spot
       virtual void set_t(double t) const
       {
           AQLMathHybridHWDDMPProcessHelper::set_t(t);
           
           if(m_t != t)
           {
                m_t = t;
                mIsCacheU3.clear();
                mCacheU3.clear();
            }    
       }
       
       //// Markovian representation of gamma
       virtual double calcGamma(double t, double x) const
       {
            size_t i = searchIdx(t, mpDDParams->mT);
            
            double simga = mpDDParams->mSigma[i];
            double beta = mpDDParams->mBeta[i];

            if(x == mFX0)
            {
                //if(t < 0.00001) return simga * ( beta + (1. - beta) );
                //return simga * ( beta + (1. - beta) * theta(t) );
                return simga;
            }
            else
            {
                //if(t < 0.00001) return  simga * ( beta + (1. - beta) * mFX0 / x * AQLMath::exp(-calcR(t) *  log(x / mFX0) ) );;
                //return simga * ( beta + (1. - beta) * mFX0 / x * AQLMath::exp(-calcR(t) *  log(x / mFX0) * theta(t) ) );
                return simga * ( beta + (1. - beta) * mFX0 / x * AQLMath::exp(-calcR(t) *  log(x / mFX0) ) );
            }
       }

       virtual double calcGamma(double t) const { return calcGamma(t, mFX0); }
       
       virtual double calcU2(double t) const;
       
       virtual double theta(double t) const
       {
           double u1 = calcU1(t);
           return AQLMath::exp( calcC1(t) - 0.5 * u1 + 0.5 * ( calcU3(t) - u1 * u1 / calcU2(t) ) );
           //return AQLMath::exp( calcC1(t) - 0.5 * u1_ + 0.5 * ( calcU3(t) ) );
       }

       //
       virtual double calcWeight(double t) const;

       virtual double calcIntLamSQApproximated(double t) const;
       virtual double calcLambdaApproximated(double t) const;
       virtual double calcLamSQApproximated(double t) const;

       // Clear Cache
       void clearCache1() const
       {
            AQLMathHybridHWDDMPProcessHelper::clearCache1();
            AQLMathHybridHWDDMPProcessHelper::clearCache2();
            
            mCacheIntLamSQApproximated.clear();
            mIsCacheIntLamSQApproximated.clear();

            mCacheC1.clear();
            mIsCacheC1.clear();

            mCacheU3.clear();
            mIsCacheU3.clear();
       }

       void clearCache() const
       {
            AQLMathHybridHWDDMPProcessHelper::clearCache();
 
            mCacheIntLamSQApproximated.clear();
            mIsCacheIntLamSQApproximated.clear();

            mCacheC1.clear();
            mIsCacheC1.clear();

            mCacheU3.clear();
            mIsCacheU3.clear();
       }
       
   private:

       //
       virtual double calcC1(double s) const;
       virtual double calcC1Helper(double s) const;

       virtual double calcU3(double s) const;
       virtual double calcU3Helper(double s) const;

       //
       AQLMathFunction<AQLMathHybridHWDDMPProcessHelper2> mIntC1Helper;
       mutable map<double, double> mCacheC1;
       mutable map<double, bool> mIsCacheC1;

       //
       AQLMathFunction<AQLMathHybridHWDDMPProcessHelper2> mIntU3Helper;
       mutable map<double, double> mCacheU3;
       mutable map<double, bool> mIsCacheU3;

       AQLMathFunction<AQLMathHybridHWDDMPProcessHelper2> mIntLamSQApproximated;
       mutable map<double, map<double, double> > mCacheIntLamSQApproximated;
       mutable map<double, map<double, bool> > mIsCacheIntLamSQApproximated;
};

//===================== Class Declare AQLMathHybridHWDDMPProcessHelperConstHWparams==================================
/*! 

*/

   // Markovian Projection of AQLMathHybridHWDDProcessHelper. 
   // This class restrict AQLMathHullWhiteParams->a = const.
class AQLMathHybridHWDDMPProcessHelperConstHWparams : public AQLMathHybridHWDDMPProcessHelper
{
   public:
       AQLMathHybridHWDDMPProcessHelperConstHWparams(double T,
                                                    double fx0,
                                                    AQLMathHullWhiteParams& hwParams_d,
                                                    AQLMathHullWhiteParams& hwParams_f,
                                                    AQLMathDisplacedDiffusionParams& ddParams,
                                                    AQLMathCorrelationParams& corParams,
                                                    const DoubleArray& integralTimeSteps,
                                                    size_t numInt = 1
                                                   )
       :
       AQLMathHybridHWDDMPProcessHelper(T, fx0, hwParams_d, hwParams_f, ddParams, corParams, integralTimeSteps, numInt)
       {
       }
       
       AQLMathHybridHWDDMPProcessHelperConstHWparams(const AQLMathHybridHWDDMPProcessHelperConstHWparams& rhs)
       :
       AQLMathHybridHWDDMPProcessHelper(rhs)
       {
       }
       
       //
       virtual ~AQLMathHybridHWDDMPProcessHelperConstHWparams() {}

       //
       virtual AQLMathHybridHWDDMPProcessHelperConstHWparams* Clone() const { return new AQLMathHybridHWDDMPProcessHelperConstHWparams(*this); }

        //
        double calcE_d(double t) const
        {
            return AQLMath::exp( mpHWParams_d->mA[0] * t );
        }    

        //
        double calcVol_Pd(double t, double T) const
        { 
            if(T == 0)
            {
	            return 0.;
            }

            double a = mpHWParams_d->mA[0];    
            if(!mIsCacheVol_Pd[T][t])
	    {
	            mCacheVol_Pd[T][t] = 1. / a * ( 1. - AQLMath::exp( -a * (T - t) ) );
	            mIsCacheVol_Pd[T][t] = true;
            }

            size_t p  = searchIdx(t, mIntegralTimeSteps);
//            return  mpHWParams_d->mSigma[p] * calcE_d(t) * mCacheVol_Pd[T][t];
            return  mpHWParams_d->mSigma[p] * mCacheVol_Pd[T][t];
        }

        double calcE_f(double t) const
        {
            return AQLMath::exp( mpHWParams_f->mA[0] * t );
        }

        double calcVol_Pf(double t, double T) const
        {
            if(T == 0)
            {
	            return 0.;
            }

            double a = mpHWParams_f->mA[0];    
            if(!mIsCacheVol_Pf[T][t])
	        {
	            mCacheVol_Pf[T][t] = 1. / a * ( 1. - AQLMath::exp( -a * (T - t) ) );
	            mIsCacheVol_Pf[T][t] = true;
            }
            
            size_t p  = searchIdx(t, mIntegralTimeSteps);
            //return  mpHWParams_f->mSigma[p] * calcE_f(t) * mCacheVol_Pf[T][t];
            return  mpHWParams_f->mSigma[p] * mCacheVol_Pf[T][t];
        }

   protected:
   private:
};
//};

#endif
