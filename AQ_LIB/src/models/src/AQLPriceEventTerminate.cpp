/*! @file
    @brief source code of trigger action class(trade terminate action).
*/
///
//beta vesion
//
///
#ifdef __GNUG__
#pragma implementation
#endif
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include "AQLPriceEventTerminate.h"
#include "AQLDataBasics.h"
#include "AQLDataMultiReference.h"
#include "AQLObjectHolder.h"
#include "AQLPriceDataDayCount.h"
#include "AQLMathDefine.h"
#include "AQLPriceEventNotExCurChange.h"
#include <algorithm>

using namespace std;



/*!
    @brief Constructor
*/
AQLPriceEventTerminate::AQLPriceEventTerminate()
: AQLPriceEventBase()
{

}
/*!
    @brief Destructor

*/
AQLPriceEventTerminate::~AQLPriceEventTerminate()
{

}

/*
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*
AQLPriceEventTerminate::clone() const    
{
    try 
	{
        return new AQLPriceEventTerminate(*this);
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
AQLPriceEventTerminate::isTypeOf(function_t id) const
{
	return (id == FN_IR_TRIGGERACTIONTERMINATE ? true : AQLPriceEventBase::isTypeOf(id));
}

/*!
    @brief Return this class type
    @return function type
*/
function_t			
AQLPriceEventTerminate::getType() const
{
	return FN_IR_TRIGGERACTIONTERMINATE;
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
AQLPriceEventTerminate::doAction(const AQLDate& actiondate,
									 double actiontime,
									 vector<PayOffToolHolderVector>& payoff,
									 vector<PayOffToolHolderVector>& extrapayoff,
									 vector<AQLPriceEventHolder*>& futureaction,
									 vector<AQLPriceEventHolder*>& pastaction,
									 vector<PayOffToolHolderIter>& iter)
{
	(void)pastaction; //20070411--Nagase--gcc
	for (unsigned int i = 0; i < payoff.size(); i++)
	{
	/*	for (; iter[i] != payoff[i].end() && iter[i]->getPayOff().getPaymentDate() <= actiondate; iter[i]++)
			iter[i]->getPayOff().calcPayOff();
		if (mIsAccrual && mLegStart[i] != actiondate)
		{
			const AQLPricePayOffTool* ptool = NULL; 
			double accrued = 0.0;

			if (mIsArrear[i])
			{
				PayOffToolHolderIter it = iter.at(i);
				for (; it != payoff[i].end(); it++)
				{
					ptool = &it->getPayOff();
					if (ptool->isCouponPayment()) break;
					ptool = NULL;
				}

				
				if (ptool != NULL && ptool->mStart <= actiondate && actiondate < ptool->mEnd)
					accrued = ptool->calcAccruedIntbyPayOffCur(actiondate);

				
			}
			else
			{
				PayOffToolHolderIter it = iter.at(i);
				if (it != payoff[i].begin())
				{
					do
					{
						it--;
						ptool = &it->getPayOff();
						if (ptool->isCouponPayment()) break;
						ptool = NULL;						
					}
					while (it != payoff[i].begin());

				
					if (ptool != NULL && ptool->mStart <= actiondate && actiondate < ptool->mEnd)
					{
						accrued = ptool->calcAccruedIntbyPayOffCur(actiondate);						
					}
				}
			}
	
			if (accrued != 0)
			{
				AQLPricePayOffToolHolder ph(new AQLPricePayOffTool());
				AQLPricePayOffTool& payofftool = ph.getPayOff();
				payofftool.mBaseCur = ptool->mBaseCur;
				payofftool.mAccruedInt = accrued;
				payofftool.mCouponsCur = ptool->mCouponsCur;
				payofftool.mSelectedCpnPos = ptool->mSelectedCpnPos;
				payofftool.mpFX_for_Coupons = ptool->mpFX_for_Coupons;		
				payofftool.mPaymentDate = actiondate;
				payofftool.mPaymentTime = actiontime;
				payofftool.calcPayOff();
				extrapayoff[i].push_back(ph);
			}
		
		}

		if (mIsNotionalExchangeAtEnd[i])
		{
			if (mIsArrear[i] && iter[i] != payoff[i].end())
			{
				AQLPricePayOffToolHolder ph(new AQLPricePayOffTool());
				AQLPricePayOffTool& payofftool = ph.getPayOff();
				payofftool.mBaseCur = iter[i]->getPayOff().mBaseCur;
				payofftool.mNotionalCur = iter[i]->getPayOff().mNotionalCur;
				payofftool.mpFX_for_Notional = iter[i]->getPayOff().mpFX_for_Notional;
				payofftool.mPaymentDate = actiondate;
				payofftool.mPaymentTime = actiontime;
				payofftool.mNotionalCF = iter[i]->getPayOff().getNotional();
				payofftool.calcPayOff();
				extrapayoff[i].push_back(ph);
			}
			else if (!mIsArrear[i] && iter[i] != payoff[i].begin())
			{
				PayOffToolHolderIter it = iter.at(i - 1);
				AQLPricePayOffToolHolder ph(new AQLPricePayOffTool());
				AQLPricePayOffTool& payofftool = ph.getPayOff();
				payofftool.mBaseCur = it->getPayOff().mBaseCur;
				payofftool.mNotionalCur = it->getPayOff().mNotionalCur;
				payofftool.mpFX_for_Notional = it->getPayOff().mpFX_for_Notional;
				payofftool.mPaymentDate = actiondate;
				payofftool.mPaymentTime = actiontime;
				payofftool.mNotionalCF = it->getPayOff().getNotional();
				payofftool.calcPayOff();
				extrapayoff[i].push_back(ph);				
			}
		}*/

		AQLPricePayOffToolHolder ph;
		double accrued = calcAccruedInterest(actiondate, actiontime, payoff, iter, i, ph); 
		if (accrued != 0.0)
			extrapayoff[i].push_back(ph);

		double notionalcf = calcNotionalExchange(actiondate,  actiontime, payoff, extrapayoff, pastaction, iter, i, ph); 
		if (notionalcf != 0.0)
			extrapayoff[i].push_back(ph);

		if (iter[i] != payoff[i].end()) iter[i] = payoff[i].erase(iter[i], payoff[i].end());
	}
	
	for (unsigned int i = 0; i < extrapayoff.size(); i++)
	{
		PayOffToolHolderIter it = extrapayoff[i].begin();
		while (it != extrapayoff[i].end())
		{
			if (it->getPayOff().getPaymentDate() > actiondate)
				it = extrapayoff[i].erase(it);
			else
				it++;
		}
	}

//	futureaction.clear();
	AQLDate backdate = actiondate;//action
	vector<AQLPriceEventHolder*>::iterator pItr =  futureaction.begin();
	while (pItr != futureaction.end())
	{
		const DateVector &actionDates = (*pItr)->getActionDates();
		DateVector::const_iterator dItr = actionDates.begin();

		bool delFlg = true;
		while(dItr != actionDates.end())
		{
			if (actiondate >= *dItr)
			{
				DateVector::const_iterator mindate =
				min_element(actionDates.begin(), actionDates.end());
				if (backdate > *mindate)
				{
					backdate = *mindate;
				}
				delFlg = false;
				break;
			}
			++dItr;
		}

		if (delFlg)
		{
			pItr = futureaction.erase(pItr++);
		}
		else
		{
			++pItr;
		}
	}
	if (futureaction.size() > 0)
	{
		const int  paySize = payoff.size();
		for (int i = 0; i < paySize; ++i)
		{
			iter[i] = payoff[i].begin();
			while (iter[i] != payoff[i].end() && iter[i]->getPayOff().getPaymentDate() < backdate)
			{
				++iter[i];
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
AQLPriceEventTerminate::setUp(const AQLDate& basedate,	
								const AQLObject& trade,
								AQLObject& triggerinfo,
								const AQLPricePayOff& payoff,
								bool isCall)
{

	AQLPriceEventBase::setUp(basedate, trade, triggerinfo, payoff, isCall);
	
	const AQLDataHolder* dh;
	//leg object
	dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const AQLDataMultiReference& legs = dynamic_cast<const AQLDataMultiReference&>(dh->get());
		
	mTargetLegNo.resize(legs.getSize());
	for (unsigned int i = 0; i < mTargetLegNo.size(); i++)
		mTargetLegNo[i] = i;
		
	
	mIsAccrual = false;
	dh= &triggerinfo.getData(PRICING_DATA_ISACCRUAL, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
		mIsAccrual = dynamic_cast<const AQLDataBool&>(dh->get()).get();


	mIsNotionalExchangeAtEnd.clear();
	mLegStart.clear();
	mIsArrear.clear();

	for (unsigned int i = 0; i < legs.getSize(); i++)
	{
		// isNotionalExchangeAtEnd
		dh = &(legs.get(i).getData(PRICING_DATA_ISNOTIONALEXCHANGEATEND, ISNOTNULL));
		mIsNotionalExchangeAtEnd.push_back(dynamic_cast<const AQLDataBool&>(dh->get()).get());		
		//payment timing
		dh = &(legs.get(i).getData(PRICING_DATA_PAYMENTTIMING, ISNOTNULL));
		const AQLString& timing = dynamic_cast<const AQLDataString&>(dh->get()).get(); 
		mIsArrear.push_back(AQLPriceCFGenUtility::isArrear(timing));	
	}

    if (!mIsAccrual) return;


	for (unsigned int i = 0; i < legs.getSize(); i++)
	{
		//start date of trade
		dh = &(legs.get(i).getData(PRICING_DATA_STARTDATE, ISNOTNULL));
		const AQLDate& startdate = dynamic_cast<const AQLDataDate&>(dh->get()).get();
		AQLDate startdate_sliding = AQLPriceCFGenUtility::getDate(startdate, legs.get(i).get(), 
							CALIBRATION_DATA_SLIDINGRULE, CALIBRATION_DATA_CALENDAR);
		mLegStart.push_back(startdate_sliding);
	}

}


/*!
	@brief calculate accrued interest at actiondate
	@param[in] actiondate action date
	@param[in] actiontime action time
	@param[in,out] payoff payoff
	@param[in,out] iter position of nearest payoff from this action expiry date
	@param[in] legno legNo (first leg No. is 0)
	@param[out] ph output

	@return accrued interest
*/	
double
AQLPriceEventTerminate::calcAccruedInterest(const AQLDate& actiondate,
										 double actiontime,
										 std::vector<PayOffToolHolderVector>& payoff,
										 std::vector<PayOffToolHolderIter>& iter,
										 unsigned int legno,
										 AQLPricePayOffToolHolder& ph) const
{
	for (; iter[legno] != payoff[legno].end() && iter[legno]->getPayOff().getPaymentDate() <= actiondate; iter[legno]++)
		iter[legno]->getPayOff().calcPayOff();
	
	double accrued = 0.0;
	if (mIsAccrual && mLegStart[legno] < actiondate)
	{
		const AQLPricePayOffTool* ptool = NULL; 
		
		if (mIsArrear[legno])
		{
			PayOffToolHolderIter it = iter.at(legno);
			for (; it != payoff[legno].end(); it++)
			{
				ptool = &it->getPayOff();
				if (ptool->isCouponPayment()) break;
				ptool = NULL;
			}
		}
		else
		{
			throw AQLCoreInvalidData("Not support case(Advance payment and IsAccrual = true)", __FILE__, __LINE__);			
			/*PayOffToolHolderIter it = iter.at(legno);
			if (it != payoff[legno].begin())
			{
				do
				{
					it--;
					ptool = &it->getPayOff();
					if (ptool->isCouponPayment()) break;
					ptool = NULL;						
				}
				while (it != payoff[legno].begin());			
			}*/
		}
		if (ptool != NULL)
			accrued = ptool->calcAccruedIntbyPayOffCur(actiondate);
/*		else if (ptool == NULL && mCashlets[legno] != NULL)
		{
			;
		}*/

		if (accrued != 0.0)
		{
			ph.setPayOff(new AQLPricePayOffTool());
			AQLPricePayOffTool& payofftool = ph.getPayOff();
			payofftool.mBaseCur = ptool->mBaseCur;
			payofftool.mAccruedInt = accrued;
			payofftool.mCouponsCur = ptool->mCouponsCur;
			payofftool.mSelectedCpnPos = ptool->mSelectedCpnPos;
			payofftool.mpFX_for_PayOff = ptool->mpFX_for_PayOff;	
			payofftool.mPaymentDate = actiondate;
			payofftool.mPaymentTime = actiontime;
			payofftool.calcPayOff();
		}

	}
	return accrued;
}

/*!
	@brief calculate notional exchange at actiondate
	@param[in] actiondate action date
	@param[in] actiontime action time
	@param[in,out] payoff payoff
	@param[in,out] iter position of nearest payoff from this action expiry date
	@param[in] legno legNo (first leg No. is 0)
	@param[out] ph output

	@return notional exchange amount
*/	
double
AQLPriceEventTerminate::calcNotionalExchange(const AQLDate& actiondate,
										 double actiontime,
										 vector<PayOffToolHolderVector>& payoff,
										 vector<PayOffToolHolderVector>& extrapayoff,
										 vector<AQLPriceEventHolder*>& pastaction,
										 vector<PayOffToolHolderIter>& iter,
										 unsigned int legno,
										 AQLPricePayOffToolHolder& ph) const
{
	for (; iter[legno] != payoff[legno].end() && iter[legno]->getPayOff().getPaymentDate() <= actiondate; iter[legno]++)
		iter[legno]->getPayOff().calcPayOff();
	
	double notionalex = 0.0;
	if (mIsNotionalExchangeAtEnd[legno])
	{
		AQLPriceEventBase* _action = NULL;
		AQLDate _date;
		double _time = 0.0;
		//Notinonal CF currency exchange trigger is hitted in past
		for (int i = pastaction.size() - 1; i >= 0; i--)
		{
			const vector<AQLPriceEventBase*>& action = pastaction[i]->getAction();
			for (unsigned int j = 0; j < action.size(); j++)
			{
				if (action[j]->isTypeOf(FN_IR_TRIGGERACTIONNOTEXCURCHANGE)
					&& pastaction[i]->getActionDates()[j] <= actiondate)
				{
					_action = action[j];
					_date = pastaction[i]->getActionDates()[j];
					_time = pastaction[i]->getActionTimes()[j];
					break;
				}
			}
			if (_action != NULL) break;
		}

		bool flag = false;		
		if (iter[legno] != payoff[legno].end())
		{
			ph.setPayOff(new AQLPricePayOffTool());
			AQLPricePayOffTool& payofftool = ph.getPayOff();
			payofftool.mBaseCur = iter[legno]->getPayOff().mBaseCur;
			payofftool.mNotionalCur = iter[legno]->getPayOff().mNotionalCur;
			payofftool.mpFX_for_PayOff = iter[legno]->getPayOff().mpFX_for_PayOff;
			payofftool.mPaymentDate = actiondate;
			payofftool.mPaymentTime = actiontime;
			payofftool.mNotionalCF = iter[legno]->getPayOff().getNotional();
			if (!mIsArrear[legno]) 
				payofftool.mNotionalCF += iter[legno]->getPayOff().mNotionalCF;			
			payofftool.mNotionalCFCur = payofftool.mNotionalCur;
			payofftool.mNotionalCFExchangeRate = 1;
			payofftool.calcPayOff();
			notionalex = payofftool.mNotionalCF;
			flag = true;
		}


		//ChangeNotionalCF
		PayOffToolHolderIter it;
		for (it = extrapayoff[legno].begin(); it != extrapayoff[legno].end(); it++)
		{
			if (it->getPayOff().mPaymentDate <= actiondate) continue;
			if (flag &&	
				it->getPayOff().mPaymentDate > ph.getPayOff().mPaymentDate) continue;
			if (flag &&	mIsArrear[legno] &&
				it->getPayOff().mPaymentDate == ph.getPayOff().mPaymentDate) continue;

			if (it->getPayOff().mNotionalCF != 0.0)
			{
				if (!flag)
				{
					ph.setPayOff(new AQLPricePayOffTool());
					AQLPricePayOffTool& payofftool = ph.getPayOff();
					payofftool.mBaseCur = it->getPayOff().mBaseCur;
					payofftool.mNotionalCur = it->getPayOff().mNotionalCur;
					payofftool.mpFX_for_PayOff = it->getPayOff().mpFX_for_PayOff;
					payofftool.mPaymentDate = actiondate;
					payofftool.mPaymentTime = actiontime;
					payofftool.mNotionalCF = it->getPayOff().mNotionalCF;
					payofftool.mNotionalCFCur = payofftool.mNotionalCur;
					payofftool.mNotionalCFExchangeRate = 1;
					payofftool.calcPayOff();
					flag = true;
				}
				else
					ph.getPayOff().mNotionalCF += it->getPayOff().mNotionalCF;
			
				notionalex += ph.getPayOff().mNotionalCF;
			}	
		}
		//
		if (flag && _action != NULL)
		{
			vector<PayOffToolHolderVector> dummy(payoff.size());
			vector<AQLPriceEventHolder*> dummy2;
			vector<PayOffToolHolderVector> tmp(payoff.size());
			vector<PayOffToolHolderIter> tmp_iter(payoff.size());
			tmp[legno].push_back(ph);
			for (unsigned int i = 0; i < tmp_iter.size(); i++)
				tmp_iter[i] = tmp[i].begin();
			_action->doAction(_date, _time, tmp, dummy, dummy2, dummy2, tmp_iter);
			ph = tmp[legno].back();
		}


/*		if (mIsArrear[legno] && iter[legno] != payoff[legno].end())
		{
			ph.setPayOff(new AQLPricePayOffTool());
			AQLPricePayOffTool& payofftool = ph.getPayOff();
			payofftool.mBaseCur = iter[legno]->getPayOff().mBaseCur;
			payofftool.mNotionalCur = iter[legno]->getPayOff().mNotionalCur;
			payofftool.mpFX_for_PayOff = iter[legno]->getPayOff().mpFX_for_PayOff;
			payofftool.mPaymentDate = actiondate;
			payofftool.mPaymentTime = actiontime;
			payofftool.mNotionalCF = iter[legno]->getPayOff().getNotional();
			payofftool.mNotionalCFCur = iter[legno]->getPayOff().mNotionalCFCur;
			payofftool.mNotionalCFExchangeRate = iter[legno]->getPayOff().mNotionalCFExchangeRate;
			payofftool.calcPayOff();
			notionalex = payofftool.mNotionalCF;
		}
		else if (!mIsArrear[legno] && iter[legno] != payoff[legno].begin())
		{
			PayOffToolHolderIter it = iter.at(legno);
			it--;
			ph.setPayOff(new AQLPricePayOffTool());
			AQLPricePayOffTool& payofftool = ph.getPayOff();
			payofftool.mBaseCur = it->getPayOff().mBaseCur;
			payofftool.mNotionalCur = it->getPayOff().mNotionalCur;
			payofftool.mpFX_for_PayOff = it->getPayOff().mpFX_for_PayOff;
			payofftool.mPaymentDate = actiondate;
			payofftool.mPaymentTime = actiontime;
			payofftool.mNotionalCF = it->getPayOff().getNotional();
			payofftool.calcPayOff();
			notionalex = payofftool.mNotionalCF;
		}*/
	}	
	return notionalex;
}



