#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAFunctionBase.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include "LAInterpolationBase.h"
#include "LAAlgorithm.h"
#include "LA1DDataSet.h"
#include "LAGaussLegendre.h"


// Funciton ID of LAMathVolFuncFX
#define FN_VOLFUNCFX	10012
// Function Name of LAMathVolFuncFX
#define FN_VOLFUNCFX_STR	"fn_volfuncfx"


class LAMathVolFuncFX : public LAFunctionBase
{
public :
//  LIFECYCLE
	// constructor
	explicit LAMathVolFuncFX(const DoubleArray &timeGrid, const DoubleArray &sigma, 
									const DoubleArray &fx, const DoubleArray &beta, const LAString &currency, int integrate_n_ = 30);
	// destructor
	virtual ~LAMathVolFuncFX(void);
	// copy constructor
	LAMathVolFuncFX(const LAMathVolFuncFX &rhs);

								//======================================
								// check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// make copy(clone) of this class
    virtual LACoreFunctionBase*     clone() const;
								//======================================
								// return this class type
    virtual function_t          getType() const;

	                            //==========================================
	                            // return function value
	virtual double				operator()(const DoubleArray& x) const;	
	                            //==========================================
	                            // return sigma values
	double                      getSigma(double t) const;
	                            //==========================================
	                            // return sigma values
	const DoubleArray            &getSigma() const
	{ 
		return mSigma;
	}
	                            //==========================================
	                            // return forward fx values
	double                     getForwardFX0(double t) const;
	                            //==========================================
	                            // return forward fx values
	const DoubleArray           &getForwardFX0() const
	{ 
		return mFX0;
	}
	                            //==========================================
	                            // return beta values
	double                      getBeta(double t) const;
	                            //==========================================
	                            // return forward fx values
	const DoubleArray           &getBeta() const
	{
		return mBeta;
	}
	
	

	                            //==========================================
	                            // return time grid
	const DoubleArray           &getTimeGrid() const
	{
		return mTimeGrid;
	}
								//==========================================
	                            // return currency
	const LAString			    getCurrency() const
	{
		return mCurrency;
	}
	                            //==========================================
	                            // return index for t
	unsigned int                searchIndex(double t) const;
								//==========================================
	                            // Return +-square dataInstance of integral of v^2(=beta^2*sigma^2)	
	double						getIntegralofSigma(double ts, double te) const;

								//==========================================
								// set fwd fx
	virtual void				setFwdFX(const DoubleArray &fx) { mFX0 = fx; };
	
protected :
	DoubleArray mTimeGrid;      // time grid
	DoubleArray mSigma;		    // sigma
	DoubleArray mFX0;		    // forward fx
	DoubleArray mBeta;          // beta

	LAString    mCurrency;		// currency
	mutable std::map<double, double> mIntegratedCacheSigma; //chache sigma
	mutable std::map<double, bool> mIsCacheSigma;           //chache flag
	mutable LA1DDataSet		mSigmaFunc;                     //sigma function
	LAGaussLegendre	mGL;        // gauss legendre
	
};

