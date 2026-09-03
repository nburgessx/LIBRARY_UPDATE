/*! @file
    @brief Source code of class to represent linear function

    This class derives from LABlackScholesBaseMethod

*/
//  2010, AlgoQuantHub.

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LABlackScholesFloorletOption.cpp
//
//  SYNOPSIS    :       LABlackScholesFloorletOption
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


#include "LABlackScholesFloorletOptionPayoff.h"
#include "AQLBasic.h"
#include "AQLDist.h"
#include "LAAnalyticFormula.h"
#include "AQLCoreComponentManager.h"

using namespace std;
//================ LABlackScholesFloorletOption ===================================
/*!
	@brief default constructor
*/
LABlackScholesFloorletOption::LABlackScholesFloorletOption() 
: LABlackScholesCapletOption()
{

}

/*!
	@brief destructor
*/
LABlackScholesFloorletOption::~LABlackScholesFloorletOption() 
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
LABlackScholesFloorletOption::clone() const
{
    try 
	{
		return new LABlackScholesFloorletOption(*this);
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
LABlackScholesFloorletOption::isTypeOf(function_t id) const
{
	return (id == FN_IR_FLOORLETOPTIONFUNC ? true : LABlackScholesCapletOption::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LABlackScholesFloorletOption::getType() const
{
	return FN_IR_FLOORLETOPTIONFUNC;
}


AQLStringVector
LABlackScholesFloorletOption::getOptionTypeVector()
{
	AQLStringVector ret(1);
	ret[0] = AQLString(PUT);
	mOptionType = ret;
	return ret;
}

AQLStringVector 
LABlackScholesFloorletOption::getBSComponentVector(AQLString risktype) const
{
	AQLStringVector ret(1);
	ret[0] = AQLString(BK) + risktype + AQLString(PUT);
	return ret;
}


AQLStringVector 
LABlackScholesFloorletOption::getBSPayoffComponentVector(AQLString risktype) const
{
	AQLStringVector ret(1);
	ret[0] = AQLString(BKPAYOFF) + risktype + AQLString(PUT);
	return ret;
}


