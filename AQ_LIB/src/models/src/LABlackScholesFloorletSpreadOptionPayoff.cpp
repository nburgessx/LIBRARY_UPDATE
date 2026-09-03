/*! @file
    @brief Source code of class to represent linear function

    This class derives from LABlackScholesBaseMethod

*/
//  2010, AlgoQuantHub.

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
#include "AQLBasic.h"
#include "AQLDist.h"
#include "LAAnalyticFormula.h"
#include "AQLCoreComponentManager.h"

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
AQLCoreFunctionBase*	
LABlackScholesFloorletSpreadOption::clone() const
{
    try 
	{
		return new LABlackScholesFloorletSpreadOption(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
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


AQLStringVector 
LABlackScholesFloorletSpreadOption::getOptionTypeVector()
{
	AQLStringVector ret(2);
	ret[0] = AQLString(PUT);
	ret[1] = AQLString(PUT);

	return ret;
}


AQLStringVector 
LABlackScholesFloorletSpreadOption::getBSComponentVector(AQLString risktype) const
{
	AQLStringVector ret(2);
	ret[0] = AQLString(BK) + risktype + AQLString(PUT);
	ret[1] = AQLString(BK) + risktype + AQLString(PUT);

	return ret;
}


AQLStringVector 
LABlackScholesFloorletSpreadOption::getBSPayoffComponentVector(AQLString risktype) const
{
	AQLStringVector ret(2);
	ret[0] = AQLString(BKPAYOFF) + risktype + AQLString(PUT);
	ret[1] = AQLString(BKPAYOFF) + risktype + AQLString(PUT);
	return ret;
}

