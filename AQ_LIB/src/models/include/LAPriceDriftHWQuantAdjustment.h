#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAMathDriftFuncBase.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include "LAPriceDriftHW.h"


// ID for LAPriceDriftHWQuantAdjustment
#define FN_DRIFTHWQUANTADJ	10036
// Function name for LAPriceDriftHWQuantAdjustment
#define FN_DRIFTHWQUANTADJ_STR	"fn_drift_hw_quantadj"


class LAMathPathEntity;
class LARatesNumeraireBase;
class LARatesSpotSDE;
class LAMathVolFuncFXDD;

///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of drift function of quant adjustment

*/
class LAPriceDriftHWQuantAdjustment : public LAMathDriftFuncBase
{
public:
//  LIFECYCLE
	// Default constructor
	LAPriceDriftHWQuantAdjustment(const LARatesNumeraireBase& domestic_nu, const LAMathVolFuncFXDD& fxvol, LARatesSpotSDE& fx_sde, 
									LAPriceDriftHW* pDriftIR, const DoubleArray& cor,
									double fx_criteria = 0.01);
	// constructor
	LAPriceDriftHWQuantAdjustment(const LAString& sdeAttrNameIR_D, const LAString& sdeAttrNameIR_F, const LAString& sdeAttrNameFX, LAPriceDriftHW* driftIR, double fx_criteria = 0.01);	
	//	Copy constructor
	LAPriceDriftHWQuantAdjustment(const LAPriceDriftHWQuantAdjustment& v);
	// Destructor
	virtual ~LAPriceDriftHWQuantAdjustment();

//  QUERY
								//======================================
								// Check function for this class ID
	virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual LACoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%
								//======================================
								// Return this class ID
	virtual function_t			getType() const;
	                            //==========================================
	                            // Return drift value
	virtual double				operator()(const DoubleArray& x) const;
								//==========================================
								// return string representaion
    virtual LAString			convertToString(void) const;
								//==========================================
								// return drift function
	LAPriceDriftHW*				getDrift() {return mpDriftIR;}

//	OPERATION
								//==========================================
								// transform from string representaion
     virtual void				convertFromString(const LAString& str);

								//======================================
								// set up this class
	 virtual void				setUp(LAMathPathEntity& path);


protected:
	void						setUp() const;


class MMHWQAdjInnerFunc1 : public LAFunctionBase
{
public:
//  LIFECYCLE
	// constructor
	MMHWQAdjInnerFunc1(const LAMathHWFuncMR& mr, const LAFunctionBase& irvol, const LAFunctionBase& fxvol)
		:mpMR(&mr), mpIRVol(&irvol), mpFxVol(&fxvol) {;}
	// destructor
	virtual ~MMHWQAdjInnerFunc1() {;}


//  QUERY

								//======================================
								// Make copy(clone) of this class
	virtual LACoreFunctionBase*     clone() const {return new MMHWQAdjInnerFunc1(*this);}
	                            //==========================================
	                            // return function value
	virtual double				operator()(const DoubleArray& x) const {return operator()(x[0]);}

	//20061017--David--Remove warning:C4512
	MMHWQAdjInnerFunc1 & operator=( const MMHWQAdjInnerFunc1 & ) { return *this; }

	virtual double				operator()(double x) const
								{
									return LAMath::exp(mpMR->integrate(0.0, x)) * (*mpIRVol)(x) * (*mpFxVol)(x);	
								}
private:
	const LAMathHWFuncMR*	mpMR;
	const LAFunctionBase* mpIRVol;
	const LAFunctionBase* mpFxVol;
};

class MMHWQAdjInnerFunc2 : public LAFunctionBase
{
public:
//  LIFECYCLE
	// constructor
	MMHWQAdjInnerFunc2(const LAMathHWFuncMR& mr, const LAFunctionBase& irvol, const LAFunctionBase& fxvol, const LAFunctionBase& s)
		:mpMR(&mr), mpIRVol(&irvol), mpFxVol(&fxvol), mpS(&s) {;}
	// destructor
	virtual ~MMHWQAdjInnerFunc2() {;}


//  QUERY

								//======================================
								// Make copy(clone) of this class
	virtual LACoreFunctionBase*     clone() const {return new MMHWQAdjInnerFunc2(*this);}
	                            //==========================================
	                            // return function value
	virtual double				operator()(const DoubleArray& x) const {return operator()(x[0]);}

	//20061017--David--Remove warning:C4512
	MMHWQAdjInnerFunc2 & operator=( const MMHWQAdjInnerFunc2 & ) { return *this; }

	virtual double				operator()(double x) const
								{
									return LAMath::exp(mpMR->integrate(0.0, x)) * (*mpIRVol)(x) * (*mpFxVol)(x) * (*mpS)(x);	
								}
private:
	const LAMathHWFuncMR*	mpMR;
	const LAFunctionBase* mpIRVol;
	const LAFunctionBase* mpFxVol;
	const LAFunctionBase* mpS;
};

class MMHWQAdjInnerFunc3 : public LAFunctionBase
{
public:
//  LIFECYCLE
	// constructor
	MMHWQAdjInnerFunc3(const LAMathHWFuncMR& mr, const LAFunctionBase& irvol, const LAFunctionBase& fxvol, const LAFunctionBase& s)
		:mpMR(&mr), mpIRVol(&irvol), mpFxVol(&fxvol), mpS(&s) {;}
	// destructor
	virtual ~MMHWQAdjInnerFunc3() {;}


//  QUERY

								//======================================
								// Make copy(clone) of this class
	virtual LACoreFunctionBase*     clone() const {return new MMHWQAdjInnerFunc3(*this);}
	                            //==========================================
	                            // return function value
	virtual double				operator()(const DoubleArray& x) const {return operator()(x[0]);}

	//20061017--David--Remove warning:C4512
	MMHWQAdjInnerFunc3 & operator=( const MMHWQAdjInnerFunc3 & ) { return *this; }

	virtual double				operator()(double x) const
								{
									return LAMath::exp(mpMR->integrate(0.0, x)) * (*mpIRVol)(x) * (*mpFxVol)(x) * (*mpS)(x) * x;	
								}
private:
	const LAMathHWFuncMR*	mpMR;
	const LAFunctionBase* mpIRVol;
	const LAFunctionBase* mpFxVol;
	const LAFunctionBase* mpS;
};

//protected:
	const LARatesNumeraireBase*	mpNumeraireD;	// numerarire of domestic currency
	const LAMathVolFuncFXDD*		mpFxVolatility;	// fx volatility		
	LARatesSpotSDE*				mpSDEFX;		// fx sde 
	LAPriceDriftHW*				mpDriftIR;		// drift function of foregin ir without quant adjust
	DoubleArray					mCorrelation;	// correlation data
	mutable DoubleArray			mMinFx;			// minimum fx for avoid zero divide
	LAString					mSDEAttrNameIR_D;	// data name of ir(domestic currency) sde
	LAString					mSDEAttrNameIR_F;	// data name of ir(foreign currency) sde
	LAString					mSDEAttrNameFX;	// data name of fx sde
	mutable	bool				mIsSetUped;		// setuped or not
	mutable	unsigned int		mPos_old;		// last cache position
	mutable DoubleArray			mCache1;		// cache
	mutable DoubleArray			mCache2;		// cache
	mutable DoubleArray			mCache3;		// cache
	
	double						mFxCriteria;	// fx criteria for avoid zero divide
};
