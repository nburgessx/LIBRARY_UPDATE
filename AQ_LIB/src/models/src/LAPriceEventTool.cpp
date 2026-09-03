
#ifdef __GNUG__
#pragma implementation
#endif
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include "LAPriceEventTool.h"

#include "LADataHolder.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataMatrix.h"
#include "LADataReference.h"
#include "LADataMultiReference.h"
#include "LAObject.h"
#include "LAObjectHolder.h"
#include "LAObjectPool.h"
#include "LADataInstance.h"

#include "LARatesNumeraireBase.h"
#include "LARatesSDEBase.h"

#include "LAPriceDataCalendar.h"
#include "LAPriceDataSlidingRule.h"
#include "LAPriceDataFunction.h"
#include "LAPriceDataDayCount.h"
#include "LAMathPathEntity.h"
#include "LAMathIndexEntity.h"
#include "LAMathDefine.h"
#include "LAPriceCFGenUtility.h"

#include "LAPriceEventExtraCF.h"
#include "LAPriceEventOption.h"
#include "LAPriceEventTerminate.h"
#include "LAPriceEventCpnChange.h"
#include "LAPriceEventNotionalChange.h"
#include "LAPriceEventNotExCurChange.h"
#include "LAPriceIndexTool.h"
#include "LAMathPlainVanillaEntity.h"

using namespace std;

#define ONETIME		"ONETIME"
#define MULTITIME	"MULTITIME"



#define FIXED_RATE	"FIXEDRATE"
#define CPN		"CPN"
#define CPNCF	"CPNCF"
#define LEG		"LEG"

#define RCV	"RCV"
#define PAY	"PAY"


/*!
    @brief Constructor
*/
LAPriceEventTool::LAPriceEventTool()
: mEffectiveFlag(true), mpOperator(0)
{

}
/*!
    @brief Destructor

*/
LAPriceEventTool::~LAPriceEventTool()
{
	clear();
}

