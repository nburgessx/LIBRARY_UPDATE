/*! @file
    @brief Source code of class to represent linear function

    This class derives from LABlackScholesBaseMethod

*/
//  2010, Mizuho International London.

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LABlackScholesFloorletSpreadOption.cpp
//
//  SYNOPSIS    :       LABlackScholesFloorletSpreadOption
//  DESCRIPTION :       Source code of class to represent function led by LABlackScholesBase class
//                      
//  SEE ALSO    :       
//  TYME        :       CLASS
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LABlackScholesFloorletSpreadOptionPayoff.h"
#include "LABasic.h"
#include "LADist.h"
#include "LAAnalyticFormula.h"
#include "LACoreComponentManager.h"

using namespace std;
//================ LABlackScholesFloorletSpreadOption ===================================
/*!
	@brief default constructor
*/
LABlackScholesFloorletSpreadOption::LABlackScholesFloorletSpreadOption() 
: LABlackScholesCapletSpreadOption()
{

}

/*!
	@brief destructor
*/
LABlackScholesFloorletSpreadOption::~LABlackScholesFloorletSpreadOption() 
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LABlackScholesFloorletSpreadOption::clone() const
{
    try 
	{
		return new LABlackScholesFloorletSpreadOption(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LABlackScholesFloorletSpreadOption::isTypeOf(function_t id) const
{
	return (id == FN_IR_FLOORLETSPREADOPTIONFUNC ? true : LABlackScholesCapletOption::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LABlackScholesFloorletSpreadOption::getType() const
{
	return FN_IR_FLOORLETSPREADOPTIONFUNC;
}


LAStringVector 
LABlackScholesFloorletSpreadOption::getOptionTypeVector()
{
	LAStringVector ret(2);
	ret[0] = LAString(PUT);
	ret[1] = LAString(PUT);

	return ret;
}


LAStringVector 
LABlackScholesFloorletSpreadOption::getBSComponentVector(LAString risktype) const
{
	LAStringVector ret(2);
	ret[0] = LAString(BK) + risktype + LAString(PUT);
	ret[1] = LAString(BK) + risktype + LAString(PUT);

	return ret;
}


LAStringVector 
LABlackScholesFloorletSpreadOption::getBSPayoffComponentVector(LAString risktype) const
{
	LAStringVector ret(2);
	ret[0] = LAString(BKPAYOFF) + risktype + LAString(PUT);
	ret[1] = LAString(BKPAYOFF) + risktype + LAString(PUT);
	return ret;
}

