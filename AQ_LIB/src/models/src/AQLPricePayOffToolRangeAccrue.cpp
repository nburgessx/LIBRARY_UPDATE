/*! @file
    @brief source code of tool class of payoff calculation in range accrue.
*/


#ifdef __GNUG__
#pragma implementation
#endif
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include "AQLPricePayOffToolRangeAccrue.h"

#include "AQLDataHolder.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataReference.h"
#include "AQLDataMultiReference.h"
#include "AQLObject.h"
#include "AQLObjectHolder.h"

#include "AQLPriceDataCalendar.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLPriceDataFunction.h"
#include "AQLMathIndexEntity.h"
#include "AQLMathPathEntity.h"
#include "AQLMathFXEntity.h"
#include "AQLMathDefine.h"
#include "AQLMathDateCalculations.h"

#include "AQLAlgorithm.h"
#include "AQLBasic.h"

#include "AQLPricePayOff.h"
#include "AQLPriceCouponTool.h"
#include "AQLPriceCFGenUtility.h"
#include "AQLPriceCouponBase.h"

#define BUSINESS_DAYS "BUSINESS_DAYS"
#define DAILY "DAILY"

#define FIXEDRATE	"FIXEDRATE"
#define CPN			"CPN"
#define CPNCF		"CPNCF"

using namespace std;

/*!
    @brief default constructor
*/
AQLPricePayOffToolRangeAccrue::AQLPricePayOffToolRangeAccrue()
: AQLPricePayOffTool(), mCalcDays(0), mIsAndCondition(false), mIsExcludeAndCondition(false), mIsNotCondition(false),
  mpHitRateOperator(0),  mIsHitRateCap(false), mIsHitRateFloor(false), mHitRateCap(1.0), mHitRateFloor(0.0), mpOperator(0),
  mIsRAStrike(false), mRAStrike(1.0)
{

}
/*!
    @brief destructor
*/
AQLPricePayOffToolRangeAccrue::~AQLPricePayOffToolRangeAccrue()
{
	for (unsigned int i = 0; i < mpRAImpls.size(); ++i)
	{
		delete mpRAImpls[i];
	}
}

/*!
    @brief copy constructor
	@param[in] v AQLPricePayOffToolRangeAccrue object
*/
AQLPricePayOffToolRangeAccrue::AQLPricePayOffToolRangeAccrue(const AQLPricePayOffToolRangeAccrue& v)
: AQLPricePayOffTool(v), mCalcDays(v.mCalcDays), 
  mRAObservationStart(v.mRAObservationStart), mRAObservationEnd(v.mRAObservationEnd), mIsAndCondition(v.mIsAndCondition), mIsExcludeAndCondition(v.mIsExcludeAndCondition), mIsNotCondition(v.mIsNotCondition),
  mpHitRateOperator(v.mpHitRateOperator), mIsHitRateCap(v.mIsHitRateCap), mIsHitRateFloor(v.mIsHitRateFloor), mHitRateCap(v.mHitRateCap), mHitRateFloor(v.mHitRateFloor), mpOperator(v.mpOperator),
  mIsRAStrike(v.mIsRAStrike), mRAStrike(v.mRAStrike)
{
	for (unsigned int i = 0; i < v.mpRAImpls.size(); ++i)
	{
		mpRAImpls[i] = dynamic_cast<AQLPricePayOffToolRangeAccrueImpl *>(v.mpRAImpls[i]->clone());
	}
}


