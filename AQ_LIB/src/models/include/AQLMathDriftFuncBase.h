#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"


// ID for AQLMathDriftFuncBase
#define FN_DRIFTFUNCBASE	2201 

class AQLMathPathEntity;

///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of abstract base class of drift function class

*/
class AQLMathDriftFuncBase : public AQLFunctionBase
{
public:
//  LIFECYCLE
	// Default constructor
	AQLMathDriftFuncBase();
	//	Copy constructor
//	AQLMathDriftFuncBase(const AQLMathDriftFuncBase& v);
	// Destructor
	virtual ~AQLMathDriftFuncBase();

//  QUERY
								//======================================
								// Check function for this class ID
	virtual bool                isTypeOf(function_t id) const;

								//======================================
								// Return this class ID
	virtual function_t			getType() const;

//	OPERATION
								//======================================
								// set up this class
								/*!
									@param[in] path path object 
								*/
	virtual void				setUp(AQLMathPathEntity& path) = 0;
private:

protected:

};
