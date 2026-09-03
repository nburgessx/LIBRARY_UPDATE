/*! @file
    @brief source code of trigger action class(notional exchange currency change action).

*/
//  2006, AlgoQuantHub..
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

#include "AQLPriceEventNotExCurChange.h"

#include "AQLDataHolder.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataReference.h"
#include "AQLObjectHolder.h"
#include "AQLMathDefine.h"
#include "AQLPriceDataCalendar.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLPriceDataFunction.h"
#include "AQLMathFXEntity.h"
#include "AQLMathDateCalculations.h"

using namespace std;

/*!
    @brief Constructor
*/
AQLPriceEventNotExCurChange::AQLPriceEventNotExCurChange()
: AQLPriceEventBase()
, mpFX(0), mpCalendar(0), mpSlidingRule(0) 
{

}
/*!
    @brief Destructor

*/
AQLPriceEventNotExCurChange::~AQLPriceEventNotExCurChange()
{

}

/*
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*
AQLPriceEventNotExCurChange::clone() const    
{
    try 
	{
        return new AQLPriceEventNotExCurChange(*this);
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
AQLPriceEventNotExCurChange::isTypeOf(function_t id) const
{
	return (id == FN_IR_TRIGGERACTIONNOTEXCURCHANGE ? true : AQLPriceEventBase::isTypeOf(id));
}

/*!
    @brief Return this class type
    @return function type
*/
function_t			
AQLPriceEventNotExCurChange::getType() const
{
	return FN_IR_TRIGGERACTIONNOTEXCURCHANGE;
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
AQLPriceEventNotExCurChange::doAction(const AQLDate& actiondate,
										double actiontime,
										vector<PayOffToolHolderVector>& payoff,
										vector<PayOffToolHolderVector>& extrapayoff,
										vector<AQLPriceEventHolder*>& futureaction,
										vector<AQLPriceEventHolder*>& pastaction,
										vector<PayOffToolHolderIter>& iter)
{
	(void)actiontime; (void)pastaction; (void)futureaction; //20070411--Nagase--gcc
	PayOffToolHolderIter it;
	AQLDate fixingdate;
	if (mIsFixingDate)
		fixingdate = mFixingDate;
	else
		fixingdate = AQLMathDateCalculations::getDate(actiondate, mFixingTerm, *mpSlidingRule, mpCalendar, false);

	DoubleArray x(1);

	for(unsigned int i = 0; i < mTargetLegNo.size(); i++)
	{	
		for (it = extrapayoff[mTargetLegNo[i]].begin(); it != extrapayoff[mTargetLegNo[i]].end(); it++)
		{
			if (it->getPayOff().mPaymentDate < actiondate) continue;
			if (it->getPayOff().mNotionalCF != 0)
			{
				x[0] = mpFX->getRate(it->getPayOff().mNotionalCur, mCurrency, fixingdate); 
				it->getPayOff().mNotionalCFExchangeRate = (*mpStructureFunc)(x);
				it->getPayOff().mNotionalCFCur = mCurrency;				
				it->getPayOff().calcPayOff();				
			}	
		}		
		for (it = iter[mTargetLegNo[i]]; it != payoff[mTargetLegNo[i]].end(); it++)
		{
			if (it->getPayOff().mPaymentDate < actiondate) continue;
			if (it->getPayOff().mNotionalCF != 0)
			{
				x[0] = mpFX->getRate(it->getPayOff().mNotionalCur, mCurrency, fixingdate); 
				
				AQLPricePayOffToolHolder ph(new AQLPricePayOffTool(it->getPayOff()));
				*it = ph;
				it->getPayOff().mNotionalCFCur = mCurrency;
				it->getPayOff().mNotionalCFExchangeRate = (*mpStructureFunc)(x);
				it->getPayOff().calcPayOff();
			}
		}
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
AQLPriceEventNotExCurChange::setUp(const AQLDate& basedate,	
									const AQLObject& trade,
									AQLObject& triggerinfo,
									const AQLPricePayOff& payoff,
									bool isCall)
{
	AQLPriceEventBase::setUp(basedate, trade, triggerinfo, payoff, isCall);
	
	const AQLDataHolder* dh;
	// notional exchange currency
	dh = &(triggerinfo.getData(PRICING_CALIBRATION_DATAOTIONALEXCHANGECURRENCY, ISNOTNULL));
	mCurrency = dynamic_cast<const AQLDataString&>(dh->get()).get();

	// fx
	dh = &(triggerinfo.getData(PRICING_CALIBRATION_DATAOTIONALEXCHANGEFXRATE, ISNOTNULL));
	const AQLDataReference& ref = dynamic_cast<const AQLDataReference&>(dh->get());
	mpFX = &dynamic_cast<const AQLMathFXEntity&>(ref.get().get());	

	// sliding rule and calendar
	dh = &(triggerinfo.getData(PRICING_DATA_FXRATEFIXINGSLIDINGRULE, ISNOTNULL));
	mpSlidingRule = &dynamic_cast<const AQLPriceDataSlidingRule&>(dh->get());	
	if (mpSlidingRule->getSlidingRule() != SLIDING_RULE_NO_CHANGE)
	{
		dh = &(triggerinfo.getData(PRICING_DATA_FXRATEFIXINGCALENDAR, ISNOTNULL));
		mpCalendar = &dynamic_cast<const AQLPriceDataCalendar&>(dh->get());		
	}
	else
		mpCalendar = NULL;

	// fx rate fixing date
	dh = &(triggerinfo.getData(PRICING_DATA_FXRATEFIXINGDATE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		const AQLDate& date = dynamic_cast<const AQLDataDate&>(dh->get()).get();
		if (mpSlidingRule->getSlidingRule() != SLIDING_RULE_NO_CHANGE)
		mFixingDate = mpSlidingRule->getDate(date, *mpCalendar);
		mIsFixingDate = true;
	}
	else
	{
		dh = &(triggerinfo.getData(PRICING_DATA_FXRATEFIXINGTERM, ISNOTNULL));
		mFixingTerm = dynamic_cast<const AQLDataString&>(dh->get()).get();
		mIsFixingDate = false;	
	}

	// fx rate structure function
	AQLDataHolder* _ah = &(triggerinfo.getData(PRICING_DATA_FXRATESTRUCTUREFUNC, ISNOTNULL));
	mpStructureFunc = &dynamic_cast<AQLPriceDataFunction&>(_ah->get()).getFunction();
	

}