/*!
	@brief set up this class
    @param[in] basedate basedate
    @param[in] trade trade object
    @param[in,out] triggerinfo trigger/call information object
    @param[in] payoff payoff
    @param[in] isCall call flag(true:call,false:trigger)
    @param[in] isLSMC lsmcl mode flag(true:lsmc mode,false:not lsmc mode)
*/
void
LAPriceEventTool::setUp(const LADate& basedate, const LAObject& trade,
						LAObject& trigger,
						const LAPricePayOff& payoff,
						unsigned int id,
						bool isCall,
						bool isLSMC)
{
	clear();

	const LADataHolder* dh;
	//one time or multi time
	bool isOneTime;
	if (isCall) isOneTime = true;
	else
	{
		dh = &(trigger.getData(PRICING_DATA_TRIGGERTYPE, ISNOTNULL));
		LAString triggertype = dynamic_cast<const LADataString&>(dh->get()).get();
		triggertype.toUpper();
		if (triggertype == MULTITIME) isOneTime = false;
		else if (triggertype == ONETIME) isOneTime = true;
		else
		{
			//error
			LAString msg = "TriggerType: ";
			msg += triggertype;
			msg += " is not support";
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);			
		}
	}

	//expiry dates
	dh = &(trigger.getData(PRICING_DATA_EXPIRYDATES, ISNOTNULL));
	const DateVector& expirydates = dynamic_cast<const LADataDates&>(dh->get()).get();

	//action dates
	dh = &(trigger.getData(PRICING_DATA_ACTIONDATES, ISNOTNULL));
	const DateVector& actiondates = dynamic_cast<const LADataDates&>(dh->get()).get();
	if (expirydates.size() != actiondates.size())
	{
		//error
		LAString msg = "ExpiryDates size is not same as ActionDates size";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);

	}
	
	for (unsigned int i = 0; i < expirydates.size(); i++)
	{
		if (expirydates[i] > actiondates[i])
		{
			//error
			LAString msg = "action date is befor expiry date! Check input.";
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);			
		}		
	}	


	const DateVector* pExtracfdates = NULL;
	if (isCall)
	{
		dh = &(trigger.getData(PRICING_DATA_EXTRACFDATES, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			pExtracfdates = &dynamic_cast<const LADataDates&>(dh->get()).get();
			if (expirydates.size() != pExtracfdates->size())
			{
				//error
				LAString msg = "ExpiryDates size is not same as ExtraCFDates size";
				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
		
			for (unsigned int i = 0; i < expirydates.size(); i++)
			{
				if (expirydates[i] > (*pExtracfdates)[i])
				{
					//error
					LAString msg = "ExtraCF date is befor expiry date! Check input.";
					throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);			
				}		
			}	
		}
	}

	unsigned int startpos = 0;
	for (unsigned int i = 0; i < expirydates.size(); i++)
	{
		if (basedate >= expirydates[i]) startpos++;
		else 
			break;
	}

	//operator
	dh = &(trigger.getData(PRICING_DATA_OPERATOR, ISNOTNULL));
	const LAFunctionBase& method = dynamic_cast<const LAPriceDataFunction&>(dh->get()).getFunction();
    mpOperator = dynamic_cast<LAFunctionBase*>(method.clone());
	
	//coefficient
	bool isChangeTriggerTargetValue = false;
	const DoubleMatrix* pCoeff = 0;
    if (!isLSMC)
	{
		dh = &(trigger.getData(PRICING_DATA_COEFFICIENTS, ISNOTNULL));
		pCoeff = &dynamic_cast<const LADataDoubleMatrix&>(dh->get()).get();
		if (pCoeff->size() == 1) mpOperator->setParam((*pCoeff)[0]);
		else if (pCoeff->size() == expirydates.size() || pCoeff->size() == expirydates.size() - startpos)
			isChangeTriggerTargetValue = true;
		else
		{
			//error
			LAString msg = "Coefficient column size must be one or same as ExpiryFDates size";
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
	}
	else
		isChangeTriggerTargetValue = true;
		
	
	////////////
	///mIndex///
	////////////
	mIndex.resize(expirydates.size() - startpos);
	//index info
	dh = &(trigger.getData(PRICING_DATA_INDEXINFOS, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		const LADataMultiReference& indexinfos
				= dynamic_cast<const LADataMultiReference&>(dh->get());
		
		for (unsigned int i = 0; i < mIndex.size(); i++)
			mIndex[i].resize(indexinfos.getSize(), NULL);
		
		for (unsigned int i = 0; i < indexinfos.getSize(); i++)
		{
			//index type
			dh = &(indexinfos.get(i).getData(PRICING_DATA_INDEXTYPE, ISNOTNULL));		
			LAString indextype = dynamic_cast<const LADataString&>(dh->get()).get();
			indextype.toUpper();
			
			if (indextype == FIXED_RATE)
				for (unsigned int j = 0; j < mIndex.size(); j++)	
					mIndex[j][i] = new LAPriceIndexToolFixed();
			else if (indextype == CPN)
				for (unsigned int j = 0; j < mIndex.size(); j++)		
					mIndex[j][i] = new LAPriceIndexToolCpn();
			else if (indextype == CPNCF)
				for (unsigned int j = 0; j < mIndex.size(); j++)			
					mIndex[j][i] = new LAPriceIndexToolCpnCF();
			else
			{
				for (unsigned int j = 0; j < mIndex.size(); j++)
				//	if (basedate >= expirydates[j])
				//		mIndex[j][i] = new LAPriceIndexToolFixed();
				//	else
						mIndex[j][i] = new LAPriceIndexTool();		
			}
			for (unsigned int j = 0; j < mIndex.size(); j++)
				mIndex[j][i]->setUp(basedate, trade, indexinfos.get(i).get(), expirydates[startpos + j], payoff);		
		}
	}

	//
	if (isCall)
		dh = &(trigger.getData(PRICING_DATA_LSMCVARIABLES, NOCHECK)); 
	else
		dh = &(trigger.getData(PRICING_DATA_TRIGGERTARGETS, NOCHECK)); 
	
	if (dh->isDefined() && !dh->isNull())
	{
		const LAStringVector& names = dynamic_cast<const LADataStrings&>(dh->get()).get();
		
		unsigned int oldsize = mIndex.at(0).size();
		for (unsigned int i = 0; i < mIndex.size(); i++)
			mIndex[i].resize(oldsize + names.size());	
		
		LAString name;
		for (unsigned int i = 0; i < names.size(); i++)
		{
			name = names[i];
			name.toUpper();
			if (name.findString(CPN) == 0)// Coupon
			{
				LAObject tmp;
				tmp.add(PRICING_DATA_INDEXTYPE, new LADataString(CPN));   
				tmp.add(PRICING_DATA_ACCESSORY, new LADataString(LEG + name.subString(3, name.size() - 1)));

				for (unsigned int j = 0; j < mIndex.size(); j++)
				{
					mIndex[j][oldsize + i] = new LAPriceIndexToolCpn();
					mIndex[j][oldsize + i]->setUp(basedate, trade, tmp, expirydates[startpos + j], payoff);		
				}

			}
			else if (name.findString(CPNCF) == 0)//CouponCF
			{
				LAObject tmp;
				tmp.add(PRICING_DATA_INDEXTYPE, new LADataString(CPNCF));   
				tmp.add(PRICING_DATA_ACCESSORY, new LADataString(LEG + name.subString(5, name.size() - 1)));
				for (unsigned int j = 0; j < mIndex.size(); j++)
				{
					mIndex[j][oldsize + i] = new LAPriceIndexToolCpnCF();	
					mIndex[j][oldsize + i]->setUp(basedate, trade, tmp, expirydates[startpos + j], payoff);		
				}
			}
			else//index
			{
				LAObjectPool& objPool = trade.getDataInstance()->getObjectPool();
				const LAObject& tmp = objPool.getObject(names[i], ENCHKTYPE_ISDEFINED).get();
			
				//index type
				dh = &(tmp.getData(PRICING_DATA_INDEXTYPE, ISNOTNULL));		
				LAString indextype = dynamic_cast<const LADataString&>(dh->get()).get();
				indextype.toUpper();
				
				if (indextype == FIXED_RATE)
					for (unsigned int j = 0; j < mIndex.size(); j++)	
						mIndex[j][oldsize + i] = new LAPriceIndexToolFixed();
				else if (indextype == CPN)
					for (unsigned int j = 0; j < mIndex.size(); j++)		
						mIndex[j][oldsize + i] = new LAPriceIndexToolCpn();
				else if (indextype == CPNCF)
					for (unsigned int j = 0; j < mIndex.size(); j++)			
						mIndex[j][oldsize + i] = new LAPriceIndexToolCpnCF();
				else
				{
					for (unsigned int j = 0; j < mIndex.size(); j++)
					//	if (basedate >= expirydates[j])
					//		mIndex[j][oldsize + i] = new LAPriceIndexToolFixed();
					//	else
							mIndex[j][oldsize + i] = new LAPriceIndexTool();		
				}
				for (unsigned int j = 0; j < mIndex.size(); j++)
					mIndex[j][i]->setUp(basedate, trade, tmp, expirydates[startpos + j], payoff);		

			}
		}
	}
	///////////////////
	///mActionMaster///
	///////////////////
	if (!isCall) // case trigger info
	{
		//extra cf
		dh = &(trigger.getData(PRICING_DATA_EXTRACFFUNC, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
			mActionMaster.push_back(new LAPriceEventExtraCF());
		//option
		dh = &(trigger.getData(PRICING_DATA_OPTION, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
			mActionMaster.push_back(new LAPriceEventOption());
	
		//notional exchange currency change
		dh = &(trigger.getData(PRICING_CALIBRATION_DATAOTIONALEXCHANGECURRENCY, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
			mActionMaster.push_back(new LAPriceEventNotExCurChange());

		
		//terminate
		dh = &(trigger.getData(PRICING_DATA_ISTERMINATE, NOCHECK));
		if (dh->isDefined() && !dh->isNull()
			&& dynamic_cast<const LADataBool&>(dh->get()).get())
				mActionMaster.push_back(new LAPriceEventTerminate());
		else
		{
			//coupon change
			dh = &(trigger.getData(PRICING_DATA_COUPONCHANGEINFO, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
				mActionMaster.push_back(new LAPriceEventCpnChange());
			else
			{
				dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
				const LADataMultiReference& legs = dynamic_cast<const LADataMultiReference&>(dh->get());
				for (unsigned int i = 0; i < legs.getSize(); i++)
				{
					dh = &(trigger.getData(PRICING_DATA_COUPONINFOS + LADataInt(i + 1).convertToString(), NOCHECK));
					if (dh->isDefined() && !dh->isNull())
					{
						mActionMaster.push_back(new LAPriceEventCpnChange());
						break;
					}
				}
			}
			//notional change
			dh = &(trigger.getData(PRICING_CALIBRATION_DATAOTIONALCHANGERATIO, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
				mActionMaster.push_back(new LAPriceEventNotionalChange());
		}
		if (mActionMaster.size() == 0)
		{
			//error
			throw LACoreInvalidData("No Trigger Action.", __FILE__, __LINE__);			
		}

	}
	else // case call info 
	{
		mActionMaster.push_back(new LAPriceEventTerminate());
		dh = &(trigger.getData(PRICING_DATA_EXTRACFFUNC, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
			mActionMaster.push_back(new LAPriceEventExtraCF());
	}

	for (unsigned int i = 0; i < mActionMaster.size(); i++)
	{
		mActionMaster[i]->setUp(basedate, trade, trigger, payoff, isCall);
	}


	//path object
	dh = &(trade.getData(PRICING_DATA_PATHENTITY, ISNOTNULL));
	const LADataReference& ref = dynamic_cast<const LADataReference&>(dh->get());
	const LAPriceDataDayCount* pdc_path=NULL;
	if (ref.get().get().isTypeOf(ENTITY_PLAINVANILLA))
	{
		const LAMathPlainVanillaEntity& path = dynamic_cast<const LAMathPlainVanillaEntity&>(ref.get().get());
		pdc_path = &(path.getDayCount());
	}
	else if (ref.get().get().isTypeOf(ENTITY_PATH))
	{
		const LAMathPathEntity& path = dynamic_cast<const LAMathPathEntity&>(ref.get().get());
		pdc_path = &(path.getDayCount());
	}

	const LARatesNumeraireBase* pNumeraire = NULL;
	BoolVector	isRcvPay;
	if (isCall)
	{
		const LAMathPathEntity& path = dynamic_cast<const LAMathPathEntity&>(ref.get().get());


		// base currency
		dh = &(trade.getData(PRICING_DATA_CURRENCY, ISNOTNULL));
		const LAString& cur = dynamic_cast<const LADataString&>(dh->get()).get();

		const LAStringVector& names = path.getSDEAttrNames().get();
		for (unsigned int i = 0; i < names.size(); i++)
		{
			dh = &path.getData(names[i], ISNOTNULL);
			const LAMathAttrSDE& attr = dynamic_cast<const LAMathAttrSDE&>(dh->get());
			if (attr.getSDEPathType() != IR || attr.getCurrency() != cur) continue;
			pNumeraire = attr.getSDE().getNumeraire();
		}
		if (pNumeraire == NULL)
		{
			//error
			throw LACoreInvalidData("Numeraire does not exist", __FILE__, __LINE__);	
		}

		//leg object
		dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
		const LADataMultiReference& legs = dynamic_cast<const LADataMultiReference&>(dh->get());
		isRcvPay.resize(legs.getSize());
		for (unsigned int i = 0; i < legs.getSize(); i++)
		{
			dh = &(legs.get(i).getData(PRICING_DATA_SELECTSIDE, ISNOTNULL));
			LAString sltside = dynamic_cast<const LADataString&>(dh->get()).get();
			sltside.toUpper();
			if (sltside == RCV) isRcvPay[i] = true;
			else if (sltside == PAY) isRcvPay[i] = false;
			else
			{
				//error
				LAString msg = PRICING_DATA_SELECTSIDE;
				msg += ": " + sltside;
				msg += " is a wrong input";
				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);	
			}

		}
	}

	mAction.resize(expirydates.size() - startpos);
	int ii = 0;
	if (!isLSMC) ii = pCoeff->size() - 1;
	for (int i = mAction.size() - 1; i >= 0; i--, ii--)
	{
		mAction[i] = new LAPriceEventHolder();
		mAction[i]->mID = id;
		mAction[i]->mExpiryDate = expirydates[startpos + i];
		mAction[i]->mExpiryTime = pdc_path->getTerm(basedate, mAction[i]->mExpiryDate);
		if (isCall && pExtracfdates != NULL)
		{
			mAction[i]->mActionDates.push_back(actiondates[startpos + i]);
			mAction[i]->mActionDates.push_back((*pExtracfdates)[startpos + i]);
			mAction[i]->mActionTimes.push_back(pdc_path->getTerm(basedate, actiondates[startpos + i]));
			mAction[i]->mActionTimes.push_back(pdc_path->getTerm(basedate, (*pExtracfdates)[startpos + i]));

		}
		else
		{
			mAction[i]->mActionDates.resize(mActionMaster.size(), actiondates[startpos + i]);
			mAction[i]->mActionTimes.resize(mActionMaster.size(), pdc_path->getTerm(basedate, actiondates[startpos + i]));
		}
		mAction[i]->mIsOneTime = isOneTime;
		mAction[i]->mpEffectiveFlag = &mEffectiveFlag;
		mAction[i]->mIndex = mIndex[i];
		mAction[i]->mAction = mActionMaster;
		if (isChangeTriggerTargetValue)
		{	
			if (!isLSMC) mpOperator->setParam((*pCoeff)[ii]);
			mAction[i]->mpOperator = dynamic_cast<LAFunctionBase*>(mpOperator->clone());
			mAction[i]->mOperatorHolder.set(mAction[i]->mpOperator, true);
		}
		else 
		{	
			mAction[i]->mpOperator = mpOperator;
			mAction[i]->mOperatorHolder.set(mAction[i]->mpOperator, false);
		}
		
		if (isCall)
		{
			mAction[i]->mpNumeraire = pNumeraire;
			mAction[i]->mRcvPay = isRcvPay;
		}
		
		mAction[i]->setUp(basedate, trade, trigger, payoff, isCall);
	}

}

/*!
	@brief clear this class members
*/
void
LAPriceEventTool::clear()
{
	if (mpOperator != NULL) delete mpOperator;
	mpOperator = NULL;

	unsigned int i, j;
	for (i = 0; i < mIndex.size(); i++)
	{
		for (j = 0; j < mIndex[i].size(); j++)
		{
			if (mIndex[i][j] != NULL) delete mIndex[i][j];
		}
		mIndex[i].clear();
	}
	mIndex.clear();

	for (i = 0; i < mActionMaster.size(); i++)
		if (mActionMaster[i] != NULL) delete mActionMaster[i];
	mActionMaster.clear();

	for (i = 0; i < mAction.size(); i++)
		if (mAction[i] != NULL) delete mAction[i];
	mAction.clear();
}

