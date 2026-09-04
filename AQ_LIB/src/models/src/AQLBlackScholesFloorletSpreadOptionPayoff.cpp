/*! @file
    @brief Source code of class to represent linear function

    This class derives from AQLBlackScholesBaseMethod

*/

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLBlackScholesFloorletSpreadOptionPayoff.h"
#include "AQLBasic.h"
#include "AQLDist.h"
#include "AQLAnalyticFormula.h"
#include "AQLCoreComponentManager.h"

using namespace std;
//================ AQLBlackScholesFloorletSpreadOption ===================================
/*!
	@brief default constructor
*/
AQLBlackScholesFloorletSpreadOption::AQLBlackScholesFloorletSpreadOption() 
: AQLBlackScholesCapletSpreadOption()
{

}

/*!
	@brief destructor
*/
AQLBlackScholesFloorletSpreadOption::~AQLBlackScholesFloorletSpreadOption() 
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLBlackScholesFloorletSpreadOption::clone() const
{
    try 
	{
		return new AQLBlackScholesFloorletSpreadOption(*this);
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
AQLBlackScholesFloorletSpreadOption::isTypeOf(function_t id) const
{
	return (id == FN_IR_FLOORLETSPREADOPTIONFUNC ? true : AQLBlackScholesCapletOption::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLBlackScholesFloorletSpreadOption::getType() const
{
	return FN_IR_FLOORLETSPREADOPTIONFUNC;
}


AQLStringVector 
AQLBlackScholesFloorletSpreadOption::getOptionTypeVector()
{
	AQLStringVector ret(2);
	ret[0] = AQLString(PUT);
	ret[1] = AQLString(PUT);

	return ret;
}


AQLStringVector 
AQLBlackScholesFloorletSpreadOption::getBSComponentVector(AQLString risktype) const
{
	AQLStringVector ret(2);
	ret[0] = AQLString(BK) + risktype + AQLString(PUT);
	ret[1] = AQLString(BK) + risktype + AQLString(PUT);

	return ret;
}


AQLStringVector 
AQLBlackScholesFloorletSpreadOption::getBSPayoffComponentVector(AQLString risktype) const
{
	AQLStringVector ret(2);
	ret[0] = AQLString(BKPAYOFF) + risktype + AQLString(PUT);
	ret[1] = AQLString(BKPAYOFF) + risktype + AQLString(PUT);
	return ret;
}

