/*! @file
    @brief source code of trigger action class(notional exchange currency change action).

*/
//  2006, AlgoQuantHub..
///
//////////////////////////////////////////////////////////
//いくかのパターンでテスト実施ずみ 
//////////////////////////////////////////////////////////
#ifdef __GNUG__
#pragma implementation
#endif
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include "LAPriceEventNotExCurChange.h"

#include "LADataHolder.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataReference.h"
#include "LAObjectHolder.h"
#include "LAMathDefine.h"
#include "LAPriceDataCalendar.h"
#include "LAPriceDataSlidingRule.h"
#include "LAPriceDataFunction.h"
#include "LAMathFXEntity.h"
#include "LAMathDateCalculations.h"

using namespace std;

/*!
    @brief Constructor
*/
LAPriceEventNotExCurChange::LAPriceEventNotExCurChange()
: LAPriceEventBase()
, mpFX(0), mpCalendar(0), mpSlidingRule(0) 
{

}
/*!
    @brief Destructor

*/
LAPriceEventNotExCurChange::~LAPriceEventNotExCurChange()
{

}

/*
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*
LAPriceEventNotExCurChange::clone() const    
{
    try 
	{
        return new LAPriceEventNotExCurChange(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }	
}

/*
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LAPriceEventNotExCurChange::isTypeOf(function_t id) const
{
	return (id == FN_IR_TRIGGERACTIONNOTEXCURCHANGE ? true : LAPriceEventBase::isTypeOf(id));
}

/*!
    @brief Return this class type
    @return function type
*/
function_t			
LAPriceEventNotExCurChange::getType() const
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
LAPriceEventNotExCurChange::doAction(const LADate& actiondate,
										double actiontime,
										vector<PayOffToolHolderVector>& payoff,
										vector<PayOffToolHolderVector>& extrapayoff,
										vector<LAPriceEventHolder*>& futureaction,
										vector<LAPriceEventHolder*>& pastaction,
										vector<PayOffToolHolderIter>& iter)
{
	(void)actiontime; (void)pastaction; (void)futureaction; //20070411--Nagase--警告削除をgccにも対応
	PayOffToolHolderIter it;
	LADate fixingdate;
	if (mIsFixingDate)
		fixingdate = mFixingDate;
	else
		fixingdate = LAMathDateCalculations::getDate(actiondate, mFixingTerm, *mpSlidingRule, mpCalendar, false);

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
				
				LAPricePayOffToolHolder ph(new LAPricePayOffTool(it->getPayOff()));
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
LAPriceEventNotExCurChange::setUp(const LADate& basedate,	
									const LAObject& trade,
									LAObject& triggerinfo,
									const LAPricePayOff& payoff,
									bool isCall)
{
	LAPriceEventBase::setUp(basedate, trade, triggerinfo, payoff, isCall);
	
	const LADataHolder* dh;
	// notional exchange currency
	dh = &(triggerinfo.getData(PRICING_CALIBRATION_DATAOTIONALEXCHANGECURRENCY, ISNOTNULL));
	mCurrency = dynamic_cast<const LADataString&>(dh->get()).get();

	// fx
	dh = &(triggerinfo.getData(PRICING_CALIBRATION_DATAOTIONALEXCHANGEFXRATE, ISNOTNULL));
	const LADataReference& ref = dynamic_cast<const LADataReference&>(dh->get());
	mpFX = &dynamic_cast<const LAMathFXEntity&>(ref.get().get());	

	// sliding rule and calendar
	dh = &(triggerinfo.getData(PRICING_DATA_FXRATEFIXINGSLIDINGRULE, ISNOTNULL));
	mpSlidingRule = &dynamic_cast<const LAPriceDataSlidingRule&>(dh->get());	
	if (mpSlidingRule->getSlidingRule() != SLIDING_RULE_NO_CHANGE)
	{
		dh = &(triggerinfo.getData(PRICING_DATA_FXRATEFIXINGCALENDAR, ISNOTNULL));
		mpCalendar = &dynamic_cast<const LAPriceDataCalendar&>(dh->get());		
	}
	else
		mpCalendar = NULL;

	// fx rate fixing date
	dh = &(triggerinfo.getData(PRICING_DATA_FXRATEFIXINGDATE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		const LADate& date = dynamic_cast<const LADataDate&>(dh->get()).get();
		if (mpSlidingRule->getSlidingRule() != SLIDING_RULE_NO_CHANGE)
		mFixingDate = mpSlidingRule->getDate(date, *mpCalendar);
		mIsFixingDate = true;
	}
	else
	{
		dh = &(triggerinfo.getData(PRICING_DATA_FXRATEFIXINGTERM, ISNOTNULL));
		mFixingTerm = dynamic_cast<const LADataString&>(dh->get()).get();
		mIsFixingDate = false;	
	}

	// fx rate structure function
	LADataHolder* _ah = &(triggerinfo.getData(PRICING_DATA_FXRATESTRUCTUREFUNC, ISNOTNULL));
	mpStructureFunc = &dynamic_cast<LAPriceDataFunction&>(_ah->get()).getFunction();
	

}


