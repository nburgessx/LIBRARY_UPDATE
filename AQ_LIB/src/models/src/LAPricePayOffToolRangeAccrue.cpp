/*! @file
    @brief source code of tool class of payoff calculation in range accrue.

*/
//  2007, Mizuho International London.


#ifdef __GNUG__
#pragma implementation
#endif
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include "LAPricePayOffToolRangeAccrue.h"

#include "LADataHolder.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataReference.h"
#include "LADataMultiReference.h"
#include "LAObject.h"
#include "LAObjectHolder.h"

#include "LAPriceDataCalendar.h"
#include "LAPriceDataSlidingRule.h"
#include "LAPriceDataFunction.h"
#include "LAMathIndexEntity.h"
#include "LAMathPathEntity.h"
#include "LAMathFXEntity.h"
#include "LAMathDefine.h"
#include "LAMathDateCalculations.h"

#include "LAAlgorithm.h"
#include "LABasic.h"

#include "LAPricePayOff.h"
#include "LAPriceCouponTool.h"
#include "LAPriceCFGenUtility.h"
#include "LAPriceCouponBase.h"

#define BUSINESS_DAYS "BUSINESS_DAYS"
#define DAILY "DAILY"

#define FIXEDRATE	"FIXEDRATE"
#define CPN			"CPN"
#define CPNCF		"CPNCF"

using namespace std;

/*!
    @brief default constructor
*/
LAPricePayOffToolRangeAccrue::LAPricePayOffToolRangeAccrue()
: LAPricePayOffTool(), mCalcDays(0), mIsAndCondition(false), mIsExcludeAndCondition(false), mIsNotCondition(false),
  mpHitRateOperator(0),  mIsHitRateCap(false), mIsHitRateFloor(false), mHitRateCap(1.0), mHitRateFloor(0.0), mpOperator(0),
  mIsRAStrike(false), mRAStrike(1.0)
{

}
/*!
    @brief destructor
*/
LAPricePayOffToolRangeAccrue::~LAPricePayOffToolRangeAccrue()
{
	for (unsigned int i = 0; i < mpRAImpls.size(); ++i)
	{
		delete mpRAImpls[i];
	}
}

/*!
    @brief copy constructor
	@param[in] v LAPricePayOffToolRangeAccrue object
*/
LAPricePayOffToolRangeAccrue::LAPricePayOffToolRangeAccrue(const LAPricePayOffToolRangeAccrue& v)
: LAPricePayOffTool(v), mCalcDays(v.mCalcDays), 
  mRAObservationStart(v.mRAObservationStart), mRAObservationEnd(v.mRAObservationEnd), mIsAndCondition(v.mIsAndCondition), mIsExcludeAndCondition(v.mIsExcludeAndCondition), mIsNotCondition(v.mIsNotCondition),
  mpHitRateOperator(v.mpHitRateOperator), mIsHitRateCap(v.mIsHitRateCap), mIsHitRateFloor(v.mIsHitRateFloor), mHitRateCap(v.mHitRateCap), mHitRateFloor(v.mHitRateFloor), mpOperator(v.mpOperator),
  mIsRAStrike(v.mIsRAStrike), mRAStrike(v.mRAStrike)
{
	for (unsigned int i = 0; i < v.mpRAImpls.size(); ++i)
	{
		mpRAImpls[i] = dynamic_cast<LAPricePayOffToolRangeAccrueImpl *>(v.mpRAImpls[i]->clone());
	}
}


