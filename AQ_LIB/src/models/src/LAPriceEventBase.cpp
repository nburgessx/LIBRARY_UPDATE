/*! @file
    @brief source code of abstract base class of trigger action and holder class of trigger action class.

*/
//  2006, Mizuho International London..
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

#include "LAPriceEventBase.h"
#include "LAPriceEventExtraCF.h"
#include "LAPriceEventTerminate.h"


#include "LADataHolder.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataReference.h"
#include "LADataMultiReference.h"
#include "LAObject.h"
#include "LAObjectHolder.h"

#include "LAAlgorithm.h"
#include "LARatesNumeraireBase.h"
#include "LAModelDynamicsCurve.h"

#include "LAMathDefine.h"
#include "LAMathFXEntity.h"

#include "LAPricePayOff.h"

using namespace std;

#define LEG		"LEG"
#define BOTH	"BOTH"

/*!
    @brief constructor
*/
LAPriceEventBase::LAPriceEventBase()
: LACoreFunctionBase()
{

}
/*!
    @brief destructor

*/
LAPriceEventBase::~LAPriceEventBase()
{

}

/*
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LAPriceEventBase::isTypeOf(function_t id) const
{
	return (id == FN_IR_TRIGGERACTIONBASE ? true : LACoreFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this class type
    @return function type
*/
function_t			
LAPriceEventBase::getType() const
{
	return FN_IR_TRIGGERACTIONBASE;
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
LAPriceEventBase::setUp(const LADate& basedate,	
								const LAObject& trade,
								LAObject& triggerinfo,
								const LAPricePayOff& payoff,
								bool isCall)
{
	mTargetLegNo.clear();

	if (isCall) return;


	(void)payoff; (void)basedate; //20070411--Nagase--警告削除をgccにも対応
	const LADataHolder* dh;
	//leg object
	dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const LADataMultiReference& legs = dynamic_cast<const LADataMultiReference&>(dh->get());

	dh =  &(triggerinfo.getData(PRICING_DATA_TARGETLEG, ISNOTNULL));
	LAString target = dynamic_cast<const LADataString&>(dh->get()).get();
	target.toUpper();
	if (target == BOTH) 
	{
		mTargetLegNo.resize(legs.getSize());
		for (unsigned int i = 0; i < legs.getSize(); i++)
			mTargetLegNo[i] = i;	
	}
	else
	{
		unsigned int legNo = 1;
		if (target.size() < 4 ||
			(legNo = (target.subString(3, target.size() - 1)).getIntValue()) == 0
			|| legs.getSize() < legNo)
		{
			//error
			LAString msg = PRICING_DATA_TARGETLEG;
			msg += ": " + target;
			msg += " is a wrong input";
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);	
		}
		mTargetLegNo.push_back(legNo - 1);
	}

}

/*!
    @brief constructor
*/
LAPriceEventHolder::LAPriceEventHolder()
: mpEffectiveFlag(0), mpOperator(0), mpNumeraire(0) 
{

}
/*!
    @brief destructor

*/
LAPriceEventHolder::~LAPriceEventHolder()
{
}

/*!
		@brief check trigger hit and if hit, execute trigger action
	    @param[in,out] payoff payoff
	    @param[in,out] extrapayoff extra payoff
	    @param[in,out] futureaction future action
	    @param[in,out] pastaction past action
		@param[in,out] iter position of nearest payoff from this action expiry date
*/	
void
LAPriceEventHolder::doAction(vector<PayOffToolHolderVector>& payoff,
								 vector<PayOffToolHolderVector>& extrapayoff,
								 vector<LAPriceEventHolder*>& futureaction,
								 vector<LAPriceEventHolder*>& pastaction,
								 vector<PayOffToolHolderIter>& iter)
{
	unsigned int i;
	for (i = 0; i < payoff.size(); i++)
		for (; iter[i] != payoff[i].end() && iter[i]->getPayOff().getPaymentDate() <= futureaction[0]->mExpiryDate; iter[i]++)
			iter[i]->getPayOff().calcPayOff();
			
	if (!*mpEffectiveFlag)
	{	
		futureaction.erase(futureaction.begin());
		return;
	}
	

	if (!mIsCall)
		for (i = 0; i < mIndex.size(); i++)
			mX[i] = mIndex[i]->calcIndex();
	
	//calculate call rebate
/*	if (mIsCall)
	{
		mX[i] = calcCallRebate(payoff, extrapayoff, futureaction, pastaction, iter);
	}
	
	if ((mIsCall && mpOperator->operator ()(mX) <= 0)
		|| mpOperator->operator ()(mX) < 0) //not trigger hit
	{
		futureaction.erase(futureaction.begin());
		return;
	}
*/
	if (!mIsCall && mpOperator->operator ()(mX) < 0) //not trigger hit
	{
		futureaction.erase(futureaction.begin());
		return;
	}

	pastaction.push_back(futureaction[0]);
	futureaction.erase(futureaction.begin());

	if (mIsOneTime) *mpEffectiveFlag = false;

	for (i = 0; i < mAction.size(); i++)
		mAction[i]->doAction(mActionDates[i], mActionTimes[i], payoff, extrapayoff, futureaction, pastaction, iter);		

}

