#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "LARatesSDEBase.h"
#include "LAModelDynamicsBase.h"


// ID for LARatesSDEIntegralBase
#define FN_SDEINTEGRALBASE	1801 


// sde type
enum SDEINTEGRAL_TYPE 
{	
	NORMAL_INTEGRAL,
	LOG_INTEGRAL,
	LOG_INTEGRAL_LOG_OUTPUT
};

///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of abstract base class of sde integral class

*/
class LARatesSDEIntegralBase : public AQLCoreFunctionBase
{
public:
//  LIFECYCLE
	// Default constructor
	explicit LARatesSDEIntegralBase(SDEINTEGRAL_TYPE type);
	// Default constructor
	explicit LARatesSDEIntegralBase(SDEINTEGRAL_TYPE type, const AQLString& sdeAttrName);
	//	Copy constructor
	LARatesSDEIntegralBase(const LARatesSDEIntegralBase& v);
	// Destructor
	virtual ~LARatesSDEIntegralBase();

//  QUERY
								//======================================
								// Check function for this class ID
	virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual AQLCoreFunctionBase*		clone() const = 0;// %%% COVARIANT RETURN %%%
								//======================================
								// Return this class ID
	virtual function_t			getType() const;
								//======================================
								// get integral type
								/*!
									@return integral type
								*/
	SDEINTEGRAL_TYPE			getIntegralType() const {return mIntegralType;}		
								//======================================
								// excecute integral
	virtual void				integral(double ts, double te, 
										std::vector<AQLFunctionBase*>::const_iterator drift,										
										std::vector<std::vector<AQLFunctionBase*> >::const_iterator vol,
										DoubleArray::const_iterator	bm,
										SCALARARRAY::iterator	x_in_out,
										unsigned int varnum
								) const = 0;


// OPERATION
	// set sde type
	/*!
		@param[in] type sde type
	*/
	void						setSDEType(SDE_TYPE type) {mSdeType = type;}
	
private:

protected:
	SDE_TYPE					mSdeType;				// sde type
	SDEINTEGRAL_TYPE			mIntegralType;			// integral type
	AQLString					mSDEAttrName;			// data name of sde


};