/*!
    @brief calculate payoff
*/
double
LAPricePayOffToolRangeAccrue::calcPayOff(void) const
{
	LAPricePayOffTool::calcPayOff();
	mPayOff /= mSettlementAdjustRatio;

	// calc range accrue index
	for (unsigned int i = 0; i < mpRAImpls.size(); ++i)
	{
		mpRAImpls[i]->calcIndex();
	}

	// check in range ?
	unsigned int hitNum = 0;
	for (unsigned int i = 0; i < mCalcDays; ++i)
	{
		bool isHit = false;
		if (mIsAndCondition)
		{
			// and conditon
			isHit = true;
			for(unsigned int j = 0; j < mpRAImpls.size(); ++j)
			{
				if (!mpRAImpls[j]->isHit(i))
				{
					isHit = false;
					break;
				}
			}
		}
		else
		{
			// or conditon
			isHit = false;
			if (mIsExcludeAndCondition)
			{
				// exclude and condition
				bool isAllHit = true;
				for(unsigned int j = 0; j < mpRAImpls.size(); ++j)
				{
					if (mpRAImpls[j]->isHit(i))
					{
						isHit = true;
					}
					else
					{
						if (isAllHit)
						{
							isAllHit = false;
						}
					}
				}
				if (!isAllHit && isHit)
				{
					isHit = true;
				}
				else
				{
					isHit = false;
				}
			}
			else
			{
				for(unsigned int j = 0; j < mpRAImpls.size(); ++j)
				{
					if (mpRAImpls[j]->isHit(i))
					{
						isHit = true;
						break;
					}
				}
			}

		}
		if (isHit)
		{
			++hitNum;
		}
	}
	// not condition
	if (mIsNotCondition)
	{
		hitNum = mCalcDays - hitNum;
	}

	// calc range accrue hit rate
	double rateRA = 1.0;
	if (mCalcDays != 0)
	{
		rateRA = static_cast<double>(hitNum) / static_cast<double>(mCalcDays);
	}
	if (mIsRAStrike)
	{
		// check hit rate is less or greater than Stike.
		if (rateRA > mRAStrike)
		{
			rateRA = 1.0;
		}
	}
	if (mpHitRateOperator)
	{
		rateRA = mpHitRateOperator->operator ()(rateRA);
	}
	if (mIsHitRateCap)
	{
		rateRA = mHitRateCap < rateRA ? mHitRateCap : rateRA;
	}
    if (mIsHitRateFloor)
	{
		rateRA = mHitRateFloor > rateRA ? mHitRateFloor : rateRA;
	}
	mCoupon *= rateRA;

	// calc range accrue final coupon
	if (mpOperator)
	{
		mCoupon = mpOperator->operator ()(mCoupon);
	}

	// recalc coupon couponcf
	mPayOff -= mCouponPayOff;
	double cpncf = 0.0;
	if (mNotionalCur != mCouponsCur[mSelectedCpnPos])
	{
		cpncf = mCoupon * mTerm * mNotional
				* mpFX_for_Notional->getRate(mNotionalCur, mCouponsCur[mSelectedCpnPos], mPaymentTime);
	}
	else
	{
		cpncf = mCoupon * mTerm * mNotional;
	}

	cpncf /= mDenomiRatio;
	round(cpncf);
	cpncf *= mDenomiRatio;

	mCpnPayOffbyPayOffCur = cpncf;

	if (mBaseCur != mCouponsCur[mSelectedCpnPos])
	{
		cpncf *= mpFX_for_PayOff->getRate(mCouponsCur[mSelectedCpnPos], mBaseCur, mPaymentTime);
	}
	mCouponPayOff = cpncf;

	mPayOff += cpncf;
	mPayOff *= mSettlementAdjustRatio;

	return  mPayOff;
}



