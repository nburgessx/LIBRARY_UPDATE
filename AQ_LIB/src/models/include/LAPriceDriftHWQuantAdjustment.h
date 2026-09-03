#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAMathDriftFuncBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
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
	LAPriceDriftHWQuantAdjustment(const AQLString& sdeAttrNameIR_D, const AQLString& sdeAttrNameIR_F, const AQLString& sdeAttrNameFX, LAPriceDriftHW* driftIR, double fx_criteria = 0.01);	
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
	LAPriceDriftHW*				getDrift() {return mpDriftIR;}

//	OPERATION
								//==========================================
								// transform from string representaion
     virtual void				convertFromString(const AQLString& str);

								//======================================
								// set up this class
	 virtual void				setUp(LAMathPathEntity& path);


protected:
	void						setUp() const;


class MMHWQAdjInnerFunc1 : public AQLFunctionBase
{
public:
//  LIFECYCLE
	// constructor
	MMHWQAdjInnerFunc1(const LAMathHWFuncMR& mr, const AQLFunctionBase& irvol, const AQLFunctionBase& fxvol)
		:mpMR(&mr), mpIRVol(&irvol), mpFxVol(&fxvol) {;}
	// destructor
	virtual ~MMHWQAdjInnerFunc1() {;}


//  QUERY

								//======================================
								// Make copy(clone) of this class
	virtual AQLCoreFunctionBase*     clone() const {return new MMHWQAdjInnerFunc1(*this);}
	                            //==========================================
	                            // return function value
	virtual double				operator()(const DoubleArray& x) const {return operator()(x[0]);}

	//20061017--David--Remove warning:C4512
	MMHWQAdjInnerFunc1 & operator=( const MMHWQAdjInnerFunc1 & ) { return *this; }

	virtual double				operator()(double x) const
								{
									return AQLMath::exp(mpMR->integrate(0.0, x)) * (*mpIRVol)(x) * (*mpFxVol)(x);	
								}
private:
	const LAMathHWFuncMR*	mpMR;
	const AQLFunctionBase* mpIRVol;
	const AQLFunctionBase* mpFxVol;
};

class MMHWQAdjInnerFunc2 : public AQLFunctionBase
{
public:
//  LIFECYCLE
	// constructor
	MMHWQAdjInnerFunc2(const LAMathHWFuncMR& mr, const AQLFunctionBase& irvol, const AQLFunctionBase& fxvol, const AQLFunctionBase& s)
		:mpMR(&mr), mpIRVol(&irvol), mpFxVol(&fxvol), mpS(&s) {;}
	// destructor
	virtual ~MMHWQAdjInnerFunc2() {;}


//  QUERY

								//======================================
								// Make copy(clone) of this class
	virtual AQLCoreFunctionBase*     clone() const {return new MMHWQAdjInnerFunc2(*this);}
	                            //==========================================
	                            // return function value
	virtual double				operator()(const DoubleArray& x) const {return operator()(x[0]);}

	//20061017--David--Remove warning:C4512
	MMHWQAdjInnerFunc2 & operator=( const MMHWQAdjInnerFunc2 & ) { return *this; }

	virtual double				operator()(double x) const
								{
									return AQLMath::exp(mpMR->integrate(0.0, x)) * (*mpIRVol)(x) * (*mpFxVol)(x) * (*mpS)(x);	
								}
private:
	const LAMathHWFuncMR*	mpMR;
	const AQLFunctionBase* mpIRVol;
	const AQLFunctionBase* mpFxVol;
	const AQLFunctionBase* mpS;
};

class MMHWQAdjInnerFunc3 : public AQLFunctionBase
{
public:
//  LIFECYCLE
	// constructor
	MMHWQAdjInnerFunc3(const LAMathHWFuncMR& mr, const AQLFunctionBase& irvol, const AQLFunctionBase& fxvol, const AQLFunctionBase& s)
		:mpMR(&mr), mpIRVol(&irvol), mpFxVol(&fxvol), mpS(&s) {;}
	// destructor
	virtual ~MMHWQAdjInnerFunc3() {;}


//  QUERY

								//======================================
								// Make copy(clone) of this class
	virtual AQLCoreFunctionBase*     clone() const {return new MMHWQAdjInnerFunc3(*this);}
	                            //==========================================
	                            // return function value
	virtual double				operator()(const DoubleArray& x) const {return operator()(x[0]);}

	//20061017--David--Remove warning:C4512
	MMHWQAdjInnerFunc3 & operator=( const MMHWQAdjInnerFunc3 & ) { return *this; }

	virtual double				operator()(double x) const
								{
									return AQLMath::exp(mpMR->integrate(0.0, x)) * (*mpIRVol)(x) * (*mpFxVol)(x) * (*mpS)(x) * x;	
								}
private:
	const LAMathHWFuncMR*	mpMR;
	const AQLFunctionBase* mpIRVol;
	const AQLFunctionBase* mpFxVol;
	const AQLFunctionBase* mpS;
};

//protected:
	const LARatesNumeraireBase*	mpNumeraireD;	// numerarire of domestic currency
	const LAMathVolFuncFXDD*		mpFxVolatility;	// fx volatility		
	LARatesSpotSDE*				mpSDEFX;		// fx sde 
	LAPriceDriftHW*				mpDriftIR;		// drift function of foregin ir without quant adjust
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
