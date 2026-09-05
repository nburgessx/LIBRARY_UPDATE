/*! @file
    @brief source code of abstract base class of trigger action and holder class of trigger action class.
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

#include "AQLPriceEventBase.h"
#include "AQLPriceEventExtraCF.h"
#include "AQLPriceEventTerminate.h"


#include "AQLDataHolder.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataReference.h"
#include "AQLDataMultiReference.h"
#include "AQLObject.h"
#include "AQLObjectHolder.h"

#include "AQLAlgorithm.h"
#include "AQLRatesNumeraireBase.h"
#include "AQLModelDynamicsCurve.h"

#include "AQLMathDefine.h"
#include "AQLMathFXEntity.h"

#include "AQLPricePayOff.h"

using namespace std;

#define LEG		"LEG"
#define BOTH	"BOTH"

/*!
    @brief constructor
*/
AQLPriceEventBase::AQLPriceEventBase()
: AQLCoreFunctionBase()
{

}
/*!
    @brief destructor

*/
AQLPriceEventBase::~AQLPriceEventBase()
{

}

/*
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLPriceEventBase::isTypeOf(function_t id) const
{
	return (id == FN_IR_TRIGGERACTIONBASE ? true : AQLCoreFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this class type
    @return function type
*/
function_t			
AQLPriceEventBase::getType() const
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
AQLPriceEventBase::setUp(const AQLDate& basedate,	
								const AQLObject& trade,
								AQLObject& triggerinfo,
								const AQLPricePayOff& payoff,
								bool isCall)
{
	mTargetLegNo.clear();

	if (isCall) return;


	(void)payoff; (void)basedate; // gcc
	const AQLDataHolder* dh;
	//leg object
	dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const AQLDataMultiReference& legs = dynamic_cast<const AQLDataMultiReference&>(dh->get());

	dh =  &(triggerinfo.getData(PRICING_DATA_TARGETLEG, ISNOTNULL));
	AQLString target = dynamic_cast<const AQLDataString&>(dh->get()).get();
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
			AQLString msg = PRICING_DATA_TARGETLEG;
			msg += ": " + target;
			msg += " is a wrong input";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);	
		}
		mTargetLegNo.push_back(legNo - 1);
	}

}

/*!
    @brief constructor
*/
AQLPriceEventHolder::AQLPriceEventHolder()
: mpEffectiveFlag(0), mpOperator(0), mpNumeraire(0) 
{

}
/*!
    @brief destructor

*/
AQLPriceEventHolder::~AQLPriceEventHolder()
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
AQLPriceEventHolder::doAction(vector<PayOffToolHolderVector>& payoff,
								 vector<PayOffToolHolderVector>& extrapayoff,
								 vector<AQLPriceEventHolder*>& futureaction,
								 vector<AQLPriceEventHolder*>& pastaction,
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
AQLPriceEventHolder::calcCallRebate(std::vector<PayOffToolHolderVector>& payoff,
										std::vector<PayOffToolHolderVector>& extrapayoff,
										std::vector<AQLPriceEventHolder*>& futureaction,
										std::vector<AQLPriceEventHolder*>& pastaction,
										std::vector<PayOffToolHolderIter>& iter) const
{
	if (!mIsCall) return 0.0;

	double rebate = 0.0;

	AQLPricePayOffToolHolder ph;
	for (unsigned int i = 0; i < mAction.size(); i++)
	{
		if (mAction[i]->isTypeOf(FN_IR_TRIGGERACTIONEXTRACF))
		{
			const AQLPriceEventExtraCF* paction = dynamic_cast<const AQLPriceEventExtraCF*>(mAction[i]);
			double extracf = paction->calcExtraCF(mActionDates[i], mActionTimes[i], payoff, extrapayoff, futureaction, pastaction, iter, ph);

			const AQLPricePayOffTool& tool = ph.getPayOff(); 
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
				const AQLPriceEventTerminate* paction = dynamic_cast<const AQLPriceEventTerminate*>(mAction[i]);
				double accrued = paction->calcAccruedInterest(mActionDates[i], mActionTimes[i],	payoff, iter, k, ph);
				if (accrued != 0.0)
				{
					const AQLPricePayOffTool& tool = ph.getPayOff();
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
					const AQLPricePayOffTool& tool = ph.getPayOff();
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
AQLPriceEventHolder::calcIndex(DoubleArray& index)
{
	
	if (index.size() != mIndex.size())
		index.resize(mIndex.size());
	for (unsigned int i = 0; i < mIndex.size(); i++)
		index[i] = mIndex[i]->calcIndex();
}