/*!
		@brief calculate call rebate (extracf and accured interest and notional exchange)
	    @param[in,out] payoff payoff
	    @param[in,out] extrapayoff extra payoff
	    @param[in,out] futureaction future action
	    @param[in,out] pastaction past action
		@param[in,out] iter position of nearest payoff from this action expiry date
		
		@return rebate value
	

*/	
double
LAPriceEventHolder::calcCallRebate(std::vector<PayOffToolHolderVector>& payoff,
										std::vector<PayOffToolHolderVector>& extrapayoff,
										std::vector<LAPriceEventHolder*>& futureaction,
										std::vector<LAPriceEventHolder*>& pastaction,
										std::vector<PayOffToolHolderIter>& iter) const
{
	if (!mIsCall) return 0.0;

	double rebate = 0.0;

	LAPricePayOffToolHolder ph;
	for (unsigned int i = 0; i < mAction.size(); i++)
	{
		if (mAction[i]->isTypeOf(FN_IR_TRIGGERACTIONEXTRACF))
		{
			const LAPriceEventExtraCF* paction = dynamic_cast<const LAPriceEventExtraCF*>(mAction[i]);
			double extracf = paction->calcExtraCF(mActionDates[i], mActionTimes[i], payoff, extrapayoff, futureaction, pastaction, iter, ph);

			const LAPricePayOffTool& tool = ph.getPayOff(); 
			if (tool.mExtraCFCur != tool.mBaseCur)
			{
				extracf *= tool.mpFX_for_ExtraCF->getRate
						(tool.mExtraCFCur, tool.mBaseCur, mActionTimes[i]);			
			}
			if (mExpiryTime != mActionTimes[i])
			{
				extracf *= (*mpNumeraire)(mExpiryTime) / (*mpNumeraire)(mActionTimes[i]);
			}
			if (mRcvPay[0])
			{
				rebate += extracf;
			}
			else
			{
				rebate -= extracf;
			}
		}
		else if (mAction[i]->isTypeOf(FN_IR_TRIGGERACTIONTERMINATE))
		{
			for (unsigned int k = 0; k < payoff.size(); k++)
			{
				const LAPriceEventTerminate* paction = dynamic_cast<const LAPriceEventTerminate*>(mAction[i]);
				double accrued = paction->calcAccruedInterest(mActionDates[i], mActionTimes[i],	payoff, iter, k, ph);
				if (accrued != 0.0)
				{
					const LAPricePayOffTool& tool = ph.getPayOff();
                    if (tool.mCouponsCur[tool.mSelectedCpnPos] != tool.mBaseCur)
					{
						accrued *= tool.mpFX_for_PayOff->getRate
							(tool.mCouponsCur[tool.mSelectedCpnPos], tool.mBaseCur, mActionTimes[i]);
					
					}
					if (mExpiryTime != mActionTimes[i])
					{
						accrued *= (*mpNumeraire)(mExpiryTime) / (*mpNumeraire)(mActionTimes[i]);
					}
									
					if (mRcvPay[k])
					{
						rebate += accrued;
					}
					else
					{
						rebate -= accrued;
					}
				}
				double notionalexchange = paction->calcNotionalExchange(mActionDates[i], mActionTimes[i], payoff, extrapayoff, pastaction, iter, k, ph);
				if (notionalexchange != 0.0)
				{
					const LAPricePayOffTool& tool = ph.getPayOff();
					if (tool.mNotionalCFCur != "" && tool.mNotionalCFCur != tool.mNotionalCur)
					{
						notionalexchange *= tool.mNotionalCFExchangeRate;
						if (tool.mNotionalCFCur != tool.mBaseCur)
						{
							notionalexchange *= tool.mpFX_for_PayOff->getRate
								(tool.mNotionalCFCur, tool.mBaseCur, mActionTimes[i]);
						
						}
					}
					else if (tool.mNotionalCur != tool.mBaseCur)
					{
						notionalexchange *= tool.mpFX_for_PayOff->getRate
							(tool.mNotionalCur, tool.mBaseCur, mActionTimes[i]);
					
					}
					if (mExpiryTime != mActionTimes[i])
					{
						notionalexchange *= (*mpNumeraire)(mExpiryTime) / (*mpNumeraire)(mActionTimes[i]);
					}
					if (mRcvPay[k])
					{
						rebate += notionalexchange;
					}
					else
					{
						rebate -= notionalexchange;
					}
				}										
			}	
		}		
	}
	return rebate;
}


/*!
	@brief calculate index
	@param[out] index index
*/	
void
LAPriceEventHolder::calcIndex(DoubleArray& index)
{
	
	if (index.size() != mIndex.size())
		index.resize(mIndex.size());
	for (unsigned int i = 0; i < mIndex.size(); i++)
		index[i] = mIndex[i]->calcIndex();
}



