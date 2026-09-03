#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLModelDynamicsBase.h"


// ID for AQLRatesPEInterpolationBase
#define FN_PEINTERPOLATIONBASE	1901 


///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of abstract base class of path element interpolation class

*/
class AQLRatesPEInterpolationBase : public AQLCoreFunctionBase
{
public:
//  LIFECYCLE
	// Default constructor
	AQLRatesPEInterpolationBase();
	//	Copy constructor
	AQLRatesPEInterpolationBase(const AQLRatesPEInterpolationBase& v);
	// Destructor
	virtual ~AQLRatesPEInterpolationBase();

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
								// Return interpolated value
	virtual const AQLRatesPathElementBase&
								value(double t,	double t1, double t2,
										const AQLRatesPathElementBase& val1,
										const AQLRatesPathElementBase& val2) = 0;
								//======================================
								// initialize this class
	virtual void				init() {;}
// OPERATION

private:

protected:


};

