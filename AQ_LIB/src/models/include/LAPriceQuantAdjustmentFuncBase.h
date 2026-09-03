#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"


// ID for LAPriceQuantAdjustmentFuncBase
#define FN_QUANTADJUSTMENTFUNCBASE	30001 

class LAMathPathEntity;

///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of abstract base class of quanto adjustment function class

*/
class LAPriceQuantAdjustmentFuncBase : public AQLFunctionBase
{
public:
//  LIFECYCLE
	// Default constructor
	LAPriceQuantAdjustmentFuncBase();
	//	Copy constructor
//	LAPriceQuantAdjustmentFuncBase(const LAPriceQuantAdjustmentFuncBase& v);
	// Destructor
	virtual ~LAPriceQuantAdjustmentFuncBase();

//  QUERY
								//======================================
								// Check function for this class ID
	virtual bool                isTypeOf(function_t id) const;

								//======================================
								// Return this class ID
	virtual function_t			getType() const;

	virtual double				operator()(const DoubleArray& x) const { return getQuantAdjust(x[0]); }
//	OPERATION
								//======================================
								// set up this class
								/*!
									@param[in] path path object 
								*/
	virtual void				setUp(LAMathPathEntity& path) = 0;
								//======================================
								// get quanto adjustment
	virtual double 				getQuantAdjust(const double ts) const = 0;
private:

protected:

};
