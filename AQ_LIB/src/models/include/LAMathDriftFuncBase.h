#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAFunctionBase.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"


// ID for LAMathDriftFuncBase
#define FN_DRIFTFUNCBASE	2201 

class LAMathPathEntity;

///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of abstract base class of drift function class

*/
class LAMathDriftFuncBase : public LAFunctionBase
{
public:
//  LIFECYCLE
	// Default constructor
	LAMathDriftFuncBase();
	//	Copy constructor
//	LAMathDriftFuncBase(const LAMathDriftFuncBase& v);
	// Destructor
	virtual ~LAMathDriftFuncBase();

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
	virtual void				setUp(LAMathPathEntity& path) = 0;
private:

protected:

};