/*!
    @brief set up this class
	@param[in] basedate basedate
	@param[in] trade trade object
	@param[in] legNo leg number(first leg number = 0)
	@param[in] cashlet cashlet object
	@param[in] payoff LAPricePayOff object
	@param[in] current position of payoff(first payoff position = 0)
*/
void
LAPricePayOffToolRangeAccrue::setUp(const LADate& basedate, const LAObject& trade,
						unsigned int legNo, const LAObject& cashlet, 
						const LAPricePayOff& payoff,
						unsigned int currentpos)
{
	LAPricePayOffTool::setUp(basedate, trade, legNo, cashlet, payoff, currentpos);

	// days in calculation period
	const LADataHolder &ahOSD = cashlet.getData(PRICING_DATA_RANGEACCRUEOBSERVATIONSTARTDATE, NOCHECK);
	if (ahOSD.isDefined() && !ahOSD.isNull())
	{
		mRAObservationStart = dynamic_cast<const LADataDate &>(ahOSD.get()).get();
	}
	else
	{
		mRAObservationStart = mStart;
	}

	const LADataHolder &ahOED = cashlet.getData(PRICING_DATA_RANGEACCRUEOBSERVATIONENDDATE, NOCHECK);
	if (ahOED.isDefined() && !ahOED.isNull())
	{
		mRAObservationEnd = dynamic_cast<const LADataDate &>(ahOED.get()).get();
	}
	else
	{
		mRAObservationEnd = mEnd;
	}
	mCalcDays = mRAObservationStart.intervalDays(mRAObservationEnd);
	// index
	const LADataMultiReference* indexRef;
	const LADataHolder &ahInfos = cashlet.getData(PRICING_DATA_RANGEACCRUEINFOS, NOCHECK);
	if (ahInfos.isDefined() && !ahInfos.isNull()) //multi index case
	{
		const LADataMultiReference& infos = dynamic_cast<const LADataMultiReference &>(ahInfos.get());
		indexRef = &(dynamic_cast<const LADataMultiReference &>
			(infos.get(0).get().getData(PRICING_DATA_RANGEACCRUEINDEXINFOS, ISNOTNULL).get()));
	}
	else //single index case
	{
		indexRef = &(dynamic_cast<const LADataMultiReference &>
			(cashlet.getData(PRICING_DATA_RANGEACCRUEINDEXINFOS, ISNOTNULL).get()));
	}

	// first slidingrule & calendar (for check)
	const LAPriceDataSlidingRule *pSrule = 0;
	const LAPriceDataCalendar *pCal = 0;
	LAPriceCFGenUtility::getBusDayRuleAndCalendar( indexRef->get(0).get(), 
												PRICING_DATA_OBSERVATIONSLIDINGRULE,
												PRICING_DATA_OBSERVATIONCALENDAR,
												indexRef->get(0).get(),
												CALIBRATION_DATA_SLIDINGRULE,
												CALIBRATION_DATA_CALENDAR,
												pSrule, pCal);


	const DateVector *pObservationDates = 0;
	const LADataHolder &ahOBD = cashlet.getData(PRICING_DATA_OBSERVATIONDATES, NOCHECK);
	if (ahOBD.isDefined() && !ahOBD.isNull())
	{
		//pObservationDates = &(dynamic_cast<const LADataDates &>(ahOBD.get()).get());
		pObservationDates = new DateVector(dynamic_cast<const LADataDates &>(ahOBD.get()).get());
		setUpCalcDays(mRAObservationStart, mRAObservationEnd, *pObservationDates, mCalcDays);
	}
	else
	{
		const LADataHolder &ahBDB = cashlet.getData(PRICING_DATA_BUSINESSDAYSBASE, NOCHECK);
		if (ahBDB.isDefined() && !ahBDB.isNull())
		{
			const bool isBussinessDaysBase = dynamic_cast<const LADataBool &>(ahBDB.get()).get();
			if (isBussinessDaysBase)
			{
				if (!pSrule || !pCal)
				{
					throw LACoreInvalidData("SlidingRule or Calender is NULL", __FILE__, __LINE__);
				}

				DateVector *out = new DateVector;
				LAMathDateCalculations::generateSchedule(mRAObservationStart, mRAObservationEnd, BUSINESS_DAYS,	
											false,
											NULL, NULL,
											NULL,
											*out,
											pSrule,
											pCal
											);
				
				pObservationDates = out;
				setUpCalcDays(mRAObservationStart, mRAObservationEnd, *pObservationDates, mCalcDays);
			}
		}
	}
	if (ahInfos.isDefined() && !ahInfos.isNull()) //multi index case
	{
		// set range accrue implement
		const LADataMultiReference& infos = dynamic_cast<const LADataMultiReference &>(ahInfos.get());
		mpRAImpls.clear();
		mpRAImpls.resize(infos.getSize());
		for (unsigned int i = 0; i < infos.getSize(); ++i)
		{
			mpRAImpls[i] = new LAPricePayOffToolRangeAccrueImpl();
			mpRAImpls[i]->setUp(basedate, trade, payoff, infos.get(i).get(), 
				mRAObservationStart, mRAObservationEnd, mPaymentDate, pObservationDates);
		}

		// set range accrue index condition
		mIsAndCondition = false;
		const LADataHolder &ahIAC = cashlet.getData(PRICING_DATA_ISANDCONDITION, NOCHECK);
		if (ahIAC.isDefined() && !ahIAC.isNull())
		{
			mIsAndCondition = dynamic_cast<const LADataBool &>(ahIAC.get()).get();
		}
		mIsExcludeAndCondition = false;
		const LADataHolder &ahIEA = cashlet.getData(PRICING_DATA_ISEXCLUDEANDCONDITION, NOCHECK);
		if (ahIEA.isDefined() && !ahIEA.isNull())
		{
			mIsExcludeAndCondition = dynamic_cast<const LADataBool &>(ahIEA.get()).get();
		}

	}
	else //single index case
	{
		mpRAImpls.clear();
		mpRAImpls.resize(1);
		mpRAImpls[0] = new LAPricePayOffToolRangeAccrueImpl();
		mpRAImpls[0]->setUp(basedate, trade, payoff, cashlet, 
				mRAObservationStart, mRAObservationEnd, mPaymentDate, pObservationDates);
		mIsAndCondition = false;
		mIsExcludeAndCondition = false;
	}
	mIsNotCondition = false;
	const LADataHolder &ahINC = cashlet.getData(PRICING_DATA_ISNOTCONDITION, NOCHECK);
	if (ahINC.isDefined() && !ahINC.isNull())
	{
		mIsNotCondition = dynamic_cast<const LADataBool &>(ahINC.get()).get();
	}
	if (pObservationDates)
	{
		delete pObservationDates;
	}

	//hit rate operator
	if (mpHitRateOperator) delete mpHitRateOperator;
	mpHitRateOperator = NULL;
	const LADataHolder* dh = &(cashlet.getData(PRICING_DATA_HITRATEOPERATOR, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		const LAFunctionBase& htfunc = dynamic_cast<const LAPriceDataFunction&>(dh->get()).getFunction();
		mpHitRateOperator = dynamic_cast<LAFunctionBase*>(htfunc.clone());
		//hit rate coefficient
		const LADataDoubles* htcoeffs = &dynamic_cast<const LADataDoubles &>(cashlet.getData(PRICING_DATA_HITRATECOEFFICIENT, ISNOTNULL).get());
		mpHitRateOperator->setParam(htcoeffs->get());
	}
	//hit rate cap
	mIsHitRateCap = false;
	dh = &(cashlet.getData(PRICING_DATA_HITRATEMAX, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		mIsHitRateCap = true;
	    mHitRateCap = dynamic_cast<const LADataDouble&>(dh->get()).get();
	}
	//hit rate floor
	mIsHitRateFloor = false;
	dh = &(cashlet.getData(PRICING_DATA_HITRATEMIN, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		mIsHitRateFloor = true;
	    mHitRateFloor = dynamic_cast<const LADataDouble&>(dh->get()).get();
	}
	//final coupon operator
	if (mpOperator) delete mpOperator;
	mpOperator = NULL;
	dh = &(cashlet.getData(PRICING_DATA_OPERATOR, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		const LAFunctionBase& method = dynamic_cast<const LAPriceDataFunction&>(dh->get()).getFunction();
		mpOperator = dynamic_cast<LAFunctionBase*>(method.clone());
		//coefficient
		const LADataDoubles* coeffs = &dynamic_cast<const LADataDoubles &>(cashlet.getData(PRICING_DATA_COEFFICIENT, ISNOTNULL).get());
		mpOperator->setParam(coeffs->get());
	}
	//range accrue strike
	mIsRAStrike = false;
	mRAStrike = 1.0;
	dh = &(cashlet.getData(PRICING_DATA_RANGEACCRUESTRIKE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		mIsRAStrike = true;
		mRAStrike = dynamic_cast<const LADataDouble&>(dh->get()).get();
	}
}

/*!
    @brief set up calc days
	@param[in] start
	@param[in] end
	@param[in] observationDates
	@param[out] calcDays
*/
void
LAPricePayOffToolRangeAccrue::setUpCalcDays(const LADate &start, const LADate &end, const DateVector &observationDates, unsigned int &calcDays)
{
	unsigned int excludenum = 0;
	DateVector::const_iterator it = observationDates.begin();
	while (it != observationDates.end())
	{
		if (*it < start || end <= *it) ++excludenum;
		++it;
	}
	calcDays = observationDates.size() - excludenum;
}

//////////////////////
// internal class !//
//////////////////////

/*!
    @brief default constructor
*/
LAPricePayOffToolRangeAccrue::LAPricePayOffToolRangeAccrueImpl::LAPricePayOffToolRangeAccrueImpl()
: mpRAFunc(0), mRAMax(0.0), mRAMin(0.0), mpRAMaxBFunc(0), mpRAMinBFunc(0)
{

}

/*!
    @brief destructor
*/
LAPricePayOffToolRangeAccrue::LAPricePayOffToolRangeAccrueImpl::~LAPricePayOffToolRangeAccrueImpl()
{
	for (unsigned int i = 0; i < mpRAIndexs.size(); ++i)
	{
		delete mpRAIndexs[i];
	}
	mRABusDayMap.clear();
	delete mpRAFunc;

	for (unsigned int i = 0; i < mpRABIndexs.size(); ++i)
	{
		delete mpRABIndexs[i];
	}
	delete mpRAMaxBFunc;
	delete mpRAMinBFunc;
}

/*!
    @brief copy constructor
	@param[in] v LAPricePayOffToolRangeAccrueImpl object
*/
LAPricePayOffToolRangeAccrue::LAPricePayOffToolRangeAccrueImpl::LAPricePayOffToolRangeAccrueImpl(const LAPricePayOffToolRangeAccrueImpl& v)
: mRAMax(v.mRAMax), mRAMin(v.mRAMin)
{
	mpRAIndexs.resize(v.mpRAIndexs.size());
	mRABusDayMap.resize(v.mpRAIndexs.size());
	for (unsigned int i = 0; i < v.mpRAIndexs.size(); ++i)
	{
		mpRAIndexs[i] = dynamic_cast<LAPriceIndexTool *>(v.mpRAIndexs[i]->clone());
		mRABusDayMap[i] = v.mRABusDayMap[i];
	}
	mpRAFunc = dynamic_cast<LAFunctionBase *>(v.mpRAFunc->clone());

	mpRABIndexs.resize(v.mpRABIndexs.size());
	for (unsigned int i = 0; i < v.mpRABIndexs.size(); ++i)
	{
		mpRABIndexs[i] = dynamic_cast<LAPriceIndexTool *>(v.mpRABIndexs[i]->clone());
	}
	mpRAMaxBFunc = dynamic_cast<LAFunctionBase *>(v.mpRAMaxBFunc->clone());
	mpRAMinBFunc = dynamic_cast<LAFunctionBase *>(v.mpRAMinBFunc->clone());
}

/*
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LAPricePayOffToolRangeAccrue::LAPricePayOffToolRangeAccrueImpl*
LAPricePayOffToolRangeAccrue::LAPricePayOffToolRangeAccrueImpl::clone() const
{
	return new LAPricePayOffToolRangeAccrue::LAPricePayOffToolRangeAccrueImpl(*this);
}

/*!
    @brief set up this class
	@param[in] basedate basedate
	@param[in] trade trade object
	@param[in] payoff LAPricePayOff object
	@param[in] info range accrue info object
	@param[in] start start date
	@param[in] end end date
	@param[in] payment payment date
	@param[in] sameobservationindex days before same observation date
*/
void
LAPricePayOffToolRangeAccrue::LAPricePayOffToolRangeAccrueImpl::setUp(const LADate& basedate, const LAObject& trade, 
															const LAPricePayOff& payoff, const LAObject& info, 
															const LADate& start, const LADate& end, const LADate& payment,
															const DateVector* pobservationDates)
{
	// range accrue same observation days
	int sameObservationDays = 0;
	const LADataHolder &ahSD = info.getData(PRICING_DATA_RANGEACCRUESAMEOBSERVATIONDAYS, NOCHECK);
	if (ahSD.isDefined() && !ahSD.isNull())
	{
		sameObservationDays = dynamic_cast<const LADataInt &>(ahSD.get()).get();
	}

	// index
	const LADataMultiReference &indexRef = dynamic_cast<const LADataMultiReference &>(info.getData(PRICING_DATA_RANGEACCRUEINDEXINFOS, ISNOTNULL).get());
	unsigned int indexSize = indexRef.getSize();
	mpRAIndexs.clear();
	mRABusDayMap.clear();
	mpRAIndexs.resize(indexSize);
	mRABusDayMap.resize(indexSize);
	for (unsigned int i = 0; i < indexSize; ++i)
	{
		LAString type = dynamic_cast<LADataString &>(indexRef.get(i).getData(PRICING_DATA_INDEXTYPE, ISNOTNULL).get()).get();
		type.toUpper();
		if (type == FIXEDRATE || type == CPN || type == CPNCF)
		{
			throw LACoreInvalidData("Index type FIXEDRATE or CPN or CPNCF is not support in Range Accrue", __FILE__, __LINE__);
		}
		// check
		//slidingrule & calendar
		const LAPriceDataSlidingRule *pSrule = 0;
		const LAPriceDataCalendar *pCal = 0;
		LAPriceCFGenUtility::getBusDayRuleAndCalendar( indexRef.get(i).get(), 
													PRICING_DATA_OBSERVATIONSLIDINGRULE,
													PRICING_DATA_OBSERVATIONCALENDAR,
													indexRef.get(i).get(),
													CALIBRATION_DATA_SLIDINGRULE,
													CALIBRATION_DATA_CALENDAR,
													pSrule, pCal);
		//frequency
		LAString freq = dynamic_cast<const LADataString &>(indexRef.get(i).getData(PRICING_DATA_OBSERVATIONFREQUENCY, ISNOTNULL).get());
		freq.toUpper();
		if (freq != BUSINESS_DAYS && freq != DAILY)
		{
			throw LACoreInvalidData("Frequency error. BUSINESS_DAYS is only supperted in Range Accrue.", __FILE__, __LINE__);
		}

		//set index
		mpRAIndexs[i] =  new LAPriceIndexTool();
		mpRAIndexs[i]->setUp(basedate, payment, trade, indexRef.get(i).get(), payoff);

		DateVector out;
		const LADataHolder &ahObStart =  indexRef.get(i).getData(PRICING_DATA_OBSERVATIONSTARTDATE, NOCHECK);
		if (ahObStart.isDefined() && !ahObStart.isNull())
		{
			const LADate &obstart =  dynamic_cast<const LADataDate &>(ahObStart.get()).get();
			const LADate &obend =  dynamic_cast<const LADataDate &>(indexRef.get(i).getData(PRICING_DATA_OBSERVATIONENDDATE, ISNOTNULL).get());
			LAMathDateCalculations::generateSchedule(obstart, obend, BUSINESS_DAYS,	
										false,
										NULL, NULL,
										NULL,
										out,
										pSrule,
										pCal
										);

		}
		else
		{
			LAMathDateCalculations::generateSchedule(start, end, BUSINESS_DAYS,	
										false,
										NULL, NULL,
										NULL,
										out,
										pSrule,
										pCal
										);
		}

		//index calculation end date
		LADate determinationBusinessDate(end);
		if (ahSD.isDefined() && !ahSD.isNull())
		{
			if(pCal == NULL || pSrule == NULL)
			{
				LAString msg;
				msg += "Calendar and SlidingRule dataValues are necessary ";
				msg += "when RangeAccrueSameObservationDays is set in range bounday info.";
				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
			LAString sameTerm = LAString(sameObservationDays) + "D";
			determinationBusinessDate = LAMathDateCalculations::getDate(end, sameTerm, *pSrule, pCal, false); 
		}

		if (pobservationDates == 0)
		{
			unsigned int count = 0;
			unsigned int pos = 0;
			LADate date = start;
			while (date < end)
			{
				LADate tmpDate;
				if (pSrule->getSlidingRule() != SLIDING_RULE_NO_CHANGE)
				{
					tmpDate = pSrule->getDate(date, pCal->getCalendar());
				}
				else
				{
					tmpDate = date;
				}

				if (!LAAlgorithm::find<DateVector, LADate>(out, tmpDate, 0, out.size() - 1, pos))
				{
					LAString msg = "Date vector is inconsistent. Error date = " + tmpDate.stringWithFormat();
					throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
				}
				mRABusDayMap[i].insert(make_pair(count, pos));
				if (tmpDate >= determinationBusinessDate)
				{
					//set next iterator.
					date.addDays(1);
					++count;

					//execute rest of while loop without updating the variable 'pos.'
					while (date < end)
					{
						mRABusDayMap[i].insert(make_pair(count, pos));
						date.addDays(1);
						++count;
					}
					break;
				}

				date.addDays(1);
				++count;
			}
		}
		else
		{
			unsigned int pos = 0;
			DateVector::const_iterator itStart = pobservationDates->begin();
			while (itStart != pobservationDates->end() && *itStart < start)
			{
				++itStart;
			}
			DateVector::const_iterator it = itStart;
			while (it != pobservationDates->end() && *it < end)
			{
				if (!LAAlgorithm::find<DateVector, LADate>(out, *it, 0, out.size() - 1, pos))
				{
					LAString msg = "Observation dates of cashlet are inconsistent with those of index. Error date = " + it->stringWithFormat();
					throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
				}
				mRABusDayMap[i].insert(make_pair(distance(itStart, it), pos));
				if (*it >= determinationBusinessDate)
				{
					//set next iterator.
					++it;

					//execute rest of while loop without updating the variable 'pos.'
					while  (it != pobservationDates->end() && *it < end)
					{
						mRABusDayMap[i].insert(make_pair(distance(itStart, it), pos));
						++it;
					}
					break;
				}
				++it;
			}
		}
	}

	// range accrue method
	LACoreFunctionBase *tmpFunc = dynamic_cast<const LAPriceDataFunction&>(info.getData(PRICING_DATA_RANGEACCRUEOPERATOR, ISNOTNULL).get()).getFunction().clone();
	mpRAFunc = dynamic_cast<LAFunctionBase *>(tmpFunc);
	// range accrue method coeff
	const DoubleArray& coeff = dynamic_cast<const LADataDoubles&>(info.getData(PRICING_DATA_RANGEACCRUECOEFFICIENT, ISNOTNULL).get()).get();
	mpRAFunc->setParam(coeff);
	// range accrue max
	const LADataHolder &ahMAX = info.getData(PRICING_DATA_RANGEACCRUEMAX, NOCHECK);
	if (ahMAX.isDefined() && !ahMAX.isNull())
	{
		mRAMax = dynamic_cast<const LADataDouble&>(ahMAX.get()).get();
	}	
	// range accrue min
	const LADataHolder &ahMIN = info.getData(PRICING_DATA_RANGEACCRUEMIN, NOCHECK);
	if (ahMIN.isDefined() && !ahMIN.isNull())
	{
		mRAMin = dynamic_cast<const LADataDouble&>(ahMIN.get()).get();
	}
	
	// set up boundary index
	const LADataHolder &ahBII = info.getData(PRICING_DATA_RANGEACCRUEBOUNDARYINDEXINFOS, NOCHECK);
	if (ahBII.isDefined() && !ahBII.isNull())
	{
		const LADataMultiReference& indexinfos = dynamic_cast<const LADataMultiReference&>(ahBII.get());
		mpRABIndexs.resize(indexinfos.getSize(), NULL);
		for (unsigned int i = 0; i < indexinfos.getSize(); i++)
		{
			// index type
			const LADataHolder &ahIT = indexinfos.get(i).getData(PRICING_DATA_INDEXTYPE, ISNOTNULL);
			LAString indextype = dynamic_cast<const LADataString&>(ahIT.get());
			indextype.toUpper();

			if(indextype == FIXEDRATE)
				mpRABIndexs[i] = new LAPriceIndexToolFixed();
			else if(indextype == CPN)
				mpRABIndexs[i] = new LAPriceIndexToolCpn();
			else if(indextype == CPNCF)
				mpRABIndexs[i] = new LAPriceIndexToolCpnCF();
			else
			{
				//fixing date
				const LADataHolder &ahFD = indexinfos.get(i).getData(PRICING_DATA_FIXINGDATE, ISNOTNULL);	 
				const LADate& fixingdate = dynamic_cast<const LADataDate&>(ahFD.get()).get();
				if (basedate > fixingdate)
					mpRABIndexs[i] = new LAPriceIndexToolFixed();
				else if (basedate == fixingdate)
				{
					const LADataHolder &ahFR = indexinfos.get(i).getData(PRICING_DATA_FIXEDRATE, NOCHECK);	 
					if (ahFR.isDefined() && !ahFR.isNull())
					{
						mpRABIndexs[i] = new LAPriceIndexToolFixed();
					}
					else
					{
						mpRABIndexs[i] = new LAPriceIndexTool();
					}
				}
				else
					mpRABIndexs[i] = new LAPriceIndexTool();		
			}		
		
			mpRABIndexs[i]->setUp(basedate, payment, trade, indexinfos.get(i).get(), payoff);
		}

		//max operator
		const LADataHolder &ahMaxBO = info.getData(PRICING_DATA_RANGEACCRUEMAXBOUNDARYOPERATOR, ISNOTNULL);
		const LAFunctionBase& maxFunc = dynamic_cast<const LAPriceDataFunction&>(ahMaxBO.get()).getFunction();
		mpRAMaxBFunc = dynamic_cast<LAFunctionBase*>(maxFunc.clone());
		
		//max coefficient
		const LADataHolder &ahMaxBC = info.getData(PRICING_DATA_RANGEACCRUEMAXBOUNDARYCOEFFICIENT, ISNOTNULL);
		const DoubleArray& maxCoeff = dynamic_cast<const LADataDoubles&>(ahMaxBC.get()).get();
		mpRAMaxBFunc->setParam(maxCoeff);

		//max operator
		const LADataHolder &ahMinBO = info.getData(PRICING_DATA_RANGEACCRUEMINBOUNDARYOPERATOR, ISNOTNULL);
		const LAFunctionBase& minFunc = dynamic_cast<const LAPriceDataFunction&>(ahMinBO.get()).getFunction();
		mpRAMinBFunc = dynamic_cast<LAFunctionBase*>(minFunc.clone());
		
		//min coefficient
		const LADataHolder &ahMinBC = info.getData(PRICING_DATA_RANGEACCRUEMINBOUNDARYCOEFFICIENT, ISNOTNULL);
		const DoubleArray& minCoeff = dynamic_cast<const LADataDoubles&>(ahMinBC.get()).get();
		mpRAMinBFunc->setParam(minCoeff);
	}
}

/*!
	@brief calc range accrue index & boundary index

*/
void
LAPricePayOffToolRangeAccrue::LAPricePayOffToolRangeAccrueImpl::calcIndex(void)
{
	// calc range accrue index
	for (unsigned int i = 0; i < mpRAIndexs.size(); ++i)
	{
		mpRAIndexs[i]->calcIndex();
	}
	// calc range accrue boundary index
	DoubleArray boundaryIndexVec(mpRABIndexs.size());
	if(mpRABIndexs.size() != 0 && mpRABIndexs[0] != NULL)
	{
			for (unsigned int i = 0; i < mpRABIndexs.size(); ++i)
			{
				boundaryIndexVec[i] = mpRABIndexs[i]->calcIndex();
			}
			mRAMax = mpRAMaxBFunc->operator ()(boundaryIndexVec);
			mRAMin = mpRAMinBFunc->operator ()(boundaryIndexVec);
	}
}

/*!
    @brief judge range accrue index is in range or not

    @param[in] index index of range accrue index
	@return is in range or not
*/
bool
LAPricePayOffToolRangeAccrue::LAPricePayOffToolRangeAccrueImpl::isHit(unsigned int index)
{
	DoubleArray indexVec(mpRAIndexs.size());
	for (unsigned int j = 0; j < mpRAIndexs.size(); ++j)
	{
		map<unsigned int, unsigned int>::const_iterator it = mRABusDayMap[j].find(index);
		if (it == mRABusDayMap[j].end())
		{
			throw LACoreInvalidData("BusinessDay Map is invalid", __FILE__, __LINE__);
		}
		indexVec[j] = mpRAIndexs[j]->getIndexResult()[it->second];
	}
	const double val = mpRAFunc->operator ()(indexVec);
	if (val >= mRAMin && val <= mRAMax)
	{
		return true;
	}
	return false;
}