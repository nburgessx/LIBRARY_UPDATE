#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLMathDriftFuncBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLRatesSDEBase.h"


// ID for AQLPriceDriftFX
#define FN_DRIFTFX	2202
// Function name for AQLPriceDriftFX
#define FN_DRIFTFX_STR	"fn_drift_fx"


class AQLMathPathEntity;
class AQLRatesNumeraireBase;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of drift function of fx sde

*/
class AQLPriceDriftFX : public AQLMathDriftFuncBase
{
public:
//  LIFECYCLE
	// Default constructor
	AQLPriceDriftFX(const double s = 0.0, SDE_TYPE type = DIVIDEdXbyX);
	// constructor
	AQLPriceDriftFX(const AQLString& sdeAttrNameD, const AQLString& sdeAttrNameF, const double s = 0.0, SDE_TYPE type = DIVIDEdXbyX);
	//	Copy constructor
//	AQLPriceDriftFX(const AQLPriceDriftFX& v);
	// Destructor
	virtual ~AQLPriceDriftFX();

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
	//furuya
								// Get drift value
	virtual double				getDriftValue(const DoubleArray& x) const;
								//==========================================

								// return string representaion
    virtual AQLString			convertToString(void) const;
	                            //==========================================
	                            // Return integral result
	virtual double				integral(const std::vector<std::pair<double,double> >& x) const;

//	OPERATION
								//==========================================
								// transform from string representaion
     virtual void				convertFromString(const AQLString& str);

								//======================================
								// set up this class
	virtual void				setUp(AQLMathPathEntity& path);
								//======================================
								// set numerarire of domestic currency
								/*!
									@param[in] p pointer to numerarire of domestic currency 
								*/
	void						setDomesticNumerarire(const AQLRatesNumeraireBase* p)
								{mpNumeraireD = p;}
								//======================================
								// set numerarire of foreign currency
								/*!
									@param[in] p pointer to numerarire of foreign currency 
								*/
	void						setForeignNumerarire(const AQLRatesNumeraireBase* p)
								{mpNumeraireF = p;}

private:
	
protected:
	const AQLRatesNumeraireBase*	mpNumeraireD;	// numerarire of domestic currency
 	const AQLRatesNumeraireBase*	mpNumeraireF;	// numerarire of foreign currency
	AQLString					mSDEAttrNameD;	// data name of ir sde of domestic currency
	AQLString					mSDEAttrNameF;	// data name of ir sde of foreign currency
	double						mS;				// parameter of displaced diffusion
	SDE_TYPE					mType;			// sde type

	mutable std::map<double, unsigned int> cache_pos;	// cache position	
	DoubleArray					cache;			// cache 

};

