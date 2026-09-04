/*! @file
    @brief source code of trigger action class(coupon change action).
*/
///
//beta vesion
//
//
#ifdef __GNUG__
#pragma implementation
#endif
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include "AQLPriceEventCpnChange.h"

#include "AQLDataBasics.h"
#include "AQLDataReference.h"
#include "AQLDataMultiReference.h"
#include "AQLObjectHolder.h"
#include "AQLPriceCouponTool.h"
#include "AQLMathFXEntity.h"
#include "AQLMathDefine.h"
using namespace std;



/*!
    @brief Constructor
*/
AQLPriceEventCpnChange::AQLPriceEventCpnChange()
: AQLPriceEventBase()
{

}
/*!
    @brief Destructor

*/
AQLPriceEventCpnChange::~AQLPriceEventCpnChange()
{

}

/*
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*
AQLPriceEventCpnChange::clone() const
{
    try 
	{
        return new AQLPriceEventCpnChange(*this);
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
AQLPriceEventCpnChange::isTypeOf(function_t id) const
{
	return (id == FN_IR_TRIGGERACTIONCPNCHANGE ? true : AQLPriceEventBase::isTypeOf(id));
}

/*!
    @brief Return this class type
    @return function type
*/
function_t			
AQLPriceEventCpnChange::getType() const
{
	return FN_IR_TRIGGERACTIONCPNCHANGE;
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
AQLPriceEventCpnChange::doAction(const AQLDate& actiondate,
									 double actiontime,
									 vector<PayOffToolHolderVector>& payoff,
									 vector<PayOffToolHolderVector>& extrapayoff,
									 vector<AQLPriceEventHolder*>& futureaction,
									 vector<AQLPriceEventHolder*>& pastaction,
									 vector<PayOffToolHolderIter>& iter)
{
	(void)pastaction; (void)futureaction; (void)extrapayoff; (void)actiontime; //20070411--Nagase--gcc
	PayOffToolHolderIter it;
	for(unsigned int i = 0; i < mTargetLegNo.size(); i++)
	{
		unsigned int j = static_cast<unsigned int>(iter[mTargetLegNo[i]] - payoff[mTargetLegNo[i]].begin());
		for (it = iter[mTargetLegNo[i]]; it != payoff[mTargetLegNo[i]].end(); it++, j++)
		{
			if (!it->getPayOff().isCouponPayment()) continue;
//			if (it->getPayOff().getPaymentDate() <= actiondate) continue;
			if (it->getPayOff().getPaymentDate() < actiondate)continue;
			if (mIsArrearPayment[i] && it->getPayOff().getPaymentDate() == actiondate) continue;
			
			// there is a case of notinal change trigger hitted before
			// so we change notional 
			double notional = it->getPayOff().getNotional();
			// there is a case of notinalcf currency change trigger hitted before
			// so we change notinal cf currency
			AQLString notionalCFCur = it->getPayOff().mNotionalCFCur;
			double notionalCFExchangeRate  =  it->getPayOff().mNotionalCFExchangeRate;
			
			*it = mPayOff[i][j];			
			it->getPayOff().setNotional(notional);
			it->getPayOff().mNotionalCFCur = notionalCFCur;
			it->getPayOff().mNotionalCFExchangeRate = notionalCFExchangeRate;

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
AQLPriceEventCpnChange::setUp(const AQLDate& basedate,	
									const AQLObject& trade,
									AQLObject& triggerinfo,
									const AQLPricePayOff& payoff,
									bool isCall)
{
	AQLPriceEventBase::setUp(basedate, trade, triggerinfo, payoff, isCall);
	
	const AQLDataHolder* dh;

	for (unsigned int i = 0; i < mPayOff.size(); i++)
		mPayOff[i].clear();
	mPayOff.clear();
	mPayOff.resize(mTargetLegNo.size());

	mIsArrearPayment.clear();

	//leg
	dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const AQLDataMultiReference& legs = dynamic_cast<const AQLDataMultiReference&>(dh->get());

	for (unsigned int i = 0; i < mTargetLegNo.size(); i++)
	{
		mPayOff[i].resize(payoff.getPayOff()[mTargetLegNo[i]].size());
		//couponinfos	
		dh = &(triggerinfo.getData(PRICING_DATA_COUPONINFOS + AQLDataInt(mTargetLegNo[i] + 1).convertToString(), ISNOTNULL));
		const AQLDataMultiReference& couponinfos = dynamic_cast<const AQLDataMultiReference&>(dh->get());
		int k = mPayOff[i].size() - 1;
		for (unsigned int j = couponinfos.getSize(); j > 0 && k >= 0; k--)
		{
			const AQLPricePayOffTool& tool = payoff.getPayOff()[mTargetLegNo[i]][k].getPayOff();
			if (!tool.isCouponPayment())
				mPayOff[i][k] = payoff.getPayOff()[mTargetLegNo[i]][k];
			else
			{
				mPayOff[i][k].setPayOff(new AQLPricePayOffTool());
				AQLPricePayOffTool& tool2 = mPayOff[i][k].getPayOff();
				tool2.copy(tool);
				tool2.mpCoupons.push_back(new AQLPriceCouponTool());
				tool2.mpCoupons[0]->setUp(basedate,	
										trade,
										mTargetLegNo[i],
										couponinfos.get(j - 1).get(),
										payoff,
										k);
				//coupon currecny
				dh = &(couponinfos.get(j - 1).getData(PRICING_DATA_CURRENCY, ISNOTNULL));
				tool2.mCouponsCur.push_back(dynamic_cast<const AQLDataString&>(dh->get()).get());
				if (tool2.mNotionalCur != tool2.mCouponsCur[0] && tool2.mpFX_for_Notional == NULL)
				{
					//error
					AQLString msg = "FxRate is need for notional to change into coupon currency ";
					throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);					
				}

				//fx rate for coupon
/*				dh = &(couponinfos.get(j - 1).getData(PRICING_DATA_FXRATE, NOCHECK));
				if (dh->isDefined() && !dh->isNull())
				{
					const AQLDataReference& ref = dynamic_cast<const AQLDataReference&>(dh->get());
					tool2.mpFX_for_Coupons.push_back(&dynamic_cast<const AQLMathFXEntity&>(ref.get().get()));	
				}*/
				j--;
			}
				
		}

		// isArrear
		dh = &(legs.get(mTargetLegNo[i]).getData(PRICING_DATA_PAYMENTTIMING, ISNOTNULL));
		const AQLString& timing = dynamic_cast<const AQLDataString&>(dh->get()).get(); 
		mIsArrearPayment.push_back(AQLPriceCFGenUtility::isArrear(timing));

	}

	


}



