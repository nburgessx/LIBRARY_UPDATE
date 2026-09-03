#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAMathDriftFuncBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"


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
	explicit LAPriceDriftQuantAdjustment(AQLFunctionBase* driftIR);
	// constructor
	LAPriceDriftQuantAdjustment(const AQLString& sdeAttrNameIR, const AQLString& sdeAttrNameFX, unsigned int i, AQLFunctionBase* driftIR);	
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
	AQLFunctionBase*				getDrift() {return mpDriftIR;}
	                            //==========================================
	                            // Return integral result
	virtual double				integral(const std::vector<std::pair<double,double> >& x) const;
//	OPERATION
								//==========================================
								// transform from string representaion
     virtual void				convertFromString(const AQLString& str);

								//======================================
								// set up this class
	virtual void				setUp(LAMathPathEntity& path);
								//======================================
								// set ir(foreign currency) volatility
								/*!
									@param[in] pvol volatility
								*/
    void						setIRVolaility(const AQLFunctionBase* pvol) 
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
	double						integral(unsigned int pos_s, AQL1DIntegral* pIntegral) const;
	
protected:
	const AQLFunctionBase*						mpVolatility;	// ir(foreign currency) volatility  
	DoubleArray									mCorrelation;	// correlation between fx and ir of foreign currency
	LARatesSpotSDE*								mpSDEFX;		// fx sde 
	const AQLFunctionBase*						mpFxVolatility;	// fx volatility
	unsigned int								m_i;			// suffix	
	AQLString									mSDEAttrNameIR;	// data name of ir(foreign currency) sde
	AQLString									mSDEAttrNameFX;	// data name of fx sde
	AQLFunctionBase*								mpDriftIR;		// drift function of ir without quant adjust
	mutable DoubleArray							mIntegratedData;// integral of t part data
	mutable	unsigned int						mPos_old;		// tempolary variable	
	mutable double								mInitialFxVol;	// Initail value of fx and t part of fx vol
	mutable double*								mpVar;			// cache
	mutable unsigned int*						mpVarPos;		// cache
	mutable double*								mpVar2;			// cache
	mutable unsigned int*						mpVarPos2;		// cache

};
