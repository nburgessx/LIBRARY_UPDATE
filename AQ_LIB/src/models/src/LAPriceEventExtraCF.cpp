/*! @file
    @brief source code of trigger action class(extra cf action).

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

#include "LAPriceEventExtraCF.h"

#include "LADataHolder.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataMatrix.h"
#include "LADataReference.h"
#include "LAObject.h"
#include "LAObjectHolder.h"
#include "LADataMultiReference.h"

#include "LAAlgorithm.h"
#include "LAConstant.h"

#include "LAMathDefine.h"
#include "LAPriceDataFunction.h"
#include "LAMathFXEntity.h"

#include "LAPricePayOff.h"
#include "LAPriceEventExtraCF.h"

using namespace std;

#define RCV			"RCV"
#define PAY			"PAY"

#define FIXEDRATE	"FIXEDRATE"
#define CPN			"CPN"
#define CPNCF		"CPNCF"
#define NOTIONALCF	"NOTIONALCF"
#define EXTRACF		"EXTRACF"
#define INDEX		"INDEX"


/*!
    @brief Constructor
*/
LAPriceEventExtraCF::LAPriceEventExtraCF()
: LAPriceEventBase()
, mpActionDates(NULL), mpExpiryDates(NULL)
, mpExtraCFFunc(NULL), mpCoefficients(NULL), mpFX_for_ExtraCF(NULL)
{

}
/*!
    @brief Destructor

*/
LAPriceEventExtraCF::~LAPriceEventExtraCF()
{
	destroy();
}

/*! 
	@brief destroy mIndex and mIndexMap
*/
void LAPriceEventExtraCF::destroy()
{
	for(unsigned int i = 0; i < mIndex.size(); i++)
	{
		for(unsigned int j = 0; j <mIndex[i].size(); j++)
		{
			if (mIndex[i][j] != NULL)
				delete mIndex[i][j];
		}
		mIndex[i].clear();
	}
	mIndex.clear();

	std::map<LADate, std::vector<LAPriceIndexToolBase*> >::iterator its = mIndexMap.begin();
	while(its != mIndexMap.end())
	{
		its->second.clear();
		++its;
	}
	mIndexMap.clear();
}
/*! 
	@brief copyIndex
	@param[in] v LAPriceEventExtraCF object
*/
void LAPriceEventExtraCF::copyIndex(const LAPriceEventExtraCF& v)
{
	std::map<LADate, std::vector<LAPriceIndexToolBase*> >::const_iterator it = v.mIndexMap.begin();
	while(it != v.mIndexMap.end())
	{
		std::vector<LAPriceIndexToolBase*>::const_iterator itv = it->second.begin();
		vector<LAPriceIndexToolBase*> c_vec;
		while(itv != it->second.end())
		{
			try
			{
				if ((*itv) == NULL)
				{
					c_vec.push_back(NULL);
				}
				else
				{
					c_vec.push_back((*itv)->clone());
				}
				++itv;
			}
			catch (bad_alloc & e)
			{
				throw LACoreSystemError(e.what(), __FILE__, __LINE__);
			}
		}
		// map insert
		mIndexMap.insert(make_pair(it->first, c_vec));
		mIndex.push_back(c_vec);
		++it;
	}
}

/*!
    @brief copy constructor
	@param[in] v LAPriceEventExtraCF object
*/
LAPriceEventExtraCF::LAPriceEventExtraCF(const LAPriceEventExtraCF& v)
: mpActionDates(v.mpActionDates), mpExpiryDates(v.mpExpiryDates), mpExtraCFFunc(v.mpExtraCFFunc)
, mpCoefficients(v.mpCoefficients), mInputsInfo(v.mInputsInfo), mpFX_for_ExtraCF(v.mpFX_for_ExtraCF)
, mBaseCur(v.mBaseCur), mExtraCFCur(v.mExtraCFCur), mPastCouponPayOff(v.mPastCouponPayOff)
, mPastExtraCF(v.mPastExtraCF), mPastNotionalExchange(v.mPastNotionalExchange) 
{
	copyIndex(v);
}

