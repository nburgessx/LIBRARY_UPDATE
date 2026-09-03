/*! @file
    @brief source code of trigger action class(notional change action).

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

#include "AQLPriceEventNotionalChange.h"
#include "AQLPriceEventNotExCurChange.h"

#include "AQLDataHolder.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataMultiReference.h"
#include "AQLObjectHolder.h"
#include "AQLMathDefine.h"

using namespace std;

#define EQUALIZATION	"EQUALIZATION"
#define AMOUNTSETTING	"AMOUNTSETTING"

/*!
    @brief Constructor
*/
AQLPriceEventNotionalChange::AQLPriceEventNotionalChange()
: AQLPriceEventBase()
{

}
/*!
    @brief Destructor

*/
AQLPriceEventNotionalChange::~AQLPriceEventNotionalChange()
{

}

/*
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*
AQLPriceEventNotionalChange::clone() const    
{
    try 
	{
        return new AQLPriceEventNotionalChange(*this);
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
AQLPriceEventNotionalChange::isTypeOf(function_t id) const
{
	return (id == FN_IR_TRIGGERACTIONNOTIONALCHANGE ? true : AQLPriceEventBase::isTypeOf(id));
}

/*!
    @brief Return this class type
    @return function type
*/
function_t			
AQLPriceEventNotionalChange::getType() const
{
	return FN_IR_TRIGGERACTIONNOTIONALCHANGE;
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
AQLPriceEventNotionalChange::doAction(const AQLDate& actiondate,
										double actiontime,
										vector<PayOffToolHolderVector>& payoff,
										vector<PayOffToolHolderVector>& extrapayoff,
										vector<AQLPriceEventHolder*>& futureaction,
										vector<AQLPriceEventHolder*>& pastaction,
										vector<PayOffToolHolderIter>& iter)
{
	(void)pastaction; (void)futureaction; //20070411--Nagase--gcc
	PayOffToolHolderIter it;
	for(unsigned int i = 0; i < mTargetLegNo.size(); i++)
	{	
		for (it = extrapayoff[mTargetLegNo[i]].begin(); it != extrapayoff[mTargetLegNo[i]].end(); it++)
		{
			if (it->getPayOff().mPaymentDate < actiondate) continue;
			if (it->getPayOff().mNotionalCF != 0)
			{
				it->getPayOff().mNotionalCF *= mChangeRatio;				
			}	
		}			
		
		AQLPriceEventBase* _action = NULL;
		AQLDate _date;
		double _time = 0.0;
		if (mIsNotionalExchangeAtEnd[i])
		{
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
		}


		unsigned int j = static_cast<unsigned int>(iter[mTargetLegNo[i]] - payoff[mTargetLegNo[i]].begin());
		// not amortize case
		if (!mIsAmortize[i])
		{
			bool flag = true;
			for (it = iter[mTargetLegNo[i]]; it != payoff[mTargetLegNo[i]].end(); it++, j++)
			{
				if (!it->getPayOff().isCouponPayment() && 
					(!mIsNotionalExchangeAtEnd[i] || mIsArrearPayment[i] || (it != (payoff[mTargetLegNo[i]].end() - 1)))) continue;
				if (it->getPayOff().getPaymentDate() < actiondate) continue;
				if (mIsArrearPayment[i] && it->getPayOff().getPaymentDate() == actiondate) continue;

				double notional = it->getPayOff().mNotional;
				AQLPricePayOffToolHolder ph(new AQLPricePayOffTool(it->getPayOff()));
				*it = ph;
				it->getPayOff().setNotional(notional * mChangeRatio);
			
				if (mIsNotionalExchangeAtEnd[i] && flag)
				{
					//notional exchange at action dates
					AQLPricePayOffToolHolder ph(new AQLPricePayOffTool());
					AQLPricePayOffTool& payofftool = ph.getPayOff();
					payofftool.mBaseCur = it->getPayOff().mBaseCur;
					payofftool.mNotionalCur = it->getPayOff().mNotionalCur;
					payofftool.mNotionalCFCur = payofftool.mNotionalCur;
					payofftool.mNotionalCFExchangeRate = 1.0;
					payofftool.mpFX_for_Notional = it->getPayOff().mpFX_for_Notional;
					payofftool.mPaymentDate = actiondate;
					payofftool.mPaymentTime = actiontime;
					payofftool.mNotionalCF = notional * (1.0 - mChangeRatio);
					payofftool.mpFX_for_PayOff = it->getPayOff().mpFX_for_PayOff;
					payofftool.calcPayOff();
					//
					if (_action != NULL)
					{
						vector<PayOffToolHolderVector> dummy(payoff.size());
						vector<AQLPriceEventHolder*> dummy2;
						vector<PayOffToolHolderVector> tmp(payoff.size());
						vector<PayOffToolHolderIter> tmp_iter(payoff.size());
						tmp[mTargetLegNo[i]].push_back(ph);
						for (unsigned int j = 0; j < tmp_iter.size(); j++)
							tmp_iter[j] = tmp[j].begin();
						_action->doAction(_date, _time, tmp, dummy, dummy2, dummy2, tmp_iter);
						ph = tmp[mTargetLegNo[i]].back();
					}				
					extrapayoff[mTargetLegNo[i]].push_back(ph);
					flag = false;
				}
				//correct notional exchange at last payoff
				if (mIsNotionalExchangeAtEnd[i] && (it == (payoff[mTargetLegNo[i]].end() - 1)))
					it->getPayOff().mNotionalCF -= notional * (1.0 - mChangeRatio);				
			}

		}
		else
		{
		//amortize case
			const DoubleArray& notionalarray = 
				getNotionalArray(actiondate, payoff[mTargetLegNo[i]], iter[mTargetLegNo[i]], mIsArrearPayment[i],
				mChangeRatio, mAmortizeRoundFunction[i], 
				mAmortizeRoundDigit[i], mAmortize1stFraction[i], 
				mIsAmortizeAmountSetting[i], mAmortizeAmount[i]);
			
			unsigned int k = 0;
			double notional1st = 0;
			for (it = iter[mTargetLegNo[i]]; it != payoff[mTargetLegNo[i]].end(); it++, j++)
			{
				if (k == notionalarray.size()) break;
				if (!it->getPayOff().isCouponPayment()) continue;
				if (it->getPayOff().getPaymentDate() < actiondate)continue;
				if (mIsArrearPayment[i] && it->getPayOff().getPaymentDate() == actiondate) continue;

				if (k == 0) notional1st = it->getPayOff().mNotional;
				AQLPricePayOffToolHolder ph(new AQLPricePayOffTool(it->getPayOff()));
				*it = ph;						
				it->getPayOff().setNotional(notionalarray[k]);
				//correct notional exchange
				if (mIsNotionalExchangeAtEnd[i])
				{
					if (k == 0)
					{
						//notional exchange at action dates
						AQLPricePayOffToolHolder ph(new AQLPricePayOffTool());
						AQLPricePayOffTool& payofftool = ph.getPayOff();
						payofftool.mBaseCur = it->getPayOff().mBaseCur;
						payofftool.mNotionalCur = it->getPayOff().mNotionalCur;
						payofftool.mNotionalCFCur = payofftool.mNotionalCur;						
						payofftool.mNotionalCFExchangeRate = 1.0;	
						payofftool.mpFX_for_Notional = it->getPayOff().mpFX_for_Notional;
						payofftool.mPaymentDate = actiondate;
						payofftool.mPaymentTime = actiontime;
						payofftool.mNotionalCF = notional1st - notionalarray[0];
						payofftool.mpFX_for_PayOff = it->getPayOff().mpFX_for_PayOff;
						payofftool.calcPayOff();						
						//
						if (_action != NULL)
						{
							vector<PayOffToolHolderVector> dummy(payoff.size());
							vector<AQLPriceEventHolder*> dummy2;
							vector<PayOffToolHolderVector> tmp(payoff.size());
							vector<PayOffToolHolderIter> tmp_iter(payoff.size());
							tmp[mTargetLegNo[i]].push_back(ph);
							for (unsigned int j = 0; j < tmp_iter.size(); j++)
								tmp_iter[j] = tmp[j].begin();
							_action->doAction(_date, _time, tmp, dummy, dummy2, dummy2, tmp_iter);
							ph = tmp[mTargetLegNo[i]].back();
						}										
						extrapayoff[mTargetLegNo[i]].push_back(ph);
					
						if (mIsArrearPayment[i])
						{
							if (notionalarray.size() == 1)
								it->getPayOff().mNotionalCF = notionalarray[0];
							else
								it->getPayOff().mNotionalCF = notionalarray[0] - notionalarray[1];
						}
					}
					else
					{
						if (mIsArrearPayment[i])
						{
							if (notionalarray.size() <= i + 1)
								it->getPayOff().mNotionalCF = notionalarray[k];
							else
								it->getPayOff().mNotionalCF = notionalarray[k] - notionalarray[k + 1];
						}
						else
							it->getPayOff().mNotionalCF = notionalarray[k - 1] - notionalarray[k];

						//it->getPayOff().mNotionalCF -= (notionalarray[k - 1] - notionalarray[k]) * (1.0 / mChangeRatio - 1.0);	
					}
				}
				k++;
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
AQLPriceEventNotionalChange::setUp(const AQLDate& basedate,	
									const AQLObject& trade,
									AQLObject& triggerinfo,
									const AQLPricePayOff& payoff,
									bool isCall)
{
	AQLPriceEventBase::setUp(basedate, trade, triggerinfo, payoff, isCall);
	
	
/*	mPayOff.resize(mTargetLegNo.size());
	for(unsigned int i = 0; i < mTargetLegNo.size(); i++)
	{
		const PayOffToolHolderVector& vec = payoff.getPayOff()[mTargetLegNo[i]];
		mPayOff[i].resize(vec.size());
		for (unsigned int j = 0; j < vec.size(); j++)
			mPayOff[i][j].setPayOff(new AQLPricePayOffTool(vec[j].getPayOff()));
	}*/
	
	const AQLDataHolder* dh;
	// changeRatio
	dh = &(triggerinfo.getData(PRICING_CALIBRATION_DATAOTIONALCHANGERATIO, ISNOTNULL));
	mChangeRatio = dynamic_cast<const AQLDataDouble&>(dh->get()).get();

	//leg
	dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const AQLDataMultiReference& legs = dynamic_cast<const AQLDataMultiReference&>(dh->get());

	mIsArrearPayment.clear();
	mIsNotionalExchangeAtEnd.clear();
	mIsAmortize.clear();
	mAmortize1stFraction.clear();
	mIsAmortizeAmountSetting.clear();
	mAmortizeAmount.clear();
	mAmortizeRoundFunction.clear();
	mAmortizeRoundDigit.clear();
	for (unsigned int i = 0; i < mTargetLegNo.size(); i++)
	{
		// isArrear
		dh = &(legs.get(mTargetLegNo[i]).getData(PRICING_DATA_PAYMENTTIMING, ISNOTNULL));
		const AQLString& timing = dynamic_cast<const AQLDataString&>(dh->get()).get(); 
		mIsArrearPayment.push_back(AQLPriceCFGenUtility::isArrear(timing));

		// isNotionalExchangeAtEnd
		dh = &(legs.get(mTargetLegNo[i]).getData(PRICING_DATA_ISNOTIONALEXCHANGEATEND, ISNOTNULL));
		mIsNotionalExchangeAtEnd.push_back(dynamic_cast<const AQLDataBool&>(dh->get()).get());
		
		// isAmortize
		dh = &(legs.get(mTargetLegNo[i]).getData(PRICING_DATA_ISAMORTIZE, ISNOTNULL));
		mIsAmortize.push_back(dynamic_cast<const AQLDataBool&>(dh->get()).get());
	
		if (!mIsAmortize.back()) continue;

		// IsFirstFraction
		dh = &(legs.get(mTargetLegNo[i]).getData(PRICING_DATA_ISAMORTIZE1STFRACTION, ISNOTNULL));		
		mAmortize1stFraction.push_back(dynamic_cast<const AQLDataBool&>(dh->get()).get());

		// AmortizeType
		dh = &(legs.get(mTargetLegNo[i]).getData(PRICING_DATA_AMORTIZETYPE, ISNOTNULL));
		AQLString amortize_type = dynamic_cast<const AQLDataString&>(dh->get()).get();
		amortize_type.toUpper();
		if (amortize_type == EQUALIZATION)
		{
			mIsAmortizeAmountSetting.push_back(false);
			mAmortizeAmount.push_back(0);//dummy
			
			dh = &(legs.get(mTargetLegNo[i]).getData(PRICING_DATA_AMORTIZEROUNDFUNCTION, ISNOTNULL));
			mAmortizeRoundFunction.push_back(dynamic_cast<const AQLDataString&>(dh->get()).get());
			dh = &(legs.get(mTargetLegNo[i]).getData(PRICING_DATA_AMORTIZEROUNDDIGIT, ISNOTNULL));
			mAmortizeRoundDigit.push_back(dynamic_cast<const AQLDataInt&>(dh->get()).get());			

		}
		else if (amortize_type == AMOUNTSETTING)
		{
			mIsAmortizeAmountSetting.push_back(true);
			dh = &(legs.get(mTargetLegNo[i]).getData(PRICING_DATA_AMORTIZEAMOUNT, ISNOTNULL));
			mAmortizeAmount.push_back(dynamic_cast<const AQLDataDouble&>(dh->get()).get());
			mAmortizeRoundFunction.push_back("");//dummmy
			mAmortizeRoundDigit.push_back(0);//dummy
		}
		else
		{
			//error
			AQLString msg = "AmortizeType : " + amortize_type;
			msg += " is not support";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}	

	}

}


/*!
	@brief get amortize notional array after notional change
    @param[in] actiondate action date
    @param[in] payoff payoff
    @param[in] iter position of payoff
    @param[in] isarrear arrear payment or advance payment
    @param[in] changeratio notional change ratio
    @param[in] roundfunction round function
    @param[in] roundfunction round function
    @param[in] rounddigit round digit
    @param[in] is1stfraction first fraction or not
    @param[in] isamountsetting amortize amotunsetting or not
    @param[in] amount one time amount (amortize amotunsetting case, no use for amortize equalization case)
*/
DoubleArray
AQLPriceEventNotionalChange::getNotionalArray(const AQLDate& actiondate, 
												 const PayOffToolHolderVector& payoff, 
												 const PayOffToolHolderIter& iter,
												 bool isarrear,
												 double changeratio,
												 const AQLString& roundfunction,
												 int rounddigit,
												 bool is1stfraction,
												 bool isamountsetting,
												 double amount) const
{
	DoubleArray ret;
	
	PayOffToolHolderIter it;
	unsigned int count = 0;
	double notional1st = 0;
	for (it = iter; it != payoff.end(); it++)
	{
		if (!it->getPayOff().isCouponPayment()) continue;
		if (it->getPayOff().getPaymentDate() < actiondate)continue;
		if (isarrear && it->getPayOff().getPaymentDate() == actiondate) continue;
		if (count == 0)
			notional1st = it->getPayOff().mNotional * changeratio;
		count++;
	}
	if (count == 0) return ret;
	double amount_;

	//fraction
	if (isamountsetting)
	{
		amount_ = amount * changeratio;
		amount_ = AQLPriceCFGenUtility::round(amount_, ROUND_DOWN, 0);
	}
	else
	{
		amount_ = notional1st / count;
		amount_ = AQLPriceCFGenUtility::round(amount_, roundfunction, rounddigit);
	}
	double fraction = notional1st - amount_ * count;
	double remains = notional1st;
	for (unsigned int i = 0; i < count; i++)
	{
		ret.push_back(remains);
		if (i == count - 1) break;
		if (is1stfraction && i == 0) remains -= (amount_ + fraction);
		else remains -= amount_;
	}

	return ret;

}
