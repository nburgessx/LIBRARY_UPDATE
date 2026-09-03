#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLMathVolFuncFX.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLRatesSDEBase.h"
#include "AQLGaussLegendre.h"
#include "AQL1DDataSet.h"


// Funciton ID of AQLMathVolFuncSZDD
#define FN_VOLFUNCSZDD	10015
// Function Name of AQLMathVolFuncSZDD
#define FN_VOLFUNCSZDD_STR	"fn_volfuncszdd"


class AQLMathVolFuncSZDD : public AQLMathVolFuncFX
{
public :
//  LIFECYCLE
	// constructor
	////AQLMathVolFuncSZDD(const DoubleArray &timeGrid, const DoubleArray &sigma, 
	////								const DoubleArray &fx, const DoubleArray &beta, 
	////								const AQLString &currency, SDE_TYPE type = DIVIDEdXbyX,
	////								int integrate_n_ = 30);
	//temp!!
	AQLMathVolFuncSZDD(const DoubleArray &timeGrid, const DoubleArray &fx0,
									const DoubleArray &beta,const DoubleArray &theta,const DoubleArray &kappa,const DoubleArray &epsilon, 
									const AQLString &currency, const DoubleArray &sigma , 
									SDE_TYPE type = DIVIDEdXbyX,int integrate_n_ = 30);
	// destructor
	virtual ~AQLMathVolFuncSZDD(void);
	// copy constructor
	AQLMathVolFuncSZDD(const AQLMathVolFuncSZDD &rhs);

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
	                            // Return integral result
	virtual double				integral(const std::vector<std::pair<double,double> >& x) const;	
	                            //==========================================
	                            // Return S function
	const AQLFunctionBase&		getS() const {return mS;}
	                            //==========================================
	                            // Return V(=beta * sigma) function
	const AQLFunctionBase&		getV() const {return mV;}
	                            // Return alpha(=(1 - beta) * fx0) function
	const AQLFunctionBase&		getAlpha() const {return mAlpha;}
								//==========================================
								// Return theta
	const AQLFunctionBase&		getTheta() const {return mTheta;}
								//==========================================
								// Return kappa
	const AQLFunctionBase&		getKappa() const {return mKappa;}
								//==========================================
								// Return Epsilon
	const AQLFunctionBase&		getEpsilon() const {return mEpsilon;}
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
	mutable AQL1DDataSet		mV;
	mutable AQL1DDataSet		mS;
	mutable AQL1DDataSet		mAlpha;
	//furuya
	mutable AQL1DDataSet		mTheta;
	mutable AQL1DDataSet		mEpsilon;
	mutable AQL1DDataSet		mKappa;
	
private	:
	mutable std::map<double, std::vector<double> > integrate_cache;
	mutable std::map<double, bool> is_cache;

};