/*!
    @brief calculate payoff
*/
double
AQLPricePayOffToolRangeAccrue::calcPayOff(void) const
{
	AQLPricePayOffTool::calcPayOff();
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
	@param[in] payoff AQLPricePayOff object
	@param[in] current position of payoff(first payoff position = 0)
*/
void
AQLPricePayOffToolRangeAccrue::setUp(const AQLDate& basedate, const AQLObject& trade,
						unsigned int legNo, const AQLObject& cashlet, 
						const AQLPricePayOff& payoff,
						unsigned int currentpos)
{
	AQLPricePayOffTool::setUp(basedate, trade, legNo, cashlet, payoff, currentpos);

	// days in calculation period
	const AQLDataHolder &ahOSD = cashlet.getData(PRICING_DATA_RANGEACCRUEOBSERVATIONSTARTDATE, NOCHECK);
	if (ahOSD.isDefined() && !ahOSD.isNull())
	{
		mRAObservationStart = dynamic_cast<const AQLDataDate &>(ahOSD.get()).get();
	}
	else
	{
		mRAObservationStart = mStart;
	}

	const AQLDataHolder &ahOED = cashlet.getData(PRICING_DATA_RANGEACCRUEOBSERVATIONENDDATE, NOCHECK);
	if (ahOED.isDefined() && !ahOED.isNull())
	{
		mRAObservationEnd = dynamic_cast<const AQLDataDate &>(ahOED.get()).get();
	}
	else
	{
		mRAObservationEnd = mEnd;
	}
	mCalcDays = mRAObservationStart.intervalDays(mRAObservationEnd);
	// index
	const AQLDataMultiReference* indexRef;
	const AQLDataHolder &ahInfos = cashlet.getData(PRICING_DATA_RANGEACCRUEINFOS, NOCHECK);
	if (ahInfos.isDefined() && !ahInfos.isNull()) //multi index case
	{
		const AQLDataMultiReference& infos = dynamic_cast<const AQLDataMultiReference &>(ahInfos.get());
		indexRef = &(dynamic_cast<const AQLDataMultiReference &>
			(infos.get(0).get().getData(PRICING_DATA_RANGEACCRUEINDEXINFOS, ISNOTNULL).get()));
	}
	else //single index case
	{
		indexRef = &(dynamic_cast<const AQLDataMultiReference &>
			(cashlet.getData(PRICING_DATA_RANGEACCRUEINDEXINFOS, ISNOTNULL).get()));
	}

	// first slidingrule & calendar (for check)
	const AQLPriceDataSlidingRule *pSrule = 0;
	const AQLPriceDataCalendar *pCal = 0;
	AQLPriceCFGenUtility::getBusDayRuleAndCalendar( indexRef->get(0).get(), 
												PRICING_DATA_OBSERVATIONSLIDINGRULE,
												PRICING_DATA_OBSERVATIONCALENDAR,
												indexRef->get(0).get(),
												CALIBRATION_DATA_SLIDINGRULE,
												CALIBRATION_DATA_CALENDAR,
												pSrule, pCal);


	const DateVector *pObservationDates = 0;
	const AQLDataHolder &ahOBD = cashlet.getData(PRICING_DATA_OBSERVATIONDATES, NOCHECK);
	if (ahOBD.isDefined() && !ahOBD.isNull())
	{
		//pObservationDates = &(dynamic_cast<const AQLDataDates &>(ahOBD.get()).get());
		pObservationDates = new DateVector(dynamic_cast<const AQLDataDates &>(ahOBD.get()).get());
		setUpCalcDays(mRAObservationStart, mRAObservationEnd, *pObservationDates, mCalcDays);
	}
	else
	{
		const AQLDataHolder &ahBDB = cashlet.getData(PRICING_DATA_BUSINESSDAYSBASE, NOCHECK);
		if (ahBDB.isDefined() && !ahBDB.isNull())
		{
			const bool isBussinessDaysBase = dynamic_cast<const AQLDataBool &>(ahBDB.get()).get();
			if (isBussinessDaysBase)
			{
				if (!pSrule || !pCal)
				{
					throw AQLCoreInvalidData("SlidingRule or Calender is NULL", __FILE__, __LINE__);
				}

				DateVector *out = new DateVector;
				AQLMathDateCalculations::generateSchedule(mRAObservationStart, mRAObservationEnd, BUSINESS_DAYS,	
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
		const AQLDataMultiReference& infos = dynamic_cast<const AQLDataMultiReference &>(ahInfos.get());
		mpRAImpls.clear();
		mpRAImpls.resize(infos.getSize());
		for (unsigned int i = 0; i < infos.getSize(); ++i)
		{
			mpRAImpls[i] = new AQLPricePayOffToolRangeAccrueImpl();
			mpRAImpls[i]->setUp(basedate, trade, payoff, infos.get(i).get(), 
				mRAObservationStart, mRAObservationEnd, mPaymentDate, pObservationDates);
		}

		// set range accrue index condition
		mIsAndCondition = false;
		const AQLDataHolder &ahIAC = cashlet.getData(PRICING_DATA_ISANDCONDITION, NOCHECK);
		if (ahIAC.isDefined() && !ahIAC.isNull())
		{
			mIsAndCondition = dynamic_cast<const AQLDataBool &>(ahIAC.get()).get();
		}
		mIsExcludeAndCondition = false;
		const AQLDataHolder &ahIEA = cashlet.getData(PRICING_DATA_ISEXCLUDEANDCONDITION, NOCHECK);
		if (ahIEA.isDefined() && !ahIEA.isNull())
		{
			mIsExcludeAndCondition = dynamic_cast<const AQLDataBool &>(ahIEA.get()).get();
		}

	}
	else //single index case
	{
		mpRAImpls.clear();
		mpRAImpls.resize(1);
		mpRAImpls[0] = new AQLPricePayOffToolRangeAccrueImpl();
		mpRAImpls[0]->setUp(basedate, trade, payoff, cashlet, 
				mRAObservationStart, mRAObservationEnd, mPaymentDate, pObservationDates);
		mIsAndCondition = false;
		mIsExcludeAndCondition = false;
	}
	mIsNotCondition = false;
	const AQLDataHolder &ahINC = cashlet.getData(PRICING_DATA_ISNOTCONDITION, NOCHECK);
	if (ahINC.isDefined() && !ahINC.isNull())
	{
		mIsNotCondition = dynamic_cast<const AQLDataBool &>(ahINC.get()).get();
	}
	if (pObservationDates)
	{
		delete pObservationDates;
	}

	//hit rate operator
	if (mpHitRateOperator) delete mpHitRateOperator;
	mpHitRateOperator = NULL;
	const AQLDataHolder* dh = &(cashlet.getData(PRICING_DATA_HITRATEOPERATOR, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		const AQLFunctionBase& htfunc = dynamic_cast<const AQLPriceDataFunction&>(dh->get()).getFunction();
		mpHitRateOperator = dynamic_cast<AQLFunctionBase*>(htfunc.clone());
		//hit rate coefficient
		const AQLDataDoubles* htcoeffs = &dynamic_cast<const AQLDataDoubles &>(cashlet.getData(PRICING_DATA_HITRATECOEFFICIENT, ISNOTNULL).get());
		mpHitRateOperator->setParam(htcoeffs->get());
	}
	//hit rate cap
	mIsHitRateCap = false;
	dh = &(cashlet.getData(PRICING_DATA_HITRATEMAX, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		mIsHitRateCap = true;
	    mHitRateCap = dynamic_cast<const AQLDataDouble&>(dh->get()).get();
	}
	//hit rate floor
	mIsHitRateFloor = false;
	dh = &(cashlet.getData(PRICING_DATA_HITRATEMIN, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		mIsHitRateFloor = true;
	    mHitRateFloor = dynamic_cast<const AQLDataDouble&>(dh->get()).get();
	}
	//final coupon operator
	if (mpOperator) delete mpOperator;
	mpOperator = NULL;
	dh = &(cashlet.getData(PRICING_DATA_OPERATOR, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		const AQLFunctionBase& method = dynamic_cast<const AQLPriceDataFunction&>(dh->get()).getFunction();
		mpOperator = dynamic_cast<AQLFunctionBase*>(method.clone());
		//coefficient
		const AQLDataDoubles* coeffs = &dynamic_cast<const AQLDataDoubles &>(cashlet.getData(PRICING_DATA_COEFFICIENT, ISNOTNULL).get());
		mpOperator->setParam(coeffs->get());
	}
	//range accrue strike
	mIsRAStrike = false;
	mRAStrike = 1.0;
	dh = &(cashlet.getData(PRICING_DATA_RANGEACCRUESTRIKE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		mIsRAStrike = true;
		mRAStrike = dynamic_cast<const AQLDataDouble&>(dh->get()).get();
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
AQLPricePayOffToolRangeAccrue::setUpCalcDays(const AQLDate &start, const AQLDate &end, const DateVector &observationDates, unsigned int &calcDays)
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
AQLPricePayOffToolRangeAccrue::AQLPricePayOffToolRangeAccrueImpl::AQLPricePayOffToolRangeAccrueImpl()
: mpRAFunc(0), mRAMax(0.0), mRAMin(0.0), mpRAMaxBFunc(0), mpRAMinBFunc(0)
{

}

/*!
    @brief destructor
*/
AQLPricePayOffToolRangeAccrue::AQLPricePayOffToolRangeAccrueImpl::~AQLPricePayOffToolRangeAccrueImpl()
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
	@param[in] v AQLPricePayOffToolRangeAccrueImpl object
*/
AQLPricePayOffToolRangeAccrue::AQLPricePayOffToolRangeAccrueImpl::AQLPricePayOffToolRangeAccrueImpl(const AQLPricePayOffToolRangeAccrueImpl& v)
: mRAMax(v.mRAMax), mRAMin(v.mRAMin)
{
	mpRAIndexs.resize(v.mpRAIndexs.size());
	mRABusDayMap.resize(v.mpRAIndexs.size());
	for (unsigned int i = 0; i < v.mpRAIndexs.size(); ++i)
	{
		mpRAIndexs[i] = dynamic_cast<AQLPriceIndexTool *>(v.mpRAIndexs[i]->clone());
		mRABusDayMap[i] = v.mRABusDayMap[i];
	}
	mpRAFunc = dynamic_cast<AQLFunctionBase *>(v.mpRAFunc->clone());

	mpRABIndexs.resize(v.mpRABIndexs.size());
	for (unsigned int i = 0; i < v.mpRABIndexs.size(); ++i)
	{
		mpRABIndexs[i] = dynamic_cast<AQLPriceIndexTool *>(v.mpRABIndexs[i]->clone());
	}
	mpRAMaxBFunc = dynamic_cast<AQLFunctionBase *>(v.mpRAMaxBFunc->clone());
	mpRAMinBFunc = dynamic_cast<AQLFunctionBase *>(v.mpRAMinBFunc->clone());
}

/*
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLPricePayOffToolRangeAccrue::AQLPricePayOffToolRangeAccrueImpl*
AQLPricePayOffToolRangeAccrue::AQLPricePayOffToolRangeAccrueImpl::clone() const
{
	return new AQLPricePayOffToolRangeAccrue::AQLPricePayOffToolRangeAccrueImpl(*this);
}

/*!
    @brief set up this class
	@param[in] basedate basedate
	@param[in] trade trade object
	@param[in] payoff AQLPricePayOff object
	@param[in] info range accrue info object
	@param[in] start start date
	@param[in] end end date
	@param[in] payment payment date
	@param[in] sameobservationindex days before same observation date
*/
void
AQLPricePayOffToolRangeAccrue::AQLPricePayOffToolRangeAccrueImpl::setUp(const AQLDate& basedate, const AQLObject& trade, 
															const AQLPricePayOff& payoff, const AQLObject& info, 
															const AQLDate& start, const AQLDate& end, const AQLDate& payment,
															const DateVector* pobservationDates)
{
	// range accrue same observation days
	int sameObservationDays = 0;
	const AQLDataHolder &ahSD = info.getData(PRICING_DATA_RANGEACCRUESAMEOBSERVATIONDAYS, NOCHECK);
	if (ahSD.isDefined() && !ahSD.isNull())
	{
		sameObservationDays = dynamic_cast<const AQLDataInt &>(ahSD.get()).get();
	}

	// index
	const AQLDataMultiReference &indexRef = dynamic_cast<const AQLDataMultiReference &>(info.getData(PRICING_DATA_RANGEACCRUEINDEXINFOS, ISNOTNULL).get());
	unsigned int indexSize = indexRef.getSize();
	mpRAIndexs.clear();
	mRABusDayMap.clear();
	mpRAIndexs.resize(indexSize);
	mRABusDayMap.resize(indexSize);
	for (unsigned int i = 0; i < indexSize; ++i)
	{
		AQLString type = dynamic_cast<AQLDataString &>(indexRef.get(i).getData(PRICING_DATA_INDEXTYPE, ISNOTNULL).get()).get();
		type.toUpper();
		if (type == FIXEDRATE || type == CPN || type == CPNCF)
		{
			throw AQLCoreInvalidData("Index type FIXEDRATE or CPN or CPNCF is not support in Range Accrue", __FILE__, __LINE__);
		}
		// check
		//slidingrule & calendar
		const AQLPriceDataSlidingRule *pSrule = 0;
		const AQLPriceDataCalendar *pCal = 0;
		AQLPriceCFGenUtility::getBusDayRuleAndCalendar( indexRef.get(i).get(), 
													PRICING_DATA_OBSERVATIONSLIDINGRULE,
													PRICING_DATA_OBSERVATIONCALENDAR,
													indexRef.get(i).get(),
													CALIBRATION_DATA_SLIDINGRULE,
													CALIBRATION_DATA_CALENDAR,
													pSrule, pCal);
		//frequency
		AQLString freq = dynamic_cast<const AQLDataString &>(indexRef.get(i).getData(PRICING_DATA_OBSERVATIONFREQUENCY, ISNOTNULL).get());
		freq.toUpper();
		if (freq != BUSINESS_DAYS && freq != DAILY)
		{
			throw AQLCoreInvalidData("Frequency error. BUSINESS_DAYS is only supperted in Range Accrue.", __FILE__, __LINE__);
		}

		//set index
		mpRAIndexs[i] =  new AQLPriceIndexTool();
		mpRAIndexs[i]->setUp(basedate, payment, trade, indexRef.get(i).get(), payoff);

		DateVector out;
		const AQLDataHolder &ahObStart =  indexRef.get(i).getData(PRICING_DATA_OBSERVATIONSTARTDATE, NOCHECK);
		if (ahObStart.isDefined() && !ahObStart.isNull())
		{
			const AQLDate &obstart =  dynamic_cast<const AQLDataDate &>(ahObStart.get()).get();
			const AQLDate &obend =  dynamic_cast<const AQLDataDate &>(indexRef.get(i).getData(PRICING_DATA_OBSERVATIONENDDATE, ISNOTNULL).get());
			AQLMathDateCalculations::generateSchedule(obstart, obend, BUSINESS_DAYS,	
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
			AQLMathDateCalculations::generateSchedule(start, end, BUSINESS_DAYS,	
										false,
										NULL, NULL,
										NULL,
										out,
										pSrule,
										pCal
										);
		}

		//index calculation end date
		AQLDate determinationBusinessDate(end);
		if (ahSD.isDefined() && !ahSD.isNull())
		{
			if(pCal == NULL || pSrule == NULL)
			{
				AQLString msg;
				msg += "Calendar and SlidingRule dataValues are necessary ";
				msg += "when RangeAccrueSameObservationDays is set in range bounday info.";
				throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
			AQLString sameTerm = AQLString(sameObservationDays) + "D";
			determinationBusinessDate = AQLMathDateCalculations::getDate(end, sameTerm, *pSrule, pCal, false); 
		}

		if (pobservationDates == 0)
		{
			unsigned int count = 0;
			unsigned int pos = 0;
			AQLDate date = start;
			while (date < end)
			{
				AQLDate tmpDate;
				if (pSrule->getSlidingRule() != SLIDING_RULE_NO_CHANGE)
				{
					tmpDate = pSrule->getDate(date, pCal->getCalendar());
				}
				else
				{
					tmpDate = date;
				}

				if (!AQLAlgorithm::find<DateVector, AQLDate>(out, tmpDate, 0, out.size() - 1, pos))
				{
					AQLString msg = "Date vector is inconsistent. Error date = " + tmpDate.stringWithFormat();
					throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
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
				if (!AQLAlgorithm::find<DateVector, AQLDate>(out, *it, 0, out.size() - 1, pos))
				{
					AQLString msg = "Observation dates of cashlet are inconsistent with those of index. Error date = " + it->stringWithFormat();
					throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
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
	AQLCoreFunctionBase *tmpFunc = dynamic_cast<const AQLPriceDataFunction&>(info.getData(PRICING_DATA_RANGEACCRUEOPERATOR, ISNOTNULL).get()).getFunction().clone();
	mpRAFunc = dynamic_cast<AQLFunctionBase *>(tmpFunc);
	// range accrue method coeff
	const DoubleArray& coeff = dynamic_cast<const AQLDataDoubles&>(info.getData(PRICING_DATA_RANGEACCRUECOEFFICIENT, ISNOTNULL).get()).get();
	mpRAFunc->setParam(coeff);
	// range accrue max
	const AQLDataHolder &ahMAX = info.getData(PRICING_DATA_RANGEACCRUEMAX, NOCHECK);
	if (ahMAX.isDefined() && !ahMAX.isNull())
	{
		mRAMax = dynamic_cast<const AQLDataDouble&>(ahMAX.get()).get();
	}	
	// range accrue min
	const AQLDataHolder &ahMIN = info.getData(PRICING_DATA_RANGEACCRUEMIN, NOCHECK);
	if (ahMIN.isDefined() && !ahMIN.isNull())
	{
		mRAMin = dynamic_cast<const AQLDataDouble&>(ahMIN.get()).get();
	}
	
	// set up boundary index
	const AQLDataHolder &ahBII = info.getData(PRICING_DATA_RANGEACCRUEBOUNDARYINDEXINFOS, NOCHECK);
	if (ahBII.isDefined() && !ahBII.isNull())
	{
		const AQLDataMultiReference& indexinfos = dynamic_cast<const AQLDataMultiReference&>(ahBII.get());
		mpRABIndexs.resize(indexinfos.getSize(), NULL);
		for (unsigned int i = 0; i < indexinfos.getSize(); i++)
		{
			// index type
			const AQLDataHolder &ahIT = indexinfos.get(i).getData(PRICING_DATA_INDEXTYPE, ISNOTNULL);
			AQLString indextype = dynamic_cast<const AQLDataString&>(ahIT.get());
			indextype.toUpper();

			if(indextype == FIXEDRATE)
				mpRABIndexs[i] = new AQLPriceIndexToolFixed();
			else if(indextype == CPN)
				mpRABIndexs[i] = new AQLPriceIndexToolCpn();
			else if(indextype == CPNCF)
				mpRABIndexs[i] = new AQLPriceIndexToolCpnCF();
			else
			{
				//fixing date
				const AQLDataHolder &ahFD = indexinfos.get(i).getData(PRICING_DATA_FIXINGDATE, ISNOTNULL);	 
				const AQLDate& fixingdate = dynamic_cast<const AQLDataDate&>(ahFD.get()).get();
				if (basedate > fixingdate)
					mpRABIndexs[i] = new AQLPriceIndexToolFixed();
				else if (basedate == fixingdate)
				{
					const AQLDataHolder &ahFR = indexinfos.get(i).getData(PRICING_DATA_FIXEDRATE, NOCHECK);	 
					if (ahFR.isDefined() && !ahFR.isNull())
					{
						mpRABIndexs[i] = new AQLPriceIndexToolFixed();
					}
					else
					{
						mpRABIndexs[i] = new AQLPriceIndexTool();
					}
				}
				else
					mpRABIndexs[i] = new AQLPriceIndexTool();		
			}		
		
			mpRABIndexs[i]->setUp(basedate, payment, trade, indexinfos.get(i).get(), payoff);
		}

		//max operator
		const AQLDataHolder &ahMaxBO = info.getData(PRICING_DATA_RANGEACCRUEMAXBOUNDARYOPERATOR, ISNOTNULL);
		const AQLFunctionBase& maxFunc = dynamic_cast<const AQLPriceDataFunction&>(ahMaxBO.get()).getFunction();
		mpRAMaxBFunc = dynamic_cast<AQLFunctionBase*>(maxFunc.clone());
		
		//max coefficient
		const AQLDataHolder &ahMaxBC = info.getData(PRICING_DATA_RANGEACCRUEMAXBOUNDARYCOEFFICIENT, ISNOTNULL);
		const DoubleArray& maxCoeff = dynamic_cast<const AQLDataDoubles&>(ahMaxBC.get()).get();
		mpRAMaxBFunc->setParam(maxCoeff);

		//max operator
		const AQLDataHolder &ahMinBO = info.getData(PRICING_DATA_RANGEACCRUEMINBOUNDARYOPERATOR, ISNOTNULL);
		const AQLFunctionBase& minFunc = dynamic_cast<const AQLPriceDataFunction&>(ahMinBO.get()).getFunction();
		mpRAMinBFunc = dynamic_cast<AQLFunctionBase*>(minFunc.clone());
		
		//min coefficient
		const AQLDataHolder &ahMinBC = info.getData(PRICING_DATA_RANGEACCRUEMINBOUNDARYCOEFFICIENT, ISNOTNULL);
		const DoubleArray& minCoeff = dynamic_cast<const AQLDataDoubles&>(ahMinBC.get()).get();
		mpRAMinBFunc->setParam(minCoeff);
	}
}

/*!
	@brief calc range accrue index & boundary index

*/
void
AQLPricePayOffToolRangeAccrue::AQLPricePayOffToolRangeAccrueImpl::calcIndex(void)
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
AQLPricePayOffToolRangeAccrue::AQLPricePayOffToolRangeAccrueImpl::isHit(unsigned int index)
{
	DoubleArray indexVec(mpRAIndexs.size());
	for (unsigned int j = 0; j < mpRAIndexs.size(); ++j)
	{
		map<unsigned int, unsigned int>::const_iterator it = mRABusDayMap[j].find(index);
		if (it == mRABusDayMap[j].end())
		{
			throw AQLCoreInvalidData("BusinessDay Map is invalid", __FILE__, __LINE__);
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