/*! @file
    @brief source code of accrued interest calculator class.

*/
//  2007, AlgoQuantHub..
///

#ifdef __GNUG__
#pragma implementation
#endif
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include "LAPriceAccruedInterest.h"
#include "LAPriceTradeValue.h"
#include "AQLObject.h"



using namespace std;





/*!
    @brief default constructor
*/
LAPriceAccruedInterest::LAPriceAccruedInterest()
: AQLCoreFunctionBase(), mpPayOff(NULL)
{

}
/*!
    @brief destructor

*/
LAPriceAccruedInterest::~LAPriceAccruedInterest()
{

}

/*!
    @brief copy constructor

	@param[in] v LAPriceAccruedInterest object
*/
LAPriceAccruedInterest::LAPriceAccruedInterest(const LAPriceAccruedInterest& v)
: AQLCoreFunctionBase(v), mpPayOff(v.mpPayOff)
{

}




/*
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LAPriceAccruedInterest::isTypeOf(function_t id) const
{
	return (id == FN_IR_ACCRUEDINTEREST ? true : AQLCoreFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this class type
    @return function type
*/
function_t			
LAPriceAccruedInterest::getType() const
{
	return FN_IR_ACCRUEDINTEREST;
}

/*
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*
LAPriceAccruedInterest::clone() const
{
    try 
	{
        return new LAPriceAccruedInterest(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }	
}

/*!
	@brief calculate accrued interest

	@param[in] basedate basedate
	@return accrued interest
*/
double
LAPriceAccruedInterest::calcAccruedInterest(const AQLDate& basedate, unsigned int legno) const
{
	const vector<PayOffToolHolderVector>& payoff_vec = mpPayOff->getPayOff();
	unsigned int size = payoff_vec.at(legno).size();

	double ret = 0;
	for (unsigned int i = 0; i < size; i++)
	{
		const LAPricePayOffTool& ptool = payoff_vec[legno][i].getPayOff();
		if (!ptool.isCouponPayment()) continue;
		if (ptool.getPaymentDate() <= basedate) continue;
		if (ptool.mStart >= basedate) break;

		ret += ptool.calcAccruedInt(basedate);
	}
		
	return ret;

}


/*!
	@brief set up for payoff calculation

	@param[in] basedate basedate
	@param[in, out] trade trade object 
	@param[in] att Data to hold evaluation class
*/
void	            
LAPriceAccruedInterest::setUp(const AQLDate& basedate, AQLObject& trade, const AQLDataValuation& att) 
{
	(void)basedate; (void)trade; (void)att; //20070411--Nagase--gcc
	if (att.isNullDataProvider())
	{
		//error
		throw AQLCoreInvalidData("AQLDataProvider class does not exist", __FILE__, __LINE__);	
	}	
	LAPriceTradeValue::LAPriceTradeValueDataProvider* dataProvider = 
		&dynamic_cast<LAPriceTradeValue::LAPriceTradeValueDataProvider&>(att.getDataProvider());
	mpPayOff = &dataProvider->payoff;

}

