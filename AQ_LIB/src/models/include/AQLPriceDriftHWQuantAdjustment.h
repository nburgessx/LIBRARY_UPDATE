#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLMathDriftFuncBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLPriceDriftHW.h"


// ID for AQLPriceDriftHWQuantAdjustment
#define FN_DRIFTHWQUANTADJ	10036
// Function name for AQLPriceDriftHWQuantAdjustment
#define FN_DRIFTHWQUANTADJ_STR	"fn_drift_hw_quantadj"


class AQLMathPathEntity;
class AQLRatesNumeraireBase;
class AQLRatesSpotSDE;
class AQLMathVolFuncFXDD;

///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of drift function of quant adjustment

*/
class AQLPriceDriftHWQuantAdjustment : public AQLMathDriftFuncBase
{
public:
//  LIFECYCLE
	// Default constructor
	AQLPriceDriftHWQuantAdjustment(const AQLRatesNumeraireBase& domestic_nu, const AQLMathVolFuncFXDD& fxvol, AQLRatesSpotSDE& fx_sde, 
									AQLPriceDriftHW* pDriftIR, const DoubleArray& cor,
									double fx_criteria = 0.01);
	// constructor
	AQLPriceDriftHWQuantAdjustment(const AQLString& sdeAttrNameIR_D, const AQLString& sdeAttrNameIR_F, const AQLString& sdeAttrNameFX, AQLPriceDriftHW* driftIR, double fx_criteria = 0.01);	
	//	Copy constructor
	AQLPriceDriftHWQuantAdjustment(const AQLPriceDriftHWQuantAdjustment& v);
	// Destructor
	virtual ~AQLPriceDriftHWQuantAdjustment();

//  QUERY
								//======================================
								// Check function for this class ID
	virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual AQLCoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%
								//======================================
								// Return this class ID
	virtual function_t			getType() const;
	                            //==========================================
	                            // Return drift value
	virtual double				operator()(const DoubleArray& x) const;
								//==========================================
								// return string representaion
    virtual AQLString			convertToString(void) const;
								//==========================================
								// return drift function
	AQLPriceDriftHW*				getDrift() {return mpDriftIR;}

//	OPERATION
								//==========================================
								// transform from string representaion
     virtual void				convertFromString(const AQLString& str);

								//======================================
								// set up this class
	 virtual void				setUp(AQLMathPathEntity& path);


protected:
	void						setUp() const;


class AQLHWQAdjInnerFunc1 : public AQLFunctionBase
{
public:
//  LIFECYCLE
	// constructor
	AQLHWQAdjInnerFunc1(const AQLMathHWFuncMR& mr, const AQLFunctionBase& irvol, const AQLFunctionBase& fxvol)
		:mpMR(&mr), mpIRVol(&irvol), mpFxVol(&fxvol) {;}
	// destructor
	virtual ~AQLHWQAdjInnerFunc1() {;}


//  QUERY

								//======================================
								// Make copy(clone) of this class
	virtual AQLCoreFunctionBase*     clone() const {return new AQLHWQAdjInnerFunc1(*this);}
	                            //==========================================
	                            // return function value
	virtual double				operator()(const DoubleArray& x) const {return operator()(x[0]);}

	// suppress warning C4512
	AQLHWQAdjInnerFunc1 & operator=( const AQLHWQAdjInnerFunc1 & ) { return *this; }

	virtual double				operator()(double x) const
								{
									return AQLMath::exp(mpMR->integrate(0.0, x)) * (*mpIRVol)(x) * (*mpFxVol)(x);	
								}
private:
	const AQLMathHWFuncMR*	mpMR;
	const AQLFunctionBase* mpIRVol;
	const AQLFunctionBase* mpFxVol;
};

class AQLHWQAdjInnerFunc2 : public AQLFunctionBase
{
public:
//  LIFECYCLE
	// constructor
	AQLHWQAdjInnerFunc2(const AQLMathHWFuncMR& mr, const AQLFunctionBase& irvol, const AQLFunctionBase& fxvol, const AQLFunctionBase& s)
		:mpMR(&mr), mpIRVol(&irvol), mpFxVol(&fxvol), mpS(&s) {;}
	// destructor
	virtual ~AQLHWQAdjInnerFunc2() {;}


//  QUERY

								//======================================
								// Make copy(clone) of this class
	virtual AQLCoreFunctionBase*     clone() const {return new AQLHWQAdjInnerFunc2(*this);}
	                            //==========================================
	                            // return function value
	virtual double				operator()(const DoubleArray& x) const {return operator()(x[0]);}

	// suppress warning C4512
	AQLHWQAdjInnerFunc2 & operator=( const AQLHWQAdjInnerFunc2 & ) { return *this; }

	virtual double				operator()(double x) const
								{
									return AQLMath::exp(mpMR->integrate(0.0, x)) * (*mpIRVol)(x) * (*mpFxVol)(x) * (*mpS)(x);	
								}
private:
	const AQLMathHWFuncMR*	mpMR;
	const AQLFunctionBase* mpIRVol;
	const AQLFunctionBase* mpFxVol;
	const AQLFunctionBase* mpS;
};

class AQLHWQAdjInnerFunc3 : public AQLFunctionBase
{
public:
//  LIFECYCLE
	// constructor
	AQLHWQAdjInnerFunc3(const AQLMathHWFuncMR& mr, const AQLFunctionBase& irvol, const AQLFunctionBase& fxvol, const AQLFunctionBase& s)
		:mpMR(&mr), mpIRVol(&irvol), mpFxVol(&fxvol), mpS(&s) {;}
	// destructor
	virtual ~AQLHWQAdjInnerFunc3() {;}


//  QUERY

								//======================================
								// Make copy(clone) of this class
	virtual AQLCoreFunctionBase*     clone() const {return new AQLHWQAdjInnerFunc3(*this);}
	                            //==========================================
	                            // return function value
	virtual double				operator()(const DoubleArray& x) const {return operator()(x[0]);}

	// suppress warning C4512
	AQLHWQAdjInnerFunc3 & operator=( const AQLHWQAdjInnerFunc3 & ) { return *this; }

	virtual double				operator()(double x) const
								{
									return AQLMath::exp(mpMR->integrate(0.0, x)) * (*mpIRVol)(x) * (*mpFxVol)(x) * (*mpS)(x) * x;	
								}
private:
	const AQLMathHWFuncMR*	mpMR;
	const AQLFunctionBase* mpIRVol;
	const AQLFunctionBase* mpFxVol;
	const AQLFunctionBase* mpS;
};

//protected:
	const AQLRatesNumeraireBase*	mpNumeraireD;	// numerarire of domestic currency
	const AQLMathVolFuncFXDD*		mpFxVolatility;	// fx volatility		
	AQLRatesSpotSDE*				mpSDEFX;		// fx sde 
	AQLPriceDriftHW*				mpDriftIR;		// drift function of foregin ir without quant adjust
	DoubleArray					mCorrelation;	// correlation data
	mutable DoubleArray			mMinFx;			// minimum fx for avoid zero divide
	AQLString					mSDEAttrNameIR_D;	// data name of ir(domestic currency) sde
	AQLString					mSDEAttrNameIR_F;	// data name of ir(foreign currency) sde
	AQLString					mSDEAttrNameFX;	// data name of fx sde
	mutable	bool				mIsSetUped;		// setuped or not
	mutable	unsigned int		mPos_old;		// last cache position
	mutable DoubleArray			mCache1;		// cache
	mutable DoubleArray			mCache2;		// cache
	mutable DoubleArray			mCache3;		// cache
	
	double						mFxCriteria;	// fx criteria for avoid zero divide
};
