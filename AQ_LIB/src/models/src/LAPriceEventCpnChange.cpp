/*! @file
    @brief source code of trigger action class(coupon change action).

*/
//  2006, AlgoQuantHub..
///
//beta vesion
//いくかのパターンでテスト実施ずみ
//
#ifdef __GNUG__
#pragma implementation
#endif
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include "LAPriceEventCpnChange.h"

#include "LADataBasics.h"
#include "LADataReference.h"
#include "LADataMultiReference.h"
#include "LAObjectHolder.h"
#include "LAPriceCouponTool.h"
#include "LAMathFXEntity.h"
#include "LAMathDefine.h"
using namespace std;



/*!
    @brief Constructor
*/
LAPriceEventCpnChange::LAPriceEventCpnChange()
: LAPriceEventBase()
{

}
/*!
    @brief Destructor

*/
LAPriceEventCpnChange::~LAPriceEventCpnChange()
{

}

/*
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*
LAPriceEventCpnChange::clone() const
{
    try 
	{
        return new LAPriceEventCpnChange(*this);
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
LAPriceEventCpnChange::isTypeOf(function_t id) const
{
	return (id == FN_IR_TRIGGERACTIONCPNCHANGE ? true : LAPriceEventBase::isTypeOf(id));
}

/*!
    @brief Return this class type
    @return function type
*/
function_t			
LAPriceEventCpnChange::getType() const
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
LAPriceEventCpnChange::doAction(const LADate& actiondate,
									 double actiontime,
									 vector<PayOffToolHolderVector>& payoff,
									 vector<PayOffToolHolderVector>& extrapayoff,
									 vector<LAPriceEventHolder*>& futureaction,
									 vector<LAPriceEventHolder*>& pastaction,
									 vector<PayOffToolHolderIter>& iter)
{
	(void)pastaction; (void)futureaction; (void)extrapayoff; (void)actiontime; //20070411--Nagase--警告削除をgccにも対応
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
			LAString notionalCFCur = it->getPayOff().mNotionalCFCur;
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
LAPriceEventCpnChange::setUp(const LADate& basedate,	
									const LAObject& trade,
									LAObject& triggerinfo,
									const LAPricePayOff& payoff,
									bool isCall)
{
	LAPriceEventBase::setUp(basedate, trade, triggerinfo, payoff, isCall);
	
	const LADataHolder* dh;

	for (unsigned int i = 0; i < mPayOff.size(); i++)
		mPayOff[i].clear();
	mPayOff.clear();
	mPayOff.resize(mTargetLegNo.size());

	mIsArrearPayment.clear();

	//leg
	dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const LADataMultiReference& legs = dynamic_cast<const LADataMultiReference&>(dh->get());

	for (unsigned int i = 0; i < mTargetLegNo.size(); i++)
	{
		mPayOff[i].resize(payoff.getPayOff()[mTargetLegNo[i]].size());
		//couponinfos	
		dh = &(triggerinfo.getData(PRICING_DATA_COUPONINFOS + LADataInt(mTargetLegNo[i] + 1).convertToString(), ISNOTNULL));
		const LADataMultiReference& couponinfos = dynamic_cast<const LADataMultiReference&>(dh->get());
		int k = mPayOff[i].size() - 1;
		for (unsigned int j = couponinfos.getSize(); j > 0 && k >= 0; k--)
		{
			const LAPricePayOffTool& tool = payoff.getPayOff()[mTargetLegNo[i]][k].getPayOff();
			if (!tool.isCouponPayment())
				mPayOff[i][k] = payoff.getPayOff()[mTargetLegNo[i]][k];
			else
			{
				mPayOff[i][k].setPayOff(new LAPricePayOffTool());
				LAPricePayOffTool& tool2 = mPayOff[i][k].getPayOff();
				tool2.copy(tool);
				tool2.mpCoupons.push_back(new LAPriceCouponTool());
				tool2.mpCoupons[0]->setUp(basedate,	
										trade,
										mTargetLegNo[i],
										couponinfos.get(j - 1).get(),
										payoff,
										k);
				//coupon currecny
				dh = &(couponinfos.get(j - 1).getData(PRICING_DATA_CURRENCY, ISNOTNULL));
				tool2.mCouponsCur.push_back(dynamic_cast<const LADataString&>(dh->get()).get());
				if (tool2.mNotionalCur != tool2.mCouponsCur[0] && tool2.mpFX_for_Notional == NULL)
				{
					//error
					LAString msg = "FxRate is need for notional to change into coupon currency ";
					throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);					
				}

				//fx rate for coupon
/*				dh = &(couponinfos.get(j - 1).getData(PRICING_DATA_FXRATE, NOCHECK));
				if (dh->isDefined() && !dh->isNull())
				{
					const LADataReference& ref = dynamic_cast<const LADataReference&>(dh->get());
					tool2.mpFX_for_Coupons.push_back(&dynamic_cast<const LAMathFXEntity&>(ref.get().get()));	
				}*/
				j--;
			}
				
		}

		// isArrear
		dh = &(legs.get(mTargetLegNo[i]).getData(PRICING_DATA_PAYMENTTIMING, ISNOTNULL));
		const LAString& timing = dynamic_cast<const LADataString&>(dh->get()).get(); 
		mIsArrearPayment.push_back(LAPriceCFGenUtility::isArrear(timing));

	}

	


}



