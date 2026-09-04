/*! @file
    @brief Source code of class to represent linear function

    This class derives from AQLBlackScholesBaseMethod

*/

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLBlackScholesFloorletOptionPayoff.h"
#include "AQLBasic.h"
#include "AQLDist.h"
#include "AQLAnalyticFormula.h"
#include "AQLCoreComponentManager.h"

using namespace std;
//================ AQLBlackScholesFloorletOption ===================================
/*!
	@brief default constructor
*/
AQLBlackScholesFloorletOption::AQLBlackScholesFloorletOption() 
: AQLBlackScholesCapletOption()
{

}

/*!
	@brief destructor
*/
AQLBlackScholesFloorletOption::~AQLBlackScholesFloorletOption() 
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLBlackScholesFloorletOption::clone() const
{
    try 
	{
		return new AQLBlackScholesFloorletOption(*this);
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
AQLBlackScholesFloorletOption::isTypeOf(function_t id) const
{
	return (id == FN_IR_FLOORLETOPTIONFUNC ? true : AQLBlackScholesCapletOption::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLBlackScholesFloorletOption::getType() const
{
	return FN_IR_FLOORLETOPTIONFUNC;
}


AQLStringVector
AQLBlackScholesFloorletOption::getOptionTypeVector()
{
	AQLStringVector ret(1);
	ret[0] = AQLString(PUT);
	mOptionType = ret;
	return ret;
}

AQLStringVector 
AQLBlackScholesFloorletOption::getBSComponentVector(AQLString risktype) const
{
	AQLStringVector ret(1);
	ret[0] = AQLString(BK) + risktype + AQLString(PUT);
	return ret;
}


AQLStringVector 
AQLBlackScholesFloorletOption::getBSPayoffComponentVector(AQLString risktype) const
{
	AQLStringVector ret(1);
	ret[0] = AQLString(BKPAYOFF) + risktype + AQLString(PUT);
	return ret;
}


