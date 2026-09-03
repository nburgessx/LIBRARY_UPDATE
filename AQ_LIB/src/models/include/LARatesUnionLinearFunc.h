#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAFunctionBase.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
 


// Funciton ID of LARatesUnionLinearFunc
#define FN_UNIONLINEARFUNC	10007
// Function Name of LARatesUnionLinearFunc
#define FN_UNIONLINEARFUNC_STR	"fn_unionlinearfunc"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class declaration to represent RangeCount function.

	This class derives from LAFunctionBase

*/
class LARatesUnionLinearFunc : public LAFunctionBase
{
public:
//  LIFECYCLE
	// constructor
	LARatesUnionLinearFunc();
	// destructor
	virtual ~LARatesUnionLinearFunc();

	//20061017--David--Remove warning:C4512
	LARatesUnionLinearFunc & operator=( const LARatesUnionLinearFunc & ) { return *this; }


//  QUERY
								//======================================
								// Check function for this class type
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
    virtual LACoreFunctionBase*     clone() const;// %%% COVARIANT RETURN %%%
								//======================================
								// Return this class type
    virtual function_t          getType() const;
								
	                            //==========================================
								// return function value
	virtual double				operator()(const DoubleArray& x) const;
	                            //==========================================
								// return function value
	virtual double				operator()(double x) const
								{
									return LAFunctionBase::operator()(x);								
								}


	virtual	LAFunctionBase&		operator()(unsigned int pos, const DoubleArray& x)
								{
									return LAFunctionBase::operator()(pos, x);
								};

	
//  OPERATION

private:

protected:
	

};

