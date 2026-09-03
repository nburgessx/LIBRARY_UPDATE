#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAMathDriftFuncBase.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"


// ID for LAPriceDriftQuantAdjustment
#define FN_DRIFTQUANTADJ	2205
// Function name for LAPriceDriftQuantAdjustment
#define FN_DRIFTQUANTADJ_STR	"fn_drift_quantadj"


class LAMathPathEntity;
class LARatesNumeraireBase;
class LARatesSpotSDE;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of drift function of quant adjustment

*/
class LAPriceDriftQuantAdjustment : public LAMathDriftFuncBase
{
public:
//  LIFECYCLE
	// Default constructor
	explicit LAPriceDriftQuantAdjustment(LAFunctionBase* driftIR);
	// constructor
	LAPriceDriftQuantAdjustment(const LAString& sdeAttrNameIR, const LAString& sdeAttrNameFX, unsigned int i, LAFunctionBase* driftIR);	
	//	Copy constructor
	LAPriceDriftQuantAdjustment(const LAPriceDriftQuantAdjustment& v);
	// Destructor
	virtual ~LAPriceDriftQuantAdjustment();

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
	LAFunctionBase*				getDrift() {return mpDriftIR;}
	                            //==========================================
	                            // Return integral result
	virtual double				integral(const std::vector<std::pair<double,double> >& x) const;
//	OPERATION
								//==========================================
								// transform from string representaion
     virtual void				convertFromString(const LAString& str);

								//======================================
								// set up this class
	virtual void				setUp(LAMathPathEntity& path);
								//======================================
								// set ir(foreign currency) volatility
								/*!
									@param[in] pvol volatility
								*/
    void						setIRVolaility(const LAFunctionBase* pvol) 
								{mpVolatility = pvol;}
								//======================================
								// set correlation between fx and ir of foreign currency
								/*!
									@param[in] cor correlation
								*/
	void						setCorrelation(const DoubleArray& cor)
								{mCorrelation = cor;}
								//======================================
								// set fx sde
	void						setFXSDE(LARatesSpotSDE* psde);
								//======================================
								// set suffix
								/*!
									@param[in] i suffix
								*/
	void						setSuffix(unsigned int i) {m_i = i;}

private:
	                            //==========================================
	                            // Return integral result
	double						integral(unsigned int pos_s, LA1DIntegral* pIntegral) const;
	
protected:
	const LAFunctionBase*						mpVolatility;	// ir(foreign currency) volatility  
	DoubleArray									mCorrelation;	// correlation between fx and ir of foreign currency
	LARatesSpotSDE*								mpSDEFX;		// fx sde 
	const LAFunctionBase*						mpFxVolatility;	// fx volatility
	unsigned int								m_i;			// suffix	
	LAString									mSDEAttrNameIR;	// data name of ir(foreign currency) sde
	LAString									mSDEAttrNameFX;	// data name of fx sde
	LAFunctionBase*								mpDriftIR;		// drift function of ir without quant adjust
	mutable DoubleArray							mIntegratedData;// integral of t part data
	mutable	unsigned int						mPos_old;		// tempolary variable	
	mutable double								mInitialFxVol;	// Initail value of fx and t part of fx vol
	mutable double*								mpVar;			// cache
	mutable unsigned int*						mpVarPos;		// cache
	mutable double*								mpVar2;			// cache
	mutable unsigned int*						mpVarPos2;		// cache

};
