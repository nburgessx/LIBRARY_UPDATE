#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAMathDriftFuncBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "LARatesSDEBase.h"


// ID for LAPriceDriftFX
#define FN_DRIFTFX	2202
// Function name for LAPriceDriftFX
#define FN_DRIFTFX_STR	"fn_drift_fx"


class LAMathPathEntity;
class LARatesNumeraireBase;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of drift function of fx sde

*/
class LAPriceDriftFX : public LAMathDriftFuncBase
{
public:
//  LIFECYCLE
	// Default constructor
	LAPriceDriftFX(const double s = 0.0, SDE_TYPE type = DIVIDEdXbyX);
	// constructor
	LAPriceDriftFX(const AQLString& sdeAttrNameD, const AQLString& sdeAttrNameF, const double s = 0.0, SDE_TYPE type = DIVIDEdXbyX);
	//	Copy constructor
//	LAPriceDriftFX(const LAPriceDriftFX& v);
	// Destructor
	virtual ~LAPriceDriftFX();

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
	virtual void				setUp(LAMathPathEntity& path);
								//======================================
								// set numerarire of domestic currency
								/*!
									@param[in] p pointer to numerarire of domestic currency 
								*/
	void						setDomesticNumerarire(const LARatesNumeraireBase* p)
								{mpNumeraireD = p;}
								//======================================
								// set numerarire of foreign currency
								/*!
									@param[in] p pointer to numerarire of foreign currency 
								*/
	void						setForeignNumerarire(const LARatesNumeraireBase* p)
								{mpNumeraireF = p;}

private:
	
protected:
	const LARatesNumeraireBase*	mpNumeraireD;	// numerarire of domestic currency
 	const LARatesNumeraireBase*	mpNumeraireF;	// numerarire of foreign currency
	AQLString					mSDEAttrNameD;	// data name of ir sde of domestic currency
	AQLString					mSDEAttrNameF;	// data name of ir sde of foreign currency
	double						mS;				// parameter of displaced diffusion
	SDE_TYPE					mType;			// sde type

	mutable std::map<double, unsigned int> cache_pos;	// cache position	
	DoubleArray					cache;			// cache 

};

