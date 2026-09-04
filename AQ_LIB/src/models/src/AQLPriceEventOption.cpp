/*! @file
    @brief source code of trigger action class(option occur action).
*/
///
//////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////
#ifdef __GNUG__
#pragma implementation
#endif
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include "AQLPriceEventOption.h"

#include "AQLDataHolder.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataReference.h"
#include "AQLObject.h"
#include "AQLObjectHolder.h"

#include "AQLMathDefine.h"
#include "AQLMathValuableEntity.h"

#include "AQLPricePayOff.h"

using namespace std;



/*!
    @brief Constructor
*/
AQLPriceEventOption::AQLPriceEventOption()
: AQLPriceEventBase()
{

}
/*!
    @brief Destructor

*/
AQLPriceEventOption::~AQLPriceEventOption()
{

}

/*
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*
AQLPriceEventOption::clone() const    
{
    try 
	{
        return new AQLPriceEventOption(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }	
}

/*
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLPriceEventOption::isTypeOf(function_t id) const
{
	return (id == FN_IR_TRIGGERACTIONOPTION ? true : AQLPriceEventBase::isTypeOf(id));
}

/*!
    @brief Return this class type
    @return function type
*/
function_t			
AQLPriceEventOption::getType() const
{
	return FN_IR_TRIGGERACTIONOPTION;
}

/*!
	@brief execute trigger action
	@param[in] actiondate action date
	@param[in] actiontime action time
	@param[in,out] payoff payoff
	@param[in,out] extrapayoff extra payoff
	@param[in,out] futureaction future action
	@param[in,out] pastaction past action
	@param[in,out] iter position of nearest payoff from this action expiry date
*/	
void
AQLPriceEventOption::doAction(const AQLDate& actiondate,
								  double actiontime,
								  vector<PayOffToolHolderVector>& payoff,
									 vector<PayOffToolHolderVector>& extrapayoff,
									 vector<AQLPriceEventHolder*>& futureaction,
									 vector<AQLPriceEventHolder*>& pastaction,
									 vector<PayOffToolHolderIter>& iter)
{
	(void)iter; (void)pastaction; (void)futureaction; (void)payoff; //20070411--Nagase--gcc
	for(unsigned int i = 0; i < mTargetLegNo.size(); i++)
	{
		AQLPricePayOffToolHolder ph(new AQLPricePayOffTool());
		AQLPricePayOffTool& payofftool = ph.getPayOff();
		payofftool.mPaymentDate = actiondate;
		payofftool.mPaymentTime = actiontime;
		payofftool.mExtraCF = mOption->value(actiondate);
		payofftool.calcPayOff();
		extrapayoff[mTargetLegNo[i]].push_back(ph);
	}
}

/*!
	@brief set up this class
    @param[in] basedate basedate
    @param[in] trade trade object
    @param[in,out] triggerinfo trigger/call information object
    @param[in] payoff payoff
    @param[in] isCall call flag(true:call,false:trigger)
*/
void
AQLPriceEventOption::setUp(const AQLDate& basedate,	
									const AQLObject& trade,
									AQLObject& triggerinfo,
									const AQLPricePayOff& payoff,
									bool isCall)
{
	AQLPriceEventBase::setUp(basedate, trade, triggerinfo, payoff, isCall);

	AQLDataHolder* dh;

	//option 
	dh = &(triggerinfo.getData(PRICING_DATA_OPTION, ISNOTNULL));
	AQLDataReference& ref = dynamic_cast<AQLDataReference&>(dh->get());
	mOption = &dynamic_cast<AQLMathObjectValue&>(ref.get().get());

}


