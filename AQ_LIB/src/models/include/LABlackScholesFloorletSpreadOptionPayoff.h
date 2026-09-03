#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAFunctionBase.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include "LABlackScholesCapletSpreadOptionPayoff.h"



// Funciton ID of LABlackScholesBaseMethod
#define FN_IR_FLOORLETSPREADOPTIONFUNC	1267
// Function Name of LABlackScholesBaseMethod
#define FN_IR_FLOORLETSPREADOPTIONFUNC_STR	"fn_ir_floorletspreadoptionfunc"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class declaration to represent linear function.

	This class derives from LABlackScholesBaseMethod

*/
class LABlackScholesFloorletSpreadOption : public LABlackScholesCapletSpreadOption
{
public:
//  LIFECYCLE
	// constructor
	LABlackScholesFloorletSpreadOption();
	// destructor
	virtual ~LABlackScholesFloorletSpreadOption();

	//20061017--David--Remove warning:C4512
	LABlackScholesFloorletSpreadOption & operator=( const LABlackScholesFloorletSpreadOption & ) { return *this; }


//  QUERY
								//======================================
								// Check function for this class type
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
    virtual LACoreFunctionBase*     clone() const;
								//======================================
								// Return this class type
    virtual function_t          getType() const;
								
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

	virtual LAStringVector getOptionTypeVector(); 

	virtual LAStringVector getBSComponentVector(LAString risktype) const;

	virtual LAStringVector getBSPayoffComponentVector(LAString risktype) const;
	
	//virtual void setOptionTerm(double Te);
	
	//virtual void setOptionNumeraire(void);


//  OPERATION

private:

protected:
	

};
