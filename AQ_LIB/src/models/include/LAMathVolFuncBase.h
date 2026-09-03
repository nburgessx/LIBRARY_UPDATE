#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLDataInstance.h"
#include "AQLObjectPool.h"
#include "LAMathFXEntity.h"
#include "AQLDataMultiReference.h"
#include "LAMathYieldCurve.h"

#ifndef VISUAL_STUDIO_2010_ANALYTICS
#include "LAMathVolFuncFX.h"
#include "LAMathVolFuncFXDD.h"
#include "LAMathVolFuncSZDD.h"
#endif

#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"


// ID for LAMathVolFuncBase
#define FN_VOLFUNCBASE	2301 
// Function name for LAMathVolFuncBase
#define FN_VOLFUNCBASE_STR	"fn_volfuncbase"



class LAMathPathEntity;
class AQL1DIntegral;
class AQLDataInstance;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of base class of volatility function class

*/
class LAMathVolFuncBase : public AQLFunctionBase
{
public:
//  LIFECYCLE
	// Default constructor
	explicit LAMathVolFuncBase(bool isMultiVariables = false);
	// constructor
	LAMathVolFuncBase(const AQLString& sdeAttrName, unsigned int i, unsigned int j = 0, bool isMultiVariables = false);
	//	Copy constructor
	LAMathVolFuncBase(const LAMathVolFuncBase& v);
	// Destructor
	virtual ~LAMathVolFuncBase();

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
								//======================================
								// get volatility
	const AQLFunctionBase*		getVolatility() const {return mpVolatility;}
	                            //==========================================
	                            // Return volatility value
	virtual double				operator()(const DoubleArray& x) const;
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
								/*!
									@param[in] path path object 
								*/
	virtual void				setUp(LAMathPathEntity& path);
								//======================================
								// set suffix
								/*!
									@param[in] i row suffix
									@param[in] j column suffix
								*/
	void						setSuffix(unsigned int i, unsigned int j = 0) {m_i = i; m_j = j;}

private:
	                            //==========================================
	                            // Return integral result
	double						integral(double t1, double t2, AQL1DIntegral* pIntegral) const;
protected:
	unsigned int								m_i;			// suffix
	unsigned int								m_j;			// suffix
	AQLFunctionBase*								mpVolatility;	// volatility
	AQLString									mSDEAttrName;	// data name of sde
	bool										mIsMultiVariables;// flag of volatilit function depend multi variables or not
	const DoubleArray*							mpTimes;		// volatility data time
	DoubleArray									mVolData;		// volatility data
	DoubleArray									mIntegratedVolData;	// integral of volatility^2 data
	mutable	unsigned int						mPos_old;		// tempolary variable	

};

