#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLInterpolationBase.h"
#include "AQLAlgorithm.h"
#include "AQL1DDataSet.h"
#include "AQLGaussLegendre.h"


// Funciton ID of AQLMathVolFuncFX
#define FN_VOLFUNCFX	10012
// Function Name of AQLMathVolFuncFX
#define FN_VOLFUNCFX_STR	"fn_volfuncfx"


class AQLMathVolFuncFX : public AQLFunctionBase
{
public :
//  LIFECYCLE
	// constructor
	explicit AQLMathVolFuncFX(const DoubleArray &timeGrid, const DoubleArray &sigma, 
									const DoubleArray &fx, const DoubleArray &beta, const AQLString &currency, int integrate_n_ = 30);
	// destructor
	virtual ~AQLMathVolFuncFX(void);
	// copy constructor
	AQLMathVolFuncFX(const AQLMathVolFuncFX &rhs);

								//======================================
								// check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// make copy(clone) of this class
    virtual AQLCoreFunctionBase*     clone() const;
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
	const AQLString			    getCurrency() const
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

	AQLString    mCurrency;		// currency
	mutable std::map<double, double> mIntegratedCacheSigma; //chache sigma
	mutable std::map<double, bool> mIsCacheSigma;           //chache flag
	mutable AQL1DDataSet		mSigmaFunc;                     //sigma function
	AQLGaussLegendre	mGL;        // gauss legendre
	
};

