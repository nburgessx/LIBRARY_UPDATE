#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAMathVolFuncFX.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include "LARatesSDEBase.h"
#include "LAGaussLegendre.h"
#include "LA1DDataSet.h"


// Funciton ID of LAMathVolFuncSZDD
#define FN_VOLFUNCSZDD	10015
// Function Name of LAMathVolFuncSZDD
#define FN_VOLFUNCSZDD_STR	"fn_volfuncszdd"


class LAMathVolFuncSZDD : public LAMathVolFuncFX
{
public :
//  LIFECYCLE
	// constructor
	////LAMathVolFuncSZDD(const DoubleArray &timeGrid, const DoubleArray &sigma, 
	////								const DoubleArray &fx, const DoubleArray &beta, 
	////								const LAString &currency, SDE_TYPE type = DIVIDEdXbyX,
	////								int integrate_n_ = 30);
	//temp!!
	LAMathVolFuncSZDD(const DoubleArray &timeGrid, const DoubleArray &fx0,
									const DoubleArray &beta,const DoubleArray &theta,const DoubleArray &kappa,const DoubleArray &epsilon, 
									const LAString &currency, const DoubleArray &sigma , 
									SDE_TYPE type = DIVIDEdXbyX,int integrate_n_ = 30);
	// destructor
	virtual ~LAMathVolFuncSZDD(void);
	// copy constructor
	LAMathVolFuncSZDD(const LAMathVolFuncSZDD &rhs);

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
	                            // Return integral result
	virtual double				integral(const std::vector<std::pair<double,double> >& x) const;	
	                            //==========================================
	                            // Return S function
	const LAFunctionBase&		getS() const {return mS;}
	                            //==========================================
	                            // Return V(=beta * sigma) function
	const LAFunctionBase&		getV() const {return mV;}
	                            // Return alpha(=(1 - beta) * fx0) function
	const LAFunctionBase&		getAlpha() const {return mAlpha;}
								//==========================================
								// Return theta
	const LAFunctionBase&		getTheta() const {return mTheta;}
								//==========================================
								// Return kappa
	const LAFunctionBase&		getKappa() const {return mKappa;}
								//==========================================
								// Return Epsilon
	const LAFunctionBase&		getEpsilon() const {return mEpsilon;}
								//==========================================
								// set fwd fx
	virtual void				setFwdFX(const DoubleArray &fx);
								//==========================================
	                            // Return +-square dataInstance of integral of v^2(=beta^2*sigma^2)	
	double						getIntegralofV(double ts, double te) const;
	                            //==========================================
	                            // Return integral of s * square of v(=beta*sigma)		
	double						getIntegralofSVV(double ts, double te) const;
	                            //==========================================
	                            // Return +-square dataInstance of integral of square of s * v(=beta*sigma)	
	double						getIntegralofSV(double ts, double te) const;

protected :
	SDE_TYPE				mType;			// sde type
	mutable LA1DDataSet		mV;
	mutable LA1DDataSet		mS;
	mutable LA1DDataSet		mAlpha;
	//furuya
	mutable LA1DDataSet		mTheta;
	mutable LA1DDataSet		mEpsilon;
	mutable LA1DDataSet		mKappa;
	
private	:
	mutable std::map<double, std::vector<double> > integrate_cache;
	mutable std::map<double, bool> is_cache;

};