/*
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*
LAPriceEventExtraCF::clone() const    
{
    try 
	{
        return new LAPriceEventExtraCF(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }	

}


  // operator
LAPriceEventExtraCF&
LAPriceEventExtraCF::operator=(const LAPriceEventExtraCF &v)

{
	if (this == &v)
		return *this;
	mpActionDates = v.mpActionDates;
	mpExpiryDates = v.mpExpiryDates;
	mpExtraCFFunc = v.mpExtraCFFunc;
    mpCoefficients = v.mpCoefficients;
	mInputsInfo = v.mInputsInfo;
	mpFX_for_ExtraCF = v.mpFX_for_ExtraCF;
    mBaseCur = v.mBaseCur;
	mExtraCFCur = v.mExtraCFCur;
	mPastCouponPayOff = v.mPastCouponPayOff;
    mPastExtraCF = v.mPastExtraCF;
	mPastNotionalExchange = v.mPastNotionalExchange;

	//delete
	destroy();

	copyIndex(v);
	return *this;
}


/*
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LAPriceEventExtraCF::isTypeOf(function_t id) const
{
	return (id == FN_IR_TRIGGERACTIONEXTRACF ? true : LAPriceEventBase::isTypeOf(id));
}

/*!
    @brief Return this class type
    @return function type
*/
function_t			
LAPriceEventExtraCF::getType() const
{
	return FN_IR_TRIGGERACTIONEXTRACF;
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
LAPriceEventExtraCF::doAction(const LADate& actiondate,
								   double actiontime,
								   vector<PayOffToolHolderVector>& payoff,
								   vector<PayOffToolHolderVector>& extrapayoff,
								   vector<LAPriceEventHolder*>& futureaction,
								   vector<LAPriceEventHolder*>& pastaction,
								   vector<PayOffToolHolderIter>& iter)
{
	LAPricePayOffToolHolder ph;
	calcExtraCF(actiondate, actiontime, payoff, extrapayoff, futureaction, pastaction, iter, ph);
	for(unsigned int i = 0; i < mTargetLegNo.size(); i++)
	{
/*		LAPricePayOffToolHolder ph(new LAPricePayOffTool());
		LAPricePayOffTool& payofftool = ph.getPayOff();
		payofftool.mBaseCur = mBaseCur;
		payofftool.mExtraCFCur = mExtraCFCur;
		payofftool.mpFX_for_ExtraCF = mpFX_for_ExtraCF;
		payofftool.mPaymentDate = actiondate;
		payofftool.mPaymentTime = actiontime;
		payofftool.mExtraCF = extracf;	
		payofftool.calcPayOff();*/

		extrapayoff[mTargetLegNo[i]].resize(extrapayoff[mTargetLegNo[i]].size() + 1);
		extrapayoff[mTargetLegNo[i]][extrapayoff[mTargetLegNo[i]].size() - 1] = ph;
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
LAPriceEventExtraCF::setUp(const LADate& basedate,	
									const LAObject& trade,
									LAObject& triggerinfo,
									const LAPricePayOff& payoff,
									bool isCall)
{
	LAPriceEventBase::setUp(basedate, trade, triggerinfo, payoff, isCall);

	if (isCall)
	{
		mTargetLegNo.push_back(0);
	}

	const LADataHolder* cah;

	mpActionDates = NULL;
	mpExpiryDates = NULL;
	mpExtraCFFunc = NULL;
	mpCoefficients = NULL;
	mInputsInfo.clear();

	mIsMultiExtraCF = false;
	cah = &(triggerinfo.getData(PRICING_DATA_ISMULTIEXTRACF, NOCHECK));
	if (cah->isDefined() && !cah->isNull())
		mIsMultiExtraCF = dynamic_cast<const LADataBool&>(cah->get()).get();
	if(mIsMultiExtraCF)
	{
		//extra cf currencies
		cah = &(triggerinfo.getData(PRICING_DATA_EXTRACFCURRENCIES, ISNOTNULL));
		mExtraCFCurs = dynamic_cast<const LADataStrings&>(cah->get()).get();
	}

	//base currency
	cah = &(trade.getData(PRICING_DATA_CURRENCY, ISNOTNULL));
	mBaseCur = dynamic_cast<const LADataString&>(cah->get()).get();
	//extra cf currency
	cah = &(triggerinfo.getData(PRICING_DATA_EXTRACFCURRENCY, ISNOTNULL));
	mExtraCFCur = dynamic_cast<const LADataString&>(cah->get()).get();
	//fx rate for extra cf
	mpFX_for_ExtraCF = NULL;
	if (mBaseCur != mExtraCFCur || mIsMultiExtraCF)
	{
		cah = &(triggerinfo.getData(PRICING_DATA_EXTRACFFXRATE, ISNOTNULL));
		const LADataReference& ref = dynamic_cast<const LADataReference&>(cah->get());
		mpFX_for_ExtraCF = &dynamic_cast<const LAMathFXEntity&>(ref.get().get());	
	}

	//expiry dates
	cah = &(triggerinfo.getData(PRICING_DATA_EXPIRYDATES, ISNOTNULL));
	mpExpiryDates = &dynamic_cast<const LADataDates&>(cah->get()).get();	

	//extra cf dates
	if (isCall)
	{
		cah = &(triggerinfo.getData(PRICING_DATA_EXTRACFDATES, NOCHECK));
		if (!cah->isDefined() || cah->isNull())
			cah = &(triggerinfo.getData(PRICING_DATA_ACTIONDATES, ISNOTNULL));
		mpActionDates = &dynamic_cast<const LADataDates&>(cah->get()).get();
	}
	else
	{
		cah = &(triggerinfo.getData(PRICING_DATA_ACTIONDATES, ISNOTNULL));
		mpActionDates = &dynamic_cast<const LADataDates&>(cah->get()).get();	
	}
	
	//coefficients
	cah = &(triggerinfo.getData(PRICING_DATA_EXTRACFFUNCCOEFFICIENTS, ISNOTNULL));
	mpCoefficients = &dynamic_cast<const LADataDoubleMatrix&>(cah->get()).get();
	if (mpCoefficients->size() != 1 && mpCoefficients->size() != mpActionDates->size())
	{
		//error
		throw LACoreInvalidData("ExtraCFFuncCoefficients row size must be one or same as ActionDates size", __FILE__, __LINE__);				
	}

	if(mIsMultiExtraCF)
	{
		for(unsigned int i = 0; i < mpCoefficients->size(); ++i)
		{
			if((*mpCoefficients)[i].size() != mExtraCFCurs.size())
			{
				throw LACoreInvalidData("Invalid ExtraCFFuncCoefficients column size",__FILE__,__LINE__);				
			}
		}
	}
	
    //LADataHolder* index;
	LADataHolder* dh;
	//extra cf method
	dh = &(triggerinfo.getData(PRICING_DATA_EXTRACFFUNC, ISNOTNULL));
	mpExtraCFFunc = &dynamic_cast<LAPriceDataFunction&>(dh->get()).getFunction();	

	if (/*mpCoefficients != NULL && */mpCoefficients->size() == 1)
	{
		if(mIsMultiExtraCF)
		{
			DoubleMatrix temp = DoubleMatrix(mpActionDates->size(), (*mpCoefficients)[0]);
			dh = &(triggerinfo.getData(PRICING_DATA_EXTRACFFUNCCOEFFICIENTS, ISNOTNULL));
			dynamic_cast<LADataDoubleMatrix &>(dh->get()).set(temp);
			cah = &(triggerinfo.getData(PRICING_DATA_EXTRACFFUNCCOEFFICIENTS, ISNOTNULL));
			mpCoefficients = &dynamic_cast<const LADataDoubleMatrix&>(cah->get()).get();
		}
		else
		{
			mpExtraCFFunc->setParam((*mpCoefficients)[0]);
		}
	}

	//input info
	cah = &(triggerinfo.getData(PRICING_DATA_EXTRACFFUNCINPUTS, NOCHECK));
	if (cah->isDefined() && !cah->isNull())
	{
		const LAStringVector inputs = dynamic_cast<const LADataStrings&>(cah->get()).get();		
		mInputsInfo.resize(inputs.size());
		cah = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
		const LADataMultiReference& legs = dynamic_cast<const LADataMultiReference&>(cah->get());
		for (unsigned int i = 0; i < inputs.size(); i++)
		{
			LAString input = inputs[i];
			input.toUpper();
			unsigned int size;
			if (input.findString(CPNCF) == 0)
			{
				mInputsInfo[i].first = 0;
				size = 5;
			}
			else if (input.findString(NOTIONALCF) == 0)
			{
				mInputsInfo[i].first = 1;
				size = 10;
			}
			else if (input.findString(EXTRACF) == 0)
			{
				mInputsInfo[i].first = 2;
				size = 7;
			}
			else if (input.findString(INDEX) == 0)
			{
				cah = &(triggerinfo.getData(PRICING_DATA_EXTRACFINDEXINFOS, NOCHECK));
				if (!cah->isDefined() || cah->isNull())
				{
					//error
					throw LACoreInvalidData("INDEXTYPE is needed when EXTRACFFUNCINPUTS is index ", __FILE__, __LINE__);
				}	
				mInputsInfo[i].first = 3;
				unsigned int startpos = 0;
				for (unsigned int j = 0; j < mpActionDates->size(); j++)
				{
					if (basedate <= (*mpActionDates)[j])
					{
						break;
					}
					else
					{
						++startpos;
					}
				}
				mIndex.resize(mpActionDates->size() - startpos);
				for (unsigned int j = 0; j < mpActionDates->size() - startpos; j++)
				{
					// do
					const LADataMultiReference& infonum = dynamic_cast<const LADataMultiReference&>(cah->get());
					mIndex[j].resize(infonum.getSize());
					
					for (unsigned int k = 0; k < static_cast<unsigned int>(infonum.getSize()); k++)
					{
						LAString type = dynamic_cast<LADataString &>(infonum.get(k).getData(PRICING_DATA_INDEXTYPE, ISNOTNULL).get()).get();
						type.toUpper();
						if (type == FIXEDRATE)
						{
							mIndex[j][k] = new LAPriceIndexToolFixed;
						}
						else if (type == CPN)
						{
							mIndex[j][k] = new LAPriceIndexToolCpn;
						}
						else if (type == CPNCF)
						{
							mIndex[j][k] = new LAPriceIndexToolCpnCF;
						}
						else
						{
							mIndex[j][k] = new LAPriceIndexTool;
						}
						mIndex[j][k]->setUp(basedate, trade, infonum.get(k).get(), (*mpActionDates)[j + startpos], payoff);
					}
					mIndexMap.insert(make_pair((*mpActionDates)[j + startpos], mIndex[j]));
				}
			}
			else 
			{
				//error
				LAString msg = PRICING_DATA_EXTRACFFUNCINPUTS;
				msg += ": " + input;
				msg += " is a wrong input";
				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);				
			}
			if (input.findString(INDEX) == -1)
			{
				unsigned int legNo = 0;
				if (input.size() < size + 1 ||
					(legNo = (input.subString(size, input.size() - 1)).getIntValue()) == 0
					|| legs.getSize() < legNo)
				{
					//error
					LAString msg = PRICING_DATA_EXTRACFFUNCINPUTS;
					msg += ": " + input;
					msg += " is a wrong input";
					throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);				
				}
			
				mInputsInfo[i].second = legNo - 1;
			}
		}

		mPastCouponPayOff.resize(legs.getSize());
		mPastNotionalExchange.resize(legs.getSize());
		mPastExtraCF.resize(legs.getSize());
		for (unsigned int i = 0; i < legs.getSize(); i++)
		{			
			dh = &legs.get(i).getData(PRICING_DATA_COUPONPAYOFFS, NOCHECK);
			if (dh->isDefined() && ! dh->isNull())
				mPastCouponPayOff[i] = dynamic_cast<const LADataDoubles&>(dh->get()).get();
			dh = &legs.get(i).getData(PRICING_CALIBRATION_DATAOTIONALCFS, NOCHECK);
			if (dh->isDefined() && ! dh->isNull())
				mPastNotionalExchange[i] = dynamic_cast<const LADataDoubles&>(dh->get()).get();
			dh = &legs.get(i).getData(PRICING_DATA_EXTRACFS, NOCHECK);
			if (dh->isDefined() && ! dh->isNull())
				mPastExtraCF[i] = dynamic_cast<const LADataDoubles&>(dh->get()).get();
		}
	}
	else if (!mpExtraCFFunc->isTypeOf(FN_CONSTANT))
	{
		//error
		throw LACoreInvalidData("If ExtraCFFuncInputs is not set. ExtraCFFunc must be LAConstant", __FILE__, __LINE__);				

	}

	// settlement adjust ratio
	cah = &(triggerinfo.getData(PRICING_DATA_SETTLEMENTADJUSTRATIOS, NOCHECK));
	if (cah->isDefined() && !dh->isNull())
		mSettlementAdjustRatios = dynamic_cast<const LADataDoubles &>(cah->get()).get();
	else
		mSettlementAdjustRatios.resize(mpActionDates->size(), 1.0);

}

/*!
	@brief calculate extra cf
	@param[in] actiondate action date
	@param[in] actiontime action time
	@param[in,out] payoff payoff
	@param[in,out] extrapayoff extra payoff
	@param[in,out] futureaction future action
	@param[in,out] pastaction past action
	@param[in,out] iter position of nearest payoff from this action expiry date
*/	
double
LAPriceEventExtraCF::calcExtraCF(const LADate& actiondate,
								   double actiontime,
								   vector<PayOffToolHolderVector>& payoff,
								   vector<PayOffToolHolderVector>& extrapayoff,
								   vector<LAPriceEventHolder*>& futureaction,
								   vector<LAPriceEventHolder*>& pastaction,
								   vector<PayOffToolHolderIter>& iter,
								   LAPricePayOffToolHolder& ph) const
{
	(void)actiontime; (void)pastaction; (void)futureaction; //20070411--Nagase--警告削除をgccにも対応
	double extracf = 0;
	////product index
	//std::map<LADate, std::vector<LAPriceIndexToolBase*> >::const_iterator it = mIndexMap.begin();
	//vector<LAPriceIndexToolBase*> c_vec;
	//while(it != mIndexMap.end())
	//{

	unsigned int pos;
	if (LAAlgorithm::find<DateVector, LADate>((*mpActionDates), actiondate, 0, mpActionDates->size() - 1, pos))
		;
	else
	{
		//error
		LAString msg = "ExtraCF action date is something wrong";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	if (mInputsInfo.size() == 0)
	{	
		if (mpCoefficients->size() != 1)
		{
			if(mIsMultiExtraCF)
			{
				DoubleArray extracfsum(1, 0.0);
				for(unsigned int i = 0; i < mExtraCFCurs.size(); ++i)
				{
					double fx = 1.0;
					if (mExtraCFCurs[i] != mExtraCFCur)
					{
						fx = mpFX_for_ExtraCF->getRate(mExtraCFCurs[i], mExtraCFCur, actiontime);
					}
					extracfsum[0] += (*mpCoefficients)[pos][i] * fx;
				}
				mpExtraCFFunc->setParam(extracfsum);
			}
			else
			{
				mpExtraCFFunc->setParam((*mpCoefficients)[pos]);
			}
		}
		DoubleArray x(1);//dummy
		extracf = (*mpExtraCFFunc)(x);
	}
	else
	{		
		if (/*mpCoefficients != NULL && */mpCoefficients->size() != 1)
			mpExtraCFFunc->setParam((*mpCoefficients)[pos]);

		PayOffToolHolderIter it;
		DoubleArray x;
		for (unsigned int i = 0; i < mInputsInfo.size(); i++)
		{
			if (mInputsInfo[i].first == 0)
			{
				if (mPastCouponPayOff[mInputsInfo[i].second].size() != 0)
					x.insert(x.end(), mPastCouponPayOff[mInputsInfo[i].second].begin(), mPastCouponPayOff[mInputsInfo[i].second].end()); 
				for (it = payoff[mInputsInfo[i].second].begin(); it != iter[mInputsInfo[i].second]; it++)
				{
					if (!it->getPayOff().isCouponPayment()) continue;
					if (it->getPayOff().getPaymentDate() > (*mpExpiryDates)[pos]) break;
					x.push_back(it->getPayOff().getCpnPayOffbyPayOffCur());
				}
				for (it = extrapayoff[mInputsInfo[i].second].begin(); it != extrapayoff[mInputsInfo[i].second].end(); it++)
				{
					if (it->getPayOff().getPaymentDate() > (*mpExpiryDates)[pos]) continue;
					if (it->getPayOff().mAccruedInt == 0) continue;
					x.push_back(it->getPayOff().mAccruedInt);
				}	
			}
			else if (mInputsInfo[i].first == 1)
			{
				if (mPastNotionalExchange[mInputsInfo[i].second].size() != 0)
					x.insert(x.end(), mPastNotionalExchange[mInputsInfo[i].second].begin(), mPastNotionalExchange[mInputsInfo[i].second].end()); 
				for (it = extrapayoff[mInputsInfo[i].second].begin(); it != extrapayoff[mInputsInfo[i].second].end(); it++)
				{
					if (it->getPayOff().getPaymentDate() > (*mpExpiryDates)[pos]) continue;
					if (it->getPayOff().mNotionalCF == 0) continue;
					x.push_back(it->getPayOff().mNotionalCF);
				}
			}
			else if (mInputsInfo[i].first == 2)
			{
				if (mPastExtraCF[mInputsInfo[i].second].size() != 0)
					x.insert(x.end(), mPastExtraCF[mInputsInfo[i].second].begin(), mPastExtraCF[mInputsInfo[i].second].end()); 
				for (it = extrapayoff[mInputsInfo[i].second].begin(); it != extrapayoff[mInputsInfo[i].second].end(); it++)
				{
					if (it->getPayOff().getPaymentDate() > (*mpExpiryDates)[pos]) continue;
					if (it->getPayOff().mExtraCF == 0) continue;
					x.push_back(it->getPayOff().mExtraCF);
				}
			}
			else if (mInputsInfo[i].first == 3)
			{
				map<LADate, std::vector<LAPriceIndexToolBase*> >::const_iterator its = mIndexMap.begin();
				its = mIndexMap.find(actiondate);
				//check
				if (its == mIndexMap.end())
				{
					//error
					LAString msg = "ActionDates don't exit in mIndexMap";
					throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
				}
				vector<LAPriceIndexToolBase*>::const_iterator itvec = its->second.begin();
				for (itvec = its->second.begin(); itvec != its->second.end(); itvec++)
					x.push_back((*itvec)->calcIndex());
			}
		}
		extracf = (*mpExtraCFFunc)(x);
	}
	ph.setPayOff(new LAPricePayOffTool());
	LAPricePayOffTool& payofftool = ph.getPayOff();
	payofftool.mBaseCur = mBaseCur;
	payofftool.mExtraCFCur = mExtraCFCur;
	payofftool.mpFX_for_ExtraCF = mpFX_for_ExtraCF;
	payofftool.mPaymentDate = actiondate;
	payofftool.mPaymentTime = actiontime;
	payofftool.mExtraCF = extracf;
	payofftool.mSettlementAdjustRatio = mSettlementAdjustRatios[pos];
	 
	return payofftool.calcPayOff();
}