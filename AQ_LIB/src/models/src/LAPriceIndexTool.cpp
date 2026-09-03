/*! @file
    @brief source code of tool class of index calculation.

*/
//  2006, Mizuho International London..
///
#ifdef __GNUG__
#pragma implementation
#endif
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif


#include <algorithm>
#include "LAPriceIndexTool.h"

#include "LADataHolder.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataReference.h"
#include "LADataMultiReference.h"
#include "LAObject.h"
#include "LAObjectHolder.h"
#include "LACoreUtility.h"

#include "LAAlgorithm.h"

#include "LAMathDateCalculations.h"
#include "LAPriceDataCalendar.h"
#include "LAPriceDataSlidingRule.h"
#include "LAPriceDataFunction.h"
#include "LAMathIndexEntity.h"
#include "LAMathDefine.h"

#include "LAConstant.h"

#include "LAPriceCFGenUtility.h"
#include "LAPricePayOff.h"

#include "LAMathPlainVanillaEntity.h"
#include "LAInterpolationBase.h"
#include "LAMathYieldCurve.h"
#include "LAMathYieldCurvePro.h"
#include "LAPriceDataInterpolation.h"

using namespace std;


#define ROUND_STR		"ROUND"
#define ROUND_UP_STR	"ROUNDUP"
#define ROUND_DOWN_STR	"ROUNDDOWN"



#define CPN		"CPN"
#define LEG		"LEG"
#define COUPON	"COUPON"

/*!
    @brief constructor
*/
LAPriceIndexToolBase::LAPriceIndexToolBase()
: mIsCap(false), mIsFloor(false), mDiscountRatio(1.0), mIsDFRatioUpdate(false), mIsRound(false), mRoundDigit(0)
{

}
/*!
    @brief destructor

*/
LAPriceIndexToolBase::~LAPriceIndexToolBase()
{

}
/*!
	@brief set up this class
	@param[in] basedate basedate
	@param[in] paydate paymentdate
	@param[in] trade trade object
	@param[in] indexinfo index information object
	@param[in] payoff LAPricePayOff object
*/
void
LAPriceIndexToolBase::setUp(const LADate& basedate,
							const LADate& paydate,
							const LAObject& trade,
							const LAObject& indexinfo,
							const LAPricePayOff& payoff)
{
	(void)payoff; (void)trade; (void)basedate; (void)paydate; //20070411--Nagase--Œxíœ‚ðgcc‚É‚à‘Î‰ž
	const LADataHolder* dh;
	//round function
	dh = &(indexinfo.getData(PRICING_DATA_ROUNDFUNCTION, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		mIsRound = true;
		LAString roundfunction = dynamic_cast<const LADataString&>(dh->get()).get();
		roundfunction.toUpper();
		if (roundfunction == ROUND_STR) mRoundFunction = ROUND;
		else if (roundfunction == ROUND_UP_STR) mRoundFunction = ROUND_UP;
		else mRoundFunction = ROUND_DOWN;

		dh = &(indexinfo.getData(PRICING_DATA_ROUNDDIGIT, ISNOTNULL));
		mRoundDigit = dynamic_cast<const LADataInt&>(dh->get()).get();
	}
	else
		mIsRound = false;

	dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONENDDATE, NOCHECK));
	if (!dh->isDefined() || dh->isNull())
	{
		dh = &(indexinfo.getData(PRICING_DATA_FIXINGDATE, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			const LADate& fixingdate = dynamic_cast<const LADataDate&>(dh->get()).get();
			if (fixingdate > paydate)
			{
				//error
				LAString msg = "Index fixingdate is after paymentdate!";
				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
		}
	}

	mDiscountRatio = 1.0;
	mIsDFRatioUpdate = false;
	dh = &(indexinfo.getData(PRICING_DATA_DISCOUNTSTARTDATE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		const LADate& startdate = dynamic_cast<const LADataDate&>(dh->get()).get();
		
		dh = &(indexinfo.getData(PRICING_DATA_DISCOUNTENDDATE, ISDEFINED));
		const LADate &enddate = dynamic_cast<const LADataDate&>(dh->get()).get();

		if (startdate > enddate)
			throw LACoreInvalidData("Startdate and Enddate error",__FILE__,__LINE__);

		//get path object
		dh = &(trade.getData("PathEntity", ISNOTNULL));
		const LADataReference& pathref = dynamic_cast<const LADataReference &>(dh->get());
		
		if(!pathref.get().get().isTypeOf(ENTITY_PLAINVANILLA))
			throw LACoreInvalidData("Discount adjust only supports plain vanilla case",__FILE__,__LINE__);

		const LAMathPlainVanillaEntity& vpath = (dynamic_cast<const LAMathPlainVanillaEntity&>(pathref.get().get()));


		LAString curvename = "STD";
		//get index currency
		LAString indexcur;
		const LAObject *pFra = 0;
		dh = &(indexinfo.getData(PRICING_DATA_FRAINDEXINFO, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			pFra = &dynamic_cast<const LADataReference &>(dh->get()).get().get();
		}
		else
		{
			pFra = &indexinfo;
		}
		dh = &(pFra->getData(PRICING_DATA_CURRENCY, ISNOTNULL));
		indexcur = dynamic_cast<const LADataString &>(dh->get()).get();
		
		dh = &(pFra->getData(PRICING_DATA_BASISCURVE, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			curvename = dynamic_cast<const LADataString &>(dh->get()).get();
		}

		// get curve convention
		LAString freq;
		LAPriceDataDayCount dc;
		LAPriceDataCalendar cal;
		LAPriceDataSlidingRule sld;
		LAString accessory;
		vpath.getIRCurve(indexcur).getCurveConvention(freq, cal, sld, dc, accessory, curvename);	
		
		LAPriceDataConvention conv(dc.getDayCount(), SIMPL);
		const double term = dc.getTerm(startdate, enddate, false);
		dh = &(pFra->getData(PRICING_DATA_FIXEDRATE, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			const double rate = dynamic_cast<const LADataDouble &>(dh->get()).get();
			mDiscountRatio = 1.0 / LAPriceDataConvention::rateToRet(rate, term, conv);
		}
		else
		{
			LAPriceDataDayCount act365_ISDA(ACT_365_ISDA);
			const double startterm = act365_ISDA.getTerm(basedate, startdate);
			const double endterm = act365_ISDA.getTerm(basedate, enddate);
			// check forward interpolation
			bool isFWDInter = false;
			dh = &(pFra->getData(PRICING_DATA_ISFWDINTERPOLATION, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				isFWDInter = dynamic_cast<const LADataBool &>(dh->get()).get();
			}
			if (isFWDInter)
			{
				DoubleArray terms;
				DoubleMatrix termsMat;
				DoubleArray tau;
				DoubleArray fwds;	
				vpath.getIRCurve(indexcur).getBaseForwardRate(curvename, terms, termsMat, tau,  fwds);
				LAPriceDataInterpolation inter = dynamic_cast<const LAPriceDataInterpolation &>(pFra->getData(PRICING_DATA_FWDINTERPOLATION, ISNOTNULL).get());
				inter.set(terms, fwds);
				const double rate = inter.value(startterm);
				mDiscountRatio = 1.0 / LAPriceDataConvention::rateToRet(rate, term, conv);
			}
			else
			{
				const LAInterpolationBase& pInter = vpath.getIRCurve(indexcur).getDFInterpolation(&curvename);
				mDiscountRatio = pInter.value(endterm);
				mDiscountRatio /= pInter.value(startterm);
			}
		}
		mIsDFRatioUpdate = true;
	}
}

/*!
	@brief set up this class
	@param[in] basedate basedate
	@param[in] trade trade object
	@param[in] indexinfo index information object
	@param[in] fixingdate fixingdate
	@param[in] payoff LAPricePayOff object
*/
void
LAPriceIndexToolBase::setUp(const LADate& basedate,	
						const LAObject& trade,
						const LAObject& indexinfo,
						const LADate& fixingdate,
						const LAPricePayOff& payoff)
{
	LAPriceIndexToolBase::setUp(basedate, fixingdate, trade, indexinfo, payoff);
}

bool
LAPriceIndexToolBase::isSavePastFixing(const LAObject& trade)
{
	const LADataHolder* dh;
	// is save past fixing
	bool isSavePastFixing = false;
	dh = &(trade.getData(PRICING_DATA_ISSAVEPASTFIXING, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		isSavePastFixing = dynamic_cast<const LADataBool &>(dh->get()).get();
	}
	return isSavePastFixing;
}



/*!
    @brief constructor
*/
LAPriceIndexTool::LAPriceIndexTool()
: LAPriceIndexToolBase(), mCap(0), mFloor(0), mpIndex(0), mpObservationOperator(0)
{

}
/*!
    @brief destructor
*/
LAPriceIndexTool::~LAPriceIndexTool()
{
}

/*!
	@brief make copy(clone) of this class
    @return deep copy of this class
*/
LAPriceIndexToolBase*
LAPriceIndexTool::clone() const
{
    try 
	{
        return new LAPriceIndexTool(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }	
}

/*!
	@brief set up this class
	@param[in] basedate basedate
	@param[in] paydate paymentdate
	@param[in] trade trade object
	@param[in] indexinfo index information object
	@param[in] payoff LAPricePayOff object
*/
void
LAPriceIndexTool::setUp(const LADate& basedate,
							const LADate& paydate,
							const LAObject& trade,
							const LAObject& indexinfo,
							const LAPricePayOff& payoff)
{
	LAPriceIndexToolBase::setUp(basedate, paydate, trade, indexinfo, payoff);

	mX.clear();
	mpObservationOperator = NULL;
	mpIndex = NULL;

	const LADataHolder* dh;
	//index object
	dh = &(indexinfo.getData(PRICING_DATA_INDEXENTITY, ISNOTNULL));
	const LADataReference& ref = dynamic_cast<const LADataReference&>(dh->get());
	mpIndex = &dynamic_cast<const LAMathIndexEntity&>(ref.get().get());

	// is save past fixing
	const bool isSavePFixing = isSavePastFixing(trade);
	//observation start
	dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONSTARTDATE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		const LADate& start = dynamic_cast<const LADataDate&>(dh->get());
		//observation end
		dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONENDDATE, ISNOTNULL)); 
		const LADate& end = dynamic_cast<const LADataDate&>(dh->get());
		//frequency
		dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONFREQUENCY, ISNOTNULL)); 
		const LAString& freq = dynamic_cast<const LADataString&>(dh->get());
		//day
		int* pday = NULL;
		int day;
		dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONDAY, NOCHECK)); 
		if (dh->isDefined() && !dh->isNull())
		{
			day = dynamic_cast<const LADataInt&>(dh->get()).get();
			pday = &day;
		}
		//slidingrule & calendar
		const LAPriceDataSlidingRule* psrule;
		const LAPriceDataCalendar* pcal ;
		LAPriceCFGenUtility::getBusDayRuleAndCalendar(indexinfo, 
													PRICING_DATA_OBSERVATIONSLIDINGRULE,
													PRICING_DATA_OBSERVATIONCALENDAR,
													indexinfo,
													CALIBRATION_DATA_SLIDINGRULE,
													CALIBRATION_DATA_CALENDAR,
													psrule, pcal);

		DateVector out;
		LAMathDateCalculations::generateSchedule(start, end, freq,	
									false,
									NULL, NULL,
									pday,
									out,
									psrule,
									pcal
									);
		LADate tmpDate;
		if (pcal != NULL)
			tmpDate = psrule->getDate(end, *pcal);
		else 
			tmpDate = end;
		if (out.back() != tmpDate) out.push_back(tmpDate);

		// observation same days
		dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONSAMEDAYS, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			int sameDays = dynamic_cast<const LADataInt&>(dh->get()).get();
			for (unsigned int i = 0; i < sameDays; ++i)
			{
				out.push_back(out.back());
			}
		}
	
		const DateVector& grid = mpIndex->getGrid();
		if (grid.size() == 0)
		{
			//error
			LAString msg = "Grid is not set in LAMathIndexEntity";
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
		
		unsigned int pos;
		bool isNeedPastRate = false;
		bool isBaseContain = false;
		for (unsigned int i = 0; i < out.size(); i++)
		{
			if (basedate <= out[i]) //future
			{
				if (!LAAlgorithm::find<DateVector, LADate>(grid, out[i], 0, grid.size() - 1, pos))
				{
					//error
					LAString msg = "LAMathIndexEntity grid is inconsistent with index observation dates";
					throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
				}

				if (basedate == out[i])
				{
					isBaseContain = true;
				}
				mIndexPos.push_back(pos);
			}
			else
			{
				isNeedPastRate = true;
			}
		}	
		
		//past rate
		if (isNeedPastRate)
		{
			if (isSavePFixing)
			{
				DateVector observationdates;
				DoubleArray observationrates;
				dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONDATES, NOCHECK));
				if (dh->isDefined() && !dh->isNull())
				{
					observationdates = dynamic_cast<const LADataDates&>(dh->get()).get();
					dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONRATES, ISNOTNULL));
					observationrates = dynamic_cast<const LADataDoubles&>(dh->get()).get();
				}
				for (unsigned int i = 0; i < out.size(); i++)
				{
					if (basedate > out[i]) //past
					{
						if (!LAAlgorithm::find<DateVector, LADate>(observationdates, out[i], 0, observationdates.size() - 1, pos))
						{
							if (!observationdates.empty() && observationdates.back() > out[i])
							{
								//error
								LAString msg = "Index rate at ";
								msg += LADataDate(out[i]).convertToString();
								msg += " is needed.";
								throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
							}
							mIndexPos.insert(mIndexPos.begin(), 0);
						}
						else
						{
							mFixedRates.push_back(observationrates.at(pos));
						}
					}
					else
					{
						break;
					}
				}
				
			}
			else
			{
				dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONDATES, ISNOTNULL));
				const DateVector& observationdates = dynamic_cast<const LADataDates&>(dh->get()).get();
				dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONRATES, ISNOTNULL));
				const DoubleArray& observationrates = dynamic_cast<const LADataDoubles&>(dh->get()).get();
				if (observationdates.size() != observationrates.size())
				{
					throw LACoreInvalidData("ObservationDates size and ObservationRates size are not same.", __FILE__, __LINE__);
				}
				for (unsigned int i = 0; i < out.size(); i++)
				{
					if (basedate > out[i]) //past
					{
						if (!LAAlgorithm::find<DateVector, LADate>(observationdates, out[i], 0, observationdates.size() - 1, pos))
						{
							//error
							LAString msg = "Index rate at ";
							msg += LADataDate(out[i]).convertToString();
							msg += " is needed.";
							throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
						}
						mFixedRates.push_back(observationrates.at(pos));
					}
					else
					{
						break;
					}
				}
			}
		}
		
		// basedate rate
		if (isBaseContain)
		{
			dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONDATES, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				const DateVector& observationdates = dynamic_cast<const LADataDates&>(dh->get()).get();
				if (LAAlgorithm::find<DateVector, LADate>(observationdates, basedate, 0, observationdates.size() - 1, pos))
				{
					dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONRATES, ISNOTNULL));
					const DoubleArray& observationrates = dynamic_cast<const LADataDoubles&>(dh->get()).get();
					if (observationdates.size() != observationrates.size())
					{
						throw LACoreInvalidData("ObservationDates size and ObservationRates size are not same.", __FILE__, __LINE__);
					}
					mFixedRates.push_back(observationrates.at(pos));
					mIndexPos.erase(mIndexPos.begin());
				}
			}
		}		

		// observation operator
		dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONOPERATOR, ISNOTNULL));	 
		mpObservationOperator = &dynamic_cast<const LAPriceDataFunction&>(dh->get()).getFunction();

		
	}
	else
	{
		//fixing date
		dh = &(indexinfo.getData(PRICING_DATA_FIXINGDATE, ISNOTNULL));	 
		const LADate& fixingdate = dynamic_cast<const LADataDate&>(dh->get()).get();
		const DateVector& grid = mpIndex->getGrid();
		if (grid.size() == 0)
		{
			//error
			LAString msg = "Grid is not set in LAMathIndexEntity";
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}

		if (isSavePFixing && basedate > fixingdate)
		{
			mIndexPos.push_back(0);
		}
		else
		{
			unsigned int pos;
			if (!LAAlgorithm::find<DateVector, LADate>(grid, fixingdate, 0, grid.size() - 1, pos))
			{
				//error
				LAString msg = "LAMathIndexEntity grid is inconsistent with index fixing dates";
				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
			mIndexPos.push_back(pos);
		}

	
	}

	//cap
	mIsCap = false;
	dh = &(indexinfo.getData(PRICING_DATA_MAXINDEX, NOCHECK));	 
	if (dh->isDefined() && !dh->isNull())
	{
		const LAPriceDataFunction& method = dynamic_cast<const LAPriceDataFunction&>(dh->get());
		if (!method.isTypeOf(FN_CONSTANT))
		{
			//error
			LAString msg = "Only LAConstant function is available";
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);		
		}

		mCap = dynamic_cast<const LAConstant&>(method.getFunction()).getParam().at(0);
		mIsCap = true;
	}
	//floor
	mIsFloor = false;
	dh = &(indexinfo.getData(PRICING_DATA_MININDEX, NOCHECK));	 
	if (dh->isDefined() && !dh->isNull())
	{
		const LAPriceDataFunction& method = dynamic_cast<const LAPriceDataFunction&>(dh->get());
		if (!method.isTypeOf(FN_CONSTANT))
		{
			//error
			LAString msg = "Only LAConstant function is available";
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);		
		}

		mFloor = dynamic_cast<const LAConstant&>(method.getFunction()).getParam().at(0);
		mIsFloor = true;	
	}
	mX.resize(mFixedRates.size() + mIndexPos.size());
}


/*!
	@brief set up this class
	@param[in] basedate basedate
	@param[in] trade trade object
	@param[in] indexinfo index information object
	@param[in] fixingdate fixingdate
	@param[in] payoff LAPricePayOff object
*/
void
LAPriceIndexTool::setUp(const LADate& basedate,	
						const LAObject& trade,
						const LAObject& indexinfo,
						const LADate& fixingdate,
						const LAPricePayOff& payoff)
{
	LAPriceIndexToolBase::setUp(basedate, trade, indexinfo, fixingdate, payoff);
	const LADataHolder* dh;

	mX.clear();
	mpObservationOperator = NULL;
	mpIndex = NULL;


	//index object
	dh = &(indexinfo.getData(PRICING_DATA_INDEXENTITY, ISNOTNULL));
	const LADataReference& ref = dynamic_cast<const LADataReference&>(dh->get());
	mpIndex = &dynamic_cast<const LAMathIndexEntity&>(ref.get().get());
	// is save past fixing
	const bool isSavePFixing = isSavePastFixing(trade);
	// observation enddate
	dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONENDTERM, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		const LAString& endstr = dynamic_cast<const LADataString&>(dh->get()).get();
	
		const LAPriceDataSlidingRule* psrule;
		const LAPriceDataCalendar* pcal;
		LAPriceCFGenUtility::getBusDayRuleAndCalendar(indexinfo, 
													PRICING_DATA_OBSERVATIONSLIDINGRULE,
													PRICING_DATA_OBSERVATIONCALENDAR,
													indexinfo,
													CALIBRATION_DATA_SLIDINGRULE,
													CALIBRATION_DATA_CALENDAR,		
													psrule,
													pcal);	


		const LADate& end
			= LAMathDateCalculations::getDate(fixingdate, endstr, 
										*psrule,
										pcal,
										false);

		// observation startdate
		LADate start;
		dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONSTARTDATE, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			start = dynamic_cast<const LADataDate&>(dh->get()).get();
			if (psrule->getSlidingRule() != SLIDING_RULE_NO_CHANGE)
				start = psrule->getDate(start, *pcal);		
		}
		else
		{
			dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONSTARTTERM, ISNOTNULL));
			const LAString& startstr = dynamic_cast<const LADataString&>(dh->get()).get();
			start = LAMathDateCalculations::getDate(fixingdate, startstr, 
										*psrule,
										pcal,
										false);					
		}

		if (end < basedate)//past! 
		{
			if (isSavePFixing)
			{
				DateVector fixeddates;
				DoubleArray fixedrates;
				dh = &(indexinfo.getData(PRICING_DATA_FIXEDDATES, NOCHECK));
				if (dh->isDefined() && !dh->isNull())
				{
					dh = &(indexinfo.getData(PRICING_DATA_FIXEDRATES, ISNOTNULL));
					fixedrates = dynamic_cast<const LADataDoubles&>(dh->get()).get();		
				}
				unsigned int pos;
				if (!LAAlgorithm::find<DateVector, LADate>(fixeddates, end, 0, fixeddates.size() - 1, pos))
				{
					mIndexPos.push_back(0);
				}
				else
				{
					mFixedRates.push_back(fixedrates.at(pos));

				}				
			}
			else
			{
				dh = &(indexinfo.getData(PRICING_DATA_FIXEDDATES, ISNOTNULL));
				const DateVector& fixeddates = dynamic_cast<const LADataDates&>(dh->get()).get();
				dh = &(indexinfo.getData(PRICING_DATA_FIXEDRATES, ISNOTNULL));
				const DoubleArray& fixedrates = dynamic_cast<const LADataDoubles&>(dh->get()).get();		
				unsigned int pos;
				if (!LAAlgorithm::find<DateVector, LADate>(fixeddates, end, 0, fixeddates.size() - 1, pos))
				{
					//error	
					LAString msg = "No fixied rate at " + LADataDate(end).convertToString();
					throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);				
				}
				mFixedRates.push_back(fixedrates.at(pos));
			}

			mIsCap = false;
			mIsFloor = false;
			return;
		}


		//frequency
		dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONFREQUENCY, ISNOTNULL)); 
		const LAString& freq = dynamic_cast<const LADataString&>(dh->get());
		//day
		int* pday = NULL;
		int day;
		dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONDAY, NOCHECK)); 
		if (dh->isDefined() && !dh->isNull())
		{
			day = dynamic_cast<const LADataInt&>(dh->get()).get();
			pday = &day;
		}

		DateVector out;
		LAMathDateCalculations::generateSchedule(start, end, freq,	
									false,
									NULL, NULL,
									pday,
									out,
									psrule,
									pcal
									);

		if (pcal != NULL)
		{
			LADate tmp = psrule->getDate(end, *pcal);
			if (out.at(out.size() - 1) != tmp) out.push_back(tmp);
		}
	
		const DateVector& grid = mpIndex->getGrid();
		if (grid.size() == 0)
		{
			//error
			LAString msg = "Grid is not set in LAMathIndexEntity";
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
		unsigned int pos;
		bool isNeedPastRate = false;
		bool isBaseContain = false;
		for (unsigned int i = 0; i < out.size(); i++)
		{
			if (basedate <= out[i])
			{
				if (!LAAlgorithm::find<DateVector, LADate>(grid, out[i], 0, grid.size() - 1, pos))
				{
					//error
					LAString msg = "LAMathIndexEntity grid is inconsistent with index observation dates";
					throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
				}
				if (basedate == out[i])
				{
					isBaseContain = true;
				}
				mIndexPos.push_back(pos);
			}
			else 
			{
				isNeedPastRate = true;
			}
		}

		//past rate
		if (isNeedPastRate)
		{
			if (isSavePFixing)
			{
				DateVector observationdates;
				DoubleArray observationrates;
				dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONDATES, NOCHECK));
				if (dh->isDefined() && !dh->isNull())
				{
					observationdates = dynamic_cast<const LADataDates&>(dh->get()).get();
					dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONRATES, ISNOTNULL));
					observationrates = dynamic_cast<const LADataDoubles&>(dh->get()).get();
				}
				for (unsigned int i = 0; i < out.size(); i++)
				{
					if (basedate > out[i]) //past
					{
						if (!LAAlgorithm::find<DateVector, LADate>(observationdates, out[i], 0, observationdates.size() - 1, pos))
						{
							if (!observationdates.empty() && observationdates.back() > out[i])
							{
								//error
								LAString msg = "Index rate at ";
								msg += LADataDate(out[i]).convertToString();
								msg += " is needed.";
								throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
							}
							mIndexPos.insert(mIndexPos.begin(), 0);
						}
						else
						{
							mFixedRates.push_back(observationrates.at(pos));
						}
					}
					else
					{
						break;
					}
				}
			}
			else
			{
				dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONDATES, ISNOTNULL));
				const DateVector& observationdates = dynamic_cast<const LADataDates&>(dh->get()).get();
				dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONRATES, ISNOTNULL));
				const DoubleArray& observationrates = dynamic_cast<const LADataDoubles&>(dh->get()).get();
				if (observationdates.size() != observationrates.size())
				{
					throw LACoreInvalidData("ObservationDates size and ObservationRates size are not same.", __FILE__, __LINE__);
				}
				for (unsigned int i = 0; i < out.size(); i++)
				{	
					if (basedate > out[i]) //past
					{
						if (!LAAlgorithm::find<DateVector, LADate>(observationdates, out[i], 0, observationdates.size() - 1, pos))
						{
							//error
							LAString msg = "Index rate at ";
							msg += LADataDate(out[i]).convertToString();
							msg += " is needed.";
							throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
						}
						mFixedRates.push_back(observationrates.at(pos));
					}
					else
					{
						break;
					}
				}
			}
		}

		// basedate rate
		if (isBaseContain)
		{
			dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONDATES, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				const DateVector& observationdates = dynamic_cast<const LADataDates&>(dh->get()).get();
				if (LAAlgorithm::find<DateVector, LADate>(observationdates, basedate, 0, observationdates.size() - 1, pos))
				{
					dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONRATES, ISNOTNULL));
					const DoubleArray& observationrates = dynamic_cast<const LADataDoubles&>(dh->get()).get();
					if (observationdates.size() != observationrates.size())
					{
						throw LACoreInvalidData("ObservationDates size and ObservationRates size are not same.", __FILE__, __LINE__);
					}
					mFixedRates.push_back(observationrates.at(pos));
					mIndexPos.erase(mIndexPos.begin());
				}
			}
		}
		
		// observation operator		
		dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONOPERATOR, ISNOTNULL));	 
		mpObservationOperator = &dynamic_cast<const LAPriceDataFunction&>(dh->get()).getFunction();
		
	}
	else
	{
		const DateVector& grid = mpIndex->getGrid();
		if (grid.size() == 0)
		{
			//error
			LAString msg = "Grid is not set in LAMathIndexEntity";
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
		unsigned int pos;
		if (!LAAlgorithm::find<DateVector, LADate>(grid, fixingdate, 0, grid.size() - 1, pos))
		{
			//error
			LAString msg = "LAMathIndexEntity grid is inconsistent with index fixing dates";
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
		mIndexPos.push_back(pos);

	}

	//cap
	mIsCap = false;
	dh = &(indexinfo.getData(PRICING_DATA_MAXINDEX, NOCHECK));	 
	if (dh->isDefined() && !dh->isNull())
	{
		const LAPriceDataFunction& method = dynamic_cast<const LAPriceDataFunction&>(dh->get());
		if (!method.isTypeOf(FN_CONSTANT))
		{
			//error
			LAString msg = "Only LAConstant function is available";
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);		
		}

		mCap = dynamic_cast<const LAConstant&>(method.getFunction()).getParam().at(0);
		mIsCap = true;
	}
	//floor
	mIsFloor = false;
	dh = &(indexinfo.getData(PRICING_DATA_MININDEX, NOCHECK));	 
	if (dh->isDefined() && !dh->isNull())
	{
		const LAPriceDataFunction& method = dynamic_cast<const LAPriceDataFunction&>(dh->get());
		if (!method.isTypeOf(FN_CONSTANT))
		{
			//error
			LAString msg = "Only LAConstant function is available";
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);		
		}

		mFloor = dynamic_cast<const LAConstant&>(method.getFunction()).getParam().at(0);
		mIsFloor = true;	
	}

	mX.resize(mFixedRates.size() + mIndexPos.size());

}



/*!
    @brief constructor
*/
LAPriceIndexToolFixed::LAPriceIndexToolFixed()
: LAPriceIndexToolBase(), mRate(NULL), mRates(NULL), mPos(0), mpFixingDate(NULL)
{

}

/*!
    @brief destructor
*/
LAPriceIndexToolFixed::~LAPriceIndexToolFixed()
{
	if (mpFixingDate) delete mpFixingDate;
}

/*!
	@brief make copy(clone) of this class
    @return deep copy of this class
*/
LAPriceIndexToolBase*
LAPriceIndexToolFixed::clone() const
{
    try 
	{
        return new LAPriceIndexToolFixed(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }	
}

/*!
	@brief set up this class
	@param[in] basedate basedate
	@param[in] paydate paymentdate
	@param[in] trade trade object
	@param[in] indexinfo index information object
	@param[in] payoff LAPricePayOff object
*/
void
LAPriceIndexToolFixed::setUp(const LADate& basedate,
							const LADate& paydate,
							const LAObject& trade,
							const LAObject& indexinfo,
							const LAPricePayOff& payoff)
{
	LAPriceIndexToolBase::setUp(basedate, paydate, trade, indexinfo, payoff);	
	const LADataHolder* dh = &(indexinfo.getData(PRICING_DATA_FIXEDRATE, ISNOTNULL));
	//mRate = dynamic_cast<const LADataDouble&>(dh->get()).get();
	mRate = &dynamic_cast<const LADataDouble&>(dh->get());
	dh = &(indexinfo.getData(PRICING_DATA_FIXINGDATE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		mpFixingDate = new LADate(dynamic_cast<const LADataDate&>(dh->get()).get());
	}
}

/*!
	@brief set up this class
	@param[in] basedate basedate
	@param[in] trade trade object
	@param[in] indexinfo index information object
	@param[in] fixingdate fixingdate
	@param[in] payoff LAPricePayOff object
*/
void
LAPriceIndexToolFixed::setUp(const LADate& basedate,
						const LAObject& trade,
						const LAObject& indexinfo,
						const LADate& fixingdate,
						const LAPricePayOff& payoff)
{
	LAPriceIndexToolBase::setUp(basedate, trade, indexinfo, fixingdate, payoff);	

	const LADataHolder* dh;
	dh = &(indexinfo.getData(PRICING_DATA_FIXEDDATES, ISNOTNULL));
	const DateVector& fixeddates = dynamic_cast<const LADataDates&>(dh->get()).get();
	dh = &(indexinfo.getData(PRICING_DATA_FIXEDRATES, ISNOTNULL));
	const DoubleArray& fixedrates = dynamic_cast<const LADataDoubles&>(dh->get()).get();

	// observation enddate
	dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONENDTERM, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		const LAString& endstr = dynamic_cast<const LADataString&>(dh->get()).get();
	
		const LAPriceDataSlidingRule* psrule;
		const LAPriceDataCalendar* pcal;
		LAPriceCFGenUtility::getBusDayRuleAndCalendar(indexinfo, 
													PRICING_DATA_OBSERVATIONSLIDINGRULE,
													PRICING_DATA_OBSERVATIONCALENDAR,
													indexinfo,
													CALIBRATION_DATA_SLIDINGRULE,
													CALIBRATION_DATA_CALENDAR,		
													psrule,
													pcal);		
	
		const LADate& end
			= LAMathDateCalculations::getDate(fixingdate, endstr, 
										*psrule,
										pcal,
										false);

		if (end > fixingdate)
		{
			//error	
			LAString msg = "Obserbation end date is after fixing date";
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);		
		}
		
		unsigned int pos;
		if (!LAAlgorithm::find<DateVector, LADate>(fixeddates, end, 0, fixeddates.size() - 1, pos))
		{
			//error	
			LAString msg = "No fixied rate at " + LADataDate(end).convertToString();
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);				
		}

		mRate = NULL;
		mRates = &dynamic_cast<const LADataDoubles&>(indexinfo.getData(PRICING_DATA_FIXEDRATES, ISNOTNULL).get());
		mPos = pos;
	}
	else
	{
		unsigned int pos;
		if (!LAAlgorithm::find<DateVector, LADate>(fixeddates, fixingdate, 0, fixeddates.size() - 1, pos))
		{
			//error	
			LAString msg = "No fixied rate at" + LADataDate(fixingdate).convertToString();;
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);	
		}
		mRate = NULL;
		mRates = &dynamic_cast<const LADataDoubles&>(indexinfo.getData(PRICING_DATA_FIXEDRATES, ISNOTNULL).get());
		mPos = pos;
	}

}



/*!
    @brief constructor
*/
LAPriceIndexToolCpn::LAPriceIndexToolCpn()
: LAPriceIndexToolBase(), mPayOff(0), mLegNo(0),
   mpObservationOperator(0), mpCap(0), mpFloor(0)
{

}
/*!
    @brief destructor

*/
LAPriceIndexToolCpn::~LAPriceIndexToolCpn()
{
	if (mpCap != NULL) delete mpCap;
	if (mpFloor != NULL) delete mpFloor;
}





/*!
    @brief copy constructor

	@param[in] v LAPricePayOff object
*/
LAPriceIndexToolCpn::LAPriceIndexToolCpn(const LAPriceIndexToolCpn& v)
: LAPriceIndexToolBase(v), 
mPayOff(v.mPayOff), mLegNo(v.mLegNo), mPayOffPos(v.mPayOffPos),
   mpObservationOperator(v.mpObservationOperator), mFixedRates(v.mFixedRates),
   mX(v.mX), mpCap(0), mpFloor(0)
{
	if (v.mpCap != NULL) mpCap = dynamic_cast<LAFunctionBase*>(v.mpCap->clone());
	if (v.mpFloor != NULL) mpFloor = dynamic_cast<LAFunctionBase*>(v.mpFloor->clone());
}


/*!
	@brief make copy(clone) of this class
    @return deep copy of this class
*/
LAPriceIndexToolBase*
LAPriceIndexToolCpn::clone() const
{
    try 
	{
        return new LAPriceIndexToolCpn(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }	
}


/*!
	@brief set up this class
	@param[in] basedate basedate
	@param[in] paydate paymentdate
	@param[in] trade trade object
	@param[in] indexinfo index information object
	@param[in] payoff LAPricePayOff object
*/
void
LAPriceIndexToolCpn::setUp(const LADate& basedate,
							const LADate& paydate,
							const LAObject& trade,
							const LAObject& indexinfo,
							const LAPricePayOff& payoff)
{
	LAPriceIndexToolBase::setUp(basedate, paydate, trade, indexinfo, payoff);	
	
	mPayOff = &payoff;
	mpObservationOperator = NULL;
	mPayOffPos.clear();
	mFixedRates.clear();
	mX.clear();

	const LADataHolder* dh;
	//index type
	dh = &(indexinfo.getData(PRICING_DATA_INDEXTYPE, ISNOTNULL));		
	LAString indextype = dynamic_cast<const LADataString&>(dh->get()).get();
	indextype.toUpper();
	if (indextype != CPN && indextype != "CPNCF")
	{
		//error
		LAString msg = "Index type is not CPN or CPNCF";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	//index accessories
	mIsActualCF = false;
	dh = &(indexinfo.getData(PRICING_DATA_ACCESSORY, ISNOTNULL));
	LAString accessory  = dynamic_cast<const LADataString&>(dh->get()).get();
	accessory.toUpper();
	vector<LAString> accessoryVec(accessory.toToken('_'));
	bool IsLegSelected(false);
	for (unsigned int i = 0; i < accessoryVec.size(); i++)
	{
		if (accessoryVec[i].findString(LEG) == 0 && accessoryVec[i].size() > 3)
		{
			mLegNo = accessoryVec[i].subString(3, accessory.size() - 1).getIntValue();
			if (mLegNo > 0)
			{
				mLegNo -= 1;
			}
			else
			{
				//error
				LAString msg = "Accessory: ";
				msg += accessory;
				msg += " is wrong format";
				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
			IsLegSelected = true;
			mIsActualCF = true;
		}
		else if (IsLegSelected && accessoryVec[i].findString(COUPON) == 0 && accessoryVec[i].size() > 6)
		{
			const LADataMultiReference& legs(dynamic_cast<const LADataMultiReference&>(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get()));
			const LADataMultiReference& cashlets(dynamic_cast<const LADataMultiReference&>(legs.get(mLegNo).get().getData(PRICING_DATA_CASHLETS, ISNOTNULL).get()));
			LAString tmp1(LACoreUtility::removeQuotation(cashlets.convertToString()));
			tmp1.toUpper();
			std::vector<LAString> cashletnames(tmp1.toToken(':'));

			mCpnNo.resize(cashlets.getSize(), 0);
			for (unsigned int j = 0; j < mCpnNo.size(); j++)
			{
				dh = &(cashlets.get(j).get().getData(PRICING_DATA_COUPONINFOS, NOCHECK));
				if (dh->isNull() || !(dh->isDefined()))
				{
					continue;
				}
				const LADataMultiReference& couponinfos(dynamic_cast<const LADataMultiReference&>(dh->get()));

				////constant coupon case
				//dh = &(couponinfos.get(0).get().getData(PRICING_DATA_OPERATOR, ISNOTNULL));
				//const function_t couponOperatorType(dynamic_cast<const LAPriceDataFunction& >(dh->get()).getType());
				//if (couponOperatorType == FN_CONSTANT)
				//{
				//	continue;
				//}

				LAString tmp2(LACoreUtility::removeQuotation(couponinfos.convertToString()));
				tmp2.toUpper();
				std::vector<LAString> couponNames(tmp2.toToken(':'));

//				LAString tgtCouponName(legNames[mLegNo] + "_" + cashletnames[j] + "_" + accessoryVec[i]);
				LAString tgtCouponName(cashletnames[j] + "_" + accessoryVec[i]);

				std::vector<LAString>::iterator iter(std::find(couponNames.begin(), couponNames.end(), tgtCouponName));
				unsigned int index = std::distance(couponNames.begin(), iter);

				if(index == couponNames.size())
				{
					////Although target coupon is not found, trade is valid when coupon of cashlet is already fixed.
					////under this case size of couponInfo and its index info must be 1, and the index type is fixed rate.
					//if (couponinfos.getSize() == 1)
					//{
					//	dh = &(couponinfos.get(0).get().getData(PRICING_DATA_INDEXINFOS, ISNOTNULL));
					//	const LADataMultiReference& indexes(dynamic_cast<const LADataMultiReference&>(dh->get()));
					//	if (indexes.getSize() == 1)
					//	{
					//		LAString indextype(dynamic_cast<LADataString& >(indexes.get(0).get().getData(PRICING_DATA_INDEXTYPE, ISNOTNULL).get()).get());
					//		if(indextype.toUpper() == FIXEDRATE)
					//		{
					//			continue;
					//		}
					//	}
					//}

					//case of not conitued
					LAString msg;
					msg += accessoryVec[i] + " does not exists in " + cashletnames[j];
					throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
				}

				mCpnNo[j] = index;

			}
			mIsActualCF = false;
		}

	}

	if (!IsLegSelected)
	{
		//error
		LAString msg = "Accessory: ";
		msg += accessory;
		msg += " is wrong format";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	//observation start
	dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONSTARTDATE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		const LADate& startdate = dynamic_cast<const LADataDate&>(dh->get());
		//observation end
		dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONENDDATE, ISNOTNULL)); 
		const LADate& enddate = dynamic_cast<const LADataDate&>(dh->get());
		
		for (unsigned int i = 0; i < mPayOff->getPayOff()[mLegNo].size(); i++)
		{
			const LADate& paymentdate = mPayOff->getPayOff()[mLegNo][i].getPayOff().getPaymentDate();
			if (paymentdate >=startdate && paymentdate <= enddate && 
				mPayOff->getPayOff()[mLegNo][i].getPayOff().isCouponPayment())
				mPayOffPos.push_back(i);
		}
		setUpPastRate(basedate, trade, mLegNo, startdate, enddate);

		dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONOPERATOR, ISNOTNULL));	 
		mpObservationOperator = &dynamic_cast<const LAPriceDataFunction&>(dh->get()).getFunction();
		
	}
	else
	{
		//fixing date
		dh = &(indexinfo.getData(PRICING_DATA_FIXINGDATE, ISNOTNULL)); 
		const LADate& fixingdate = dynamic_cast<const LADataDate&>(dh->get());
		if (basedate < fixingdate)
		{
			for (unsigned int i = 0; i < mPayOff->getPayOff()[mLegNo].size(); i++)
			{
				const LADate& paymentdate = mPayOff->getPayOff()[mLegNo][i].getPayOff().getPaymentDate();
				if (fixingdate == paymentdate && mPayOff->getPayOff()[mLegNo][i].getPayOff().isCouponPayment())
				{
					mPayOffPos.push_back(i);
					break;
				}			
			}
		
			if (mPayOffPos.size() == 0)
			{
				//error
				LAString msg = "Fixing date must be fitted some paymentdate";
				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
		}
		else
		{
			dh = &(indexinfo.getData(PRICING_DATA_FIXEDRATE, NOCHECK)); 
			if (dh->isDefined() && !dh->isNull())
				mFixedRates.push_back(dynamic_cast<const LADataDouble&>(dh->get()).get());
			else
				setUpPastRate(trade, mLegNo, fixingdate);
		}


	}
	//cap
	mIsCap = false;
	dh = &(indexinfo.getData(PRICING_DATA_MAXINDEX, NOCHECK));	 
	if (dh->isDefined() && !dh->isNull())
	{
		const LAPriceDataFunction& method = dynamic_cast<const LAPriceDataFunction&>(dh->get());
		mpCap = dynamic_cast<LAFunctionBase*>(method.getFunction().clone());
		mIsCap = true;
	}
	//floor
	mIsFloor = false;
	dh = &(indexinfo.getData(PRICING_DATA_MININDEX, NOCHECK));	 
	if (dh->isDefined() && !dh->isNull())
	{
		const LAPriceDataFunction& method = dynamic_cast<const LAPriceDataFunction&>(dh->get());
		mpFloor = dynamic_cast<LAFunctionBase*>(method.getFunction().clone());
		mIsFloor = true;
	}

	if (mIsCap || mIsFloor)
	{
		//fixing date
		dh = &(indexinfo.getData(PRICING_DATA_FIXINGDATE, ISNOTNULL)); 
		const LADate& fixingdate = dynamic_cast<const LADataDate&>(dh->get());
		setUpCapandFloor(trade, mLegNo, fixingdate, payoff);
	}
	mX.resize(mFixedRates.size() + mPayOffPos.size());



}
/*!
	@brief set up this class
	@param[in] basedate basedate
	@param[in] trade trade object
	@param[in] indexinfo index information object
	@param[in] fixingdate fixingdate
	@param[in] payoff LAPricePayOff object
*/
void
LAPriceIndexToolCpn::setUp(const LADate& basedate,	
						const LAObject& trade,
						const LAObject& indexinfo,
						const LADate& fixingdate,
						const LAPricePayOff& payoff)
{
	LAPriceIndexToolBase::setUp(basedate, trade, indexinfo, fixingdate, payoff);	
	
	mPayOff = &payoff;
	mpObservationOperator = NULL;
	mPayOffPos.clear();
	mFixedRates.clear();
	mX.clear();

	const LADataHolder* dh;
	//index type
	dh = &(indexinfo.getData(PRICING_DATA_INDEXTYPE, ISNOTNULL));		
	LAString indextype = dynamic_cast<const LADataString&>(dh->get()).get();
	indextype.toUpper();
	if (indextype != CPN && indextype != "CPNCF")
	{
		//error
		LAString msg = "Index type is not CPN or CPNCF";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	//index accessories
	mIsActualCF = false;
	dh = &(indexinfo.getData(PRICING_DATA_ACCESSORY, ISNOTNULL));
	LAString accessory  = dynamic_cast<const LADataString&>(dh->get()).get();
	accessory.toUpper();
	vector<LAString> accessoryVec(accessory.toToken('_'));
	bool IsLegSelected(false);
	for (unsigned int i = 0; i < accessoryVec.size(); i++)
	{
		if (accessoryVec[i].findString(LEG) == 0 && accessoryVec[i].size() > 3)
		{
			mLegNo = accessoryVec[i].subString(3, accessory.size() - 1).getIntValue();
			if (mLegNo > 0)
			{
				mLegNo -= 1;
			}
			else
			{
				//error
				LAString msg = "Accessory: ";
				msg += accessory;
				msg += " is wrong format";
				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
			IsLegSelected = true;
			mIsActualCF = true;
		}
		else if (IsLegSelected && accessoryVec[i].findString(COUPON) == 0 && accessoryVec[i].size() > 6)
		{
			const LADataMultiReference& legs(dynamic_cast<const LADataMultiReference&>(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get()));
			const LADataMultiReference& cashlets(dynamic_cast<const LADataMultiReference&>(legs.get(mLegNo).get().getData(PRICING_DATA_CASHLETS, ISNOTNULL).get()));
			LAString tmp1(LACoreUtility::removeQuotation(cashlets.convertToString()));
			tmp1.toUpper();
			std::vector<LAString> cashletnames(tmp1.toToken(':'));

			mCpnNo.resize(cashlets.getSize(), 0);
			for (unsigned int j = 0; j < mCpnNo.size(); j++)
			{
				dh = &(cashlets.get(j).get().getData(PRICING_DATA_COUPONINFOS, NOCHECK));
				if (dh->isNull() || !(dh->isDefined()))
				{
					continue;
				}
				const LADataMultiReference& couponinfos(dynamic_cast<const LADataMultiReference&>(dh->get()));

				////constant coupon case
				//dh = &(couponinfos.get(0).get().getData(PRICING_DATA_OPERATOR, ISNOTNULL));
				//const function_t couponOperatorType(dynamic_cast<const LAPriceDataFunction& >(dh->get()).getType());
				//if (couponOperatorType == FN_CONSTANT)
				//{
				//	continue;
				//}

				LAString tmp2(LACoreUtility::removeQuotation(couponinfos.convertToString()));
				tmp2.toUpper();
				std::vector<LAString> couponNames(tmp2.toToken(':'));

//				LAString tgtCouponName(legNames[mLegNo] + "_" + cashletnames[j] + "_" + accessoryVec[i]);
				LAString tgtCouponName(cashletnames[j] + "_" + accessoryVec[i]);

				std::vector<LAString>::iterator iter(std::find(couponNames.begin(), couponNames.end(), tgtCouponName));
				unsigned int index = std::distance(couponNames.begin(), iter);

				if(index == couponNames.size())
				{
					////Although target coupon is not found, trade is valid when coupon of cashlet is already fixed.
					////under this case size of couponInfo and its index info must be 1, and the index type is fixed rate.
					//if (couponinfos.getSize() == 1)
					//{
					//	dh = &(couponinfos.get(0).get().getData(PRICING_DATA_INDEXINFOS, ISNOTNULL));
					//	const LADataMultiReference& indexes(dynamic_cast<const LADataMultiReference&>(dh->get()));
					//	if (indexes.getSize() == 1)
					//	{
					//		LAString indextype(dynamic_cast<LADataString& >(indexes.get(0).get().getData(PRICING_DATA_INDEXTYPE, ISNOTNULL).get()).get());
					//		if(indextype.toUpper() == FIXEDRATE)
					//		{
					//			continue;
					//		}
					//	}
					//}

					//case of not conitued
					LAString msg;
					msg += accessoryVec[i] + " does not exists in " + cashletnames[j];
					throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
				}

				mCpnNo[j] = index;

			}
			mIsActualCF = false;
		}

	}

	if (!IsLegSelected)
	{
		//error
		LAString msg = "Accessory: ";
		msg += accessory;
		msg += " is wrong format";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}


	// observation enddate
	dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONENDTERM, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		const LAString& endstr = dynamic_cast<const LADataString&>(dh->get()).get();

		const LAPriceDataSlidingRule* psrule;
		const LAPriceDataCalendar* pcal;
		LAPriceCFGenUtility::getBusDayRuleAndCalendar(indexinfo, 
													PRICING_DATA_OBSERVATIONSLIDINGRULE,
													PRICING_DATA_OBSERVATIONCALENDAR,
													indexinfo,
													CALIBRATION_DATA_SLIDINGRULE,
													CALIBRATION_DATA_CALENDAR,		
													psrule,
													pcal);

		const LADate& enddate
			= LAMathDateCalculations::getDate(fixingdate, endstr, 
										*psrule,
										pcal,
										false);	



		// observation startdate
		LADate startdate;
		dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONSTARTDATE, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			startdate = dynamic_cast<const LADataDate&>(dh->get()).get();
			if (psrule->getSlidingRule() != SLIDING_RULE_NO_CHANGE)
				startdate = psrule->getDate(startdate, *pcal);		
		}
		else
		{
			dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONSTARTTERM, ISNOTNULL));
			const LAString& startstr = dynamic_cast<const LADataString&>(dh->get()).get();
			startdate = LAMathDateCalculations::getDate(fixingdate, startstr, 
										*psrule,
										pcal,
										false);		
		}


		for (unsigned int i = 0; i < mPayOff->getPayOff()[mLegNo].size(); i++)
		{
			const LADate& paymentdate = mPayOff->getPayOff()[mLegNo][i].getPayOff().getPaymentDate();
			if (paymentdate >=startdate && paymentdate <= enddate && 
				mPayOff->getPayOff()[mLegNo][i].getPayOff().isCouponPayment())
				mPayOffPos.push_back(i);
		}
		setUpPastRate(basedate, trade, mLegNo, startdate, enddate);

		dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONOPERATOR, ISNOTNULL));	 
		mpObservationOperator = &dynamic_cast<const LAPriceDataFunction&>(dh->get()).getFunction();
		
	}
	else
	{
		if (basedate < fixingdate)
		{
			for (unsigned int i = 0; i < mPayOff->getPayOff()[mLegNo].size(); i++)
			{
				const LADate& paymentdate = mPayOff->getPayOff()[mLegNo][i].getPayOff().getPaymentDate();
				if (fixingdate == paymentdate && mPayOff->getPayOff()[mLegNo][i].getPayOff().isCouponPayment())
				{
					mPayOffPos.push_back(i);
					break;
				}			
			}
			if (mPayOffPos.size() == 0)
			{
				//error
				LAString msg = "Fixing date must be fitted some paymentdate";
				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
		}
		else
		{
			dh = &(indexinfo.getData(PRICING_DATA_FIXEDRATE, NOCHECK)); 
			if (dh->isDefined() && !dh->isNull())
				mFixedRates.push_back(dynamic_cast<const LADataDouble&>(dh->get()).get());
			else
				setUpPastRate(trade, mLegNo, fixingdate);
		}
	}

	//cap
	mIsCap = false;
	dh = &(indexinfo.getData(PRICING_DATA_MAXINDEX, NOCHECK));	 
	if (dh->isDefined() && !dh->isNull())
	{
		const LAPriceDataFunction& method = dynamic_cast<const LAPriceDataFunction&>(dh->get());
		mpCap = dynamic_cast<LAFunctionBase*>(method.getFunction().clone());
		mIsCap = true;
	}
	//floor
	mIsFloor = false;
	dh = &(indexinfo.getData(PRICING_DATA_MININDEX, NOCHECK));	 
	if (dh->isDefined() && !dh->isNull())
	{
		const LAPriceDataFunction& method = dynamic_cast<const LAPriceDataFunction&>(dh->get());
		mpFloor = dynamic_cast<LAFunctionBase*>(method.getFunction().clone());
		mIsFloor = true;
	}
	if (mIsCap || mIsFloor)
	{
		setUpCapandFloor(trade, mLegNo, fixingdate, payoff);
	}


	mX.resize(mFixedRates.size() + mPayOffPos.size());

}

/*!
	@brief set up past rate
	@param[in] trade trade object
	@param[in] legNo leg number(first leg number is 0)
	@param[in] fixingdate fixing date
*/
void
LAPriceIndexToolCpn::setUpPastRate(const LAObject& trade,
									unsigned int legNo,
									const LADate& fixingdate)
{
	//leg object
	const LADataHolder* dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const LADataMultiReference& legs = dynamic_cast<const LADataMultiReference&>(dh->get());
	const bool isSavePFixing = isSavePastFixing(trade);
	unsigned int pos;
	if (isSavePFixing)
	{
		DateVector paymentdates;
		DoubleArray coupons;
		dh = &(legs.get(legNo).getData(PRICING_DATA_PAYMENTDATES, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			paymentdates = dynamic_cast<const LADataDates&>(dh->get()).get();
			dh = &(legs.get(mLegNo).getData(PRICING_DATA_COUPONS, ISNOTNULL));
			coupons = dynamic_cast<const LADataDoubles&>(dh->get()).get();
		}

		if (!LAAlgorithm::find<DateVector, LADate>(paymentdates, fixingdate, 0, paymentdates.size() - 1, pos))
		{
			mFixedRates.push_back(0.0);
		}
		else
		{
			mFixedRates.push_back(coupons.at(pos));
		}
	}
	else
	{
		// past coupon
		dh = &(legs.get(legNo).getData(PRICING_DATA_PAYMENTDATES, ISNOTNULL)); 
		const DateVector& paymentdates = dynamic_cast<const LADataDates&>(dh->get()).get();
		dh = &(legs.get(mLegNo).getData(PRICING_DATA_COUPONS, ISNOTNULL)); 
		const DoubleArray& coupons = dynamic_cast<const LADataDoubles&>(dh->get()).get();
		if (!LAAlgorithm::find<DateVector, LADate>(paymentdates, fixingdate, 0, paymentdates.size() - 1, pos))
		{
			//error
			LAString msg = "Past Coupon at fixingdate(";
			msg += LADataDate(fixingdate).convertToString();
			msg += ") is needed.";
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
		mFixedRates.push_back(coupons.at(pos));
	}
}
/*!
	@brief set up past rate
	@param[in] basedate basedate
	@param[in] trade trade object
	@param[in] legNo leg number(first leg number is 0)
	@param[in] observationstartdate observation start date
	@param[in] observationenddate observation end date
*/
void
LAPriceIndexToolCpn::setUpPastRate(const LADate& basedate,	
								const LAObject& trade,
									unsigned int legNo,
									const LADate& observationstartdate,
									const LADate& observationenddate)
{

	//leg object
	const LADataHolder* dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const LADataMultiReference& legs = dynamic_cast<const LADataMultiReference&>(dh->get());

	// past coupon
	//get past paymentdate
	DateVector needdates;
	dh = &(legs.get(legNo).getData(PRICING_DATA_CASHLETS, ISNOTNULL));
	const LADataMultiReference& cashlets = dynamic_cast<const LADataMultiReference&>(dh->get());
	for (unsigned int i = 0; i < cashlets.getSize(); i++)
	{
		dh = &(cashlets.get(i).getData(PRICING_DATA_COUPONINFOS, NOCHECK));
		if (!dh->isDefined() || dh->isNull()) continue;//not coupon payment			
		dh = &(cashlets.get(i).getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL));
		const LADate& paymentdate = dynamic_cast<const LADataDate&>(dh->get()).get();
		if (paymentdate < observationstartdate) continue;
		if (paymentdate > basedate || paymentdate > observationenddate) break;
		needdates.push_back(paymentdate);
	}		
	if (needdates.size() > 0)
	{
		const bool isSavePFixing = isSavePastFixing(trade);
		if (isSavePFixing)
		{
			DateVector paymentdates;
			DoubleArray coupons;
			dh = &(legs.get(legNo).getData(PRICING_DATA_PAYMENTDATES, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				paymentdates = dynamic_cast<const LADataDates&>(dh->get()).get();
				dh = &(legs.get(mLegNo).getData(PRICING_DATA_COUPONS, ISNOTNULL));
				coupons = dynamic_cast<const LADataDoubles&>(dh->get()).get();
			}
			for (unsigned int i = 0; i < needdates.size(); i++)
			{
				unsigned int pos;
				if (!LAAlgorithm::find<DateVector, LADate>(paymentdates, needdates[i], 0, paymentdates.size() - 1, pos))
				{
					mFixedRates.push_back(0.0);
				}
				else
				{
					mFixedRates.push_back(coupons.at(pos));	
				}
			}
		}
		else
		{
			// past coupon
			dh = &(legs.get(legNo).getData(PRICING_DATA_PAYMENTDATES, ISNOTNULL)); 
			const DateVector& paymentdates = dynamic_cast<const LADataDates&>(dh->get()).get();
			dh = &(legs.get(legNo).getData(PRICING_DATA_COUPONS, ISNOTNULL)); 
			const DoubleArray& coupons = dynamic_cast<const LADataDoubles&>(dh->get()).get();
			for (unsigned int i = 0; i < needdates.size(); i++)
			{
				unsigned int pos;
				if (!LAAlgorithm::find<DateVector, LADate>(paymentdates, needdates[i], 0, paymentdates.size() - 1, pos))
				{
					//error
					LAString msg = "Past Coupon at ";
					msg += LADataDate(needdates[i]).convertToString();
					msg += " is needed.";
					throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
				}
				mFixedRates.push_back(coupons.at(pos));	
			}
		}
	}	


	// past coupon
/*	dh = &(legs.get(legNo).getData(PRICING_DATA_PAYMENTDATES, NOCHECK)); 
	if (dh->isDefined() && !dh->isNull())
	{
		const DateVector& paymentdates = dynamic_cast<const LADataDates&>(dh->get()).get();
		dh = &(legs.get(mLegNo).getData(PRICING_DATA_COUPONS, ISNOTNULL)); 
		const DoubleArray& coupons = dynamic_cast<const LADataDoubles&>(dh->get()).get();
		for (unsigned int i = 0; i < paymentdates.size(); i++)
			if (paymentdates[i] >=observationstartdate && paymentdates[i] <= observationenddate)
				mFixedRates.push_back(coupons.at(i));
	}*/
}

/*!
	@brief set up cap and floor condition
	@param[in] trade trade object
	@param[in] legNo leg number(first leg number is 0)
	@param[in] fixingdate fixing date
	@param[in] payoff payoff
*/
void
LAPriceIndexToolCpn::setUpCapandFloor(const LAObject& trade,	 
				 unsigned int legNo, 
				 const LADate& fixingdate,
				 const LAPricePayOff& payoff)
{
	(void)payoff; //20070411--Nagase--Œxíœ‚ðgcc‚É‚à‘Î‰ž
	mLatestPos = -1;
	for (unsigned int i = 0; i < mPayOff->getPayOff()[mLegNo].size(); i++)
	{
		const LADate& paymentdate = mPayOff->getPayOff()[mLegNo][i].getPayOff().getPaymentDate();
		if (paymentdate <= fixingdate) 
			mLatestPos = i;
		else
			break;
	}
	//leg object
	const LADataHolder* dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const LADataMultiReference& legs = dynamic_cast<const LADataMultiReference&>(dh->get());
	// past coupon
	mFixedRatesforCapFloor.clear();
	dh = &(legs.get(legNo).getData(PRICING_DATA_PAYMENTDATES, NOCHECK)); 
	if (dh->isDefined() && !dh->isNull())
	{
		const DateVector& paymentdates = dynamic_cast<const LADataDates&>(dh->get()).get();
		dh = &(legs.get(mLegNo).getData(PRICING_DATA_COUPONS, ISNOTNULL)); 
		const DoubleArray& coupons = dynamic_cast<const LADataDoubles&>(dh->get()).get();
		for (unsigned int i = 0; i < paymentdates.size(); i++)
			if (paymentdates[i] <= fixingdate)
				mFixedRatesforCapFloor.push_back(coupons.at(i));
	}
	mX2.clear();
	mX2.resize(mFixedRatesforCapFloor.size() + mLatestPos + 1 + 1);

}

/*!
    @brief constructor
*/
LAPriceIndexToolCpnCF::LAPriceIndexToolCpnCF()
: LAPriceIndexToolCpn()
{

}
/*!
    @brief destructor

*/
LAPriceIndexToolCpnCF::~LAPriceIndexToolCpnCF()
{

}

/*!
	@brief make copy(clone) of this class
    @return deep copy of this class
*/
LAPriceIndexToolBase*
LAPriceIndexToolCpnCF::clone() const
{
    try 
	{
        return new LAPriceIndexToolCpnCF(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }	
}

/*!
	@brief set up this class
	@param[in] basedate basedate
	@param[in] paydate paymentdate
	@param[in] trade trade object
	@param[in] indexinfo index information object
	@param[in] payoff LAPricePayOff object
*/
void
LAPriceIndexToolCpnCF::setUp(const LADate& basedate,
							const LADate& paydate,
							const LAObject& trade,
							const LAObject& indexinfo,
							const LAPricePayOff& payoff)
{
	LAPriceIndexToolCpn::setUp(basedate, paydate, trade, indexinfo, payoff);	
}
/*!
	@brief set up this class
	@param[in] basedate basedate
	@param[in] trade trade object
	@param[in] indexinfo index information object
	@param[in] fixingdate fixingdate
	@param[in] payoff LAPricePayOff object
*/
void
LAPriceIndexToolCpnCF::setUp(const LADate& basedate,	
							const LAObject& trade,
							const LAObject& indexinfo,
							const LADate& fixingdate,
							const LAPricePayOff& payoff)
{
	LAPriceIndexToolCpn::setUp(basedate, trade, indexinfo, fixingdate, payoff);	
}

/*!
	@brief set up past rate
	@param[in] trade trade object
	@param[in] legNo leg number(first leg number is 0)
	@param[in] fixingdate fixing date
*/
void
LAPriceIndexToolCpnCF::setUpPastRate(const LAObject& trade,
									unsigned int legNo,
									const LADate& fixingdate)
{
	//leg object
	const LADataHolder* dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const LADataMultiReference& legs = dynamic_cast<const LADataMultiReference&>(dh->get());
	const bool isSavePFixing = isSavePastFixing(trade);
	unsigned int pos;
	if (isSavePFixing)
	{
		DateVector paymentdates;
		DoubleArray couponcfs;
		dh = &(legs.get(legNo).getData(PRICING_DATA_PAYMENTDATES, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			paymentdates = dynamic_cast<const LADataDates&>(dh->get()).get();
			dh = &(legs.get(mLegNo).getData(PRICING_DATA_COUPONPAYOFFS, ISNOTNULL));
			couponcfs = dynamic_cast<const LADataDoubles&>(dh->get()).get();
		}

		if (!LAAlgorithm::find<DateVector, LADate>(paymentdates, fixingdate, 0, paymentdates.size() - 1, pos))
		{
			mFixedRates.push_back(0.0);
		}
		else
		{
			mFixedRates.push_back(couponcfs.at(pos));
		}
	}
	else
	{
		// past coupon
		dh = &(legs.get(legNo).getData(PRICING_DATA_PAYMENTDATES, ISNOTNULL)); 
		const DateVector& paymentdates = dynamic_cast<const LADataDates&>(dh->get()).get();
		dh = &(legs.get(mLegNo).getData(PRICING_DATA_COUPONPAYOFFS, ISNOTNULL)); 
		const DoubleArray& couponcfs = dynamic_cast<const LADataDoubles&>(dh->get()).get();

		if (!LAAlgorithm::find<DateVector, LADate>(paymentdates, fixingdate, 0, paymentdates.size() - 1, pos))
		{
			//error
			LAString msg = "Past CouponCF of fixingdate(";
			msg += LADataDate(fixingdate).convertToString();
			msg += ") is needed.";
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
		mFixedRates.push_back(couponcfs.at(pos));
	}
}
/*!
	@brief set up past rate
	@param[in] basedate basedate
	@param[in] trade trade object
	@param[in] legNo leg number(first leg number is 0)
	@param[in] observationstartdate observation start date
	@param[in] observationenddate observation end date
*/
void
LAPriceIndexToolCpnCF::setUpPastRate(const LADate& basedate,	
								  const LAObject& trade,
									unsigned int legNo,
									const LADate& observationstartdate,
									const LADate& observationenddate)
{

	//leg object
	const LADataHolder* dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const LADataMultiReference& legs = dynamic_cast<const LADataMultiReference&>(dh->get());
	
	// past couponcf
	//get past paymentdate
	DateVector needdates;
	dh = &(legs.get(legNo).getData(PRICING_DATA_CASHLETS, ISNOTNULL));
	const LADataMultiReference& cashlets = dynamic_cast<const LADataMultiReference&>(dh->get());
	for (unsigned int i = 0; i < cashlets.getSize(); i++)
	{
		dh = &(cashlets.get(i).getData(PRICING_DATA_COUPONINFOS, NOCHECK));
		if (!dh->isDefined() || dh->isNull()) continue;//not coupon payment			
		dh = &(cashlets.get(i).getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL));
		const LADate& paymentdate = dynamic_cast<const LADataDate&>(dh->get()).get();
		if (paymentdate < observationstartdate) continue;
		if (paymentdate > basedate || paymentdate > observationenddate) break;
		needdates.push_back(paymentdate);
	}		
	if (needdates.size() > 0)
	{
		
		const bool isSavePFixing = isSavePastFixing(trade);
		if (isSavePFixing)
		{
			DateVector paymentdates;
			DoubleArray couponcfs;
			dh = &(legs.get(legNo).getData(PRICING_DATA_PAYMENTDATES, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				paymentdates = dynamic_cast<const LADataDates&>(dh->get()).get();
				dh = &(legs.get(mLegNo).getData(PRICING_DATA_COUPONPAYOFFS, ISNOTNULL));
				couponcfs = dynamic_cast<const LADataDoubles&>(dh->get()).get();
			}
			for (unsigned int i = 0; i < needdates.size(); i++)
			{
				unsigned int pos;
				if (!LAAlgorithm::find<DateVector, LADate>(paymentdates, needdates[i], 0, paymentdates.size() - 1, pos))
				{
					mFixedRates.push_back(0.0);
				}
				else
				{
					mFixedRates.push_back(couponcfs.at(pos));
				}
			}
		}
		else
		{
			// past coupon
			dh = &(legs.get(legNo).getData(PRICING_DATA_PAYMENTDATES, ISNOTNULL)); 
			const DateVector& paymentdates = dynamic_cast<const LADataDates&>(dh->get()).get();
			dh = &(legs.get(legNo).getData(PRICING_DATA_COUPONPAYOFFS, ISNOTNULL)); 
			const DoubleArray& couponcfs = dynamic_cast<const LADataDoubles&>(dh->get()).get();
			for (unsigned int i = 0; i < needdates.size(); i++)
			{
				unsigned int pos;
				if (!LAAlgorithm::find<DateVector, LADate>(paymentdates, needdates[i], 0, paymentdates.size() - 1, pos))
				{
					//error
					LAString msg = "Past Coupon at ";
					msg += LADataDate(needdates[i]).convertToString();
					msg += " is needed.";
					throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
				}
				mFixedRates.push_back(couponcfs.at(pos));
			}
		}
	}	
	
	
/*	// past coupon
	dh = &(legs.get(legNo).getData(PRICING_DATA_PAYMENTDATES, NOCHECK)); 
	if (dh->isDefined() && !dh->isNull())
	{
		const DateVector& paymentdates = dynamic_cast<const LADataDates&>(dh->get()).get();
		dh = &(legs.get(mLegNo).getData(PRICING_DATA_COUPONPAYOFFS, ISNOTNULL)); 
		const DoubleArray& couponcfs = dynamic_cast<const LADataDoubles&>(dh->get()).get();
		for (unsigned int i = 0; i < paymentdates.size(); i++)
			if (paymentdates[i] >=observationstartdate && paymentdates[i] <= observationenddate)
				mFixedRates.push_back(couponcfs.at(i));
	}*/
}

/*!
	@brief set up cap and floor condition
	@param[in] trade trade object
	@param[in] legNo leg number(first leg number is 0)
	@param[in] fixingdate fixing date
	@param[in] payoff payoff
*/
void
LAPriceIndexToolCpnCF::setUpCapandFloor(const LAObject& trade,	 
				 unsigned int legNo, 
				 const LADate& fixingdate,
				 const LAPricePayOff& payoff)
{
	(void)payoff; //20070411--Nagase--Œxíœ‚ðgcc‚É‚à‘Î‰ž
	mLatestPos = -1;
	for (unsigned int i = 0; i < mPayOff->getPayOff()[mLegNo].size(); i++)
	{
		const LADate& paymentdate = mPayOff->getPayOff()[mLegNo][i].getPayOff().getPaymentDate();
		if (paymentdate <= fixingdate) 
			mLatestPos = i;
		else
			break;
	}
	//leg object
	const LADataHolder* dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const LADataMultiReference& legs = dynamic_cast<const LADataMultiReference&>(dh->get());
	// past coupon
	mFixedRatesforCapFloor.clear();
	dh = &(legs.get(legNo).getData(PRICING_DATA_PAYMENTDATES, NOCHECK)); 
	if (dh->isDefined() && !dh->isNull())
	{
		const DateVector& paymentdates = dynamic_cast<const LADataDates&>(dh->get()).get();
		dh = &(legs.get(mLegNo).getData(PRICING_DATA_COUPONPAYOFFS, ISNOTNULL)); 
		const DoubleArray& coupons = dynamic_cast<const LADataDoubles&>(dh->get()).get();
		for (unsigned int i = 0; i < paymentdates.size(); i++)
			if (paymentdates[i] <= fixingdate)
				mFixedRatesforCapFloor.push_back(coupons.at(i));
	}
	mX2.clear();
	mX2.resize(mFixedRatesforCapFloor.size() + mLatestPos + 1 + 1);

}










LAPriceIndexToolCompound::LAPriceIndexToolCompound() : LAPriceIndexTool(), mCompoundTerms(), mMargins() {}
LAPriceIndexToolCompound::~LAPriceIndexToolCompound() {}

LAPriceIndexToolBase* LAPriceIndexToolCompound::clone() const
{
    try{
        return new LAPriceIndexToolCompound(*this);
    } catch (bad_alloc & e) {
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }	
}

void LAPriceIndexToolCompound::setUpFixingDates(const LAObject& indexInfo)
{	
    const LADataHolder* dh;
    dh = &indexInfo.getData(PRICING_DATA_FIXINGDATES);
    if(dh->isDefined() && !dh->isNull()){
        fixingDates = dynamic_cast<const LADataDates&>(dh->get()).get();
        return;
    }




	const LADate& start  = dynamic_cast<const LADataDate&>(indexInfo.getData(PRICING_DATA_OBSERVATIONSTARTDATE, ISNOTNULL).get());
	const LADate& end    = dynamic_cast<const LADataDate&>(indexInfo.getData(PRICING_DATA_OBSERVATIONENDDATE, ISNOTNULL).get());
	const LAString& freq = dynamic_cast<const LADataString&>(indexInfo.getData(PRICING_DATA_OBSERVATIONFREQUENCY, ISNOTNULL).get());




    const int* pday = NULL;
    int temp_day;
	dh = &indexInfo.getData(PRICING_DATA_OBSERVATIONDAY); 
    if (dh->isDefined() && !dh->isNull()){
        temp_day = dynamic_cast<const LADataInt&>(dh->get()).get();
        pday = &temp_day;
    }
	
	const LAPriceDataSlidingRule* psrule;
	const LAPriceDataCalendar* pcal ;
	LAPriceCFGenUtility::getBusDayRuleAndCalendar(indexInfo, 
                                               PRICING_DATA_OBSERVATIONSLIDINGRULE,
                                               PRICING_DATA_OBSERVATIONCALENDAR,
                                               indexInfo,
                                               CALIBRATION_DATA_SLIDINGRULE,
                                               CALIBRATION_DATA_CALENDAR,
                                               psrule, 
                                               pcal);

	
    fixingDates.clear();
	LAMathDateCalculations::generateSchedule(start, 
                                   end, 
                                   freq,	
                                   false,
                                   NULL, 
                                   NULL,
                                   pday,
                                   fixingDates,
                                   psrule,
                                   pcal);

    const LADate tmpDate = pcal != NULL ? psrule->getDate(end, *pcal) : end;
	if (fixingDates.back() != tmpDate) fixingDates.push_back(tmpDate);
}







/*!
	@brief set up this class
	@param[in] basedate basedate
	@param[in] paydate paymentdate
	@param[in] trade trade object
	@param[in] indexinfo index information object
	@param[in] payoff LAPricePayOff object
*/
void
LAPriceIndexToolCompound::setUp(const LADate& basedate,
                                  const LADate& paydate,
                                  const LAObject& trade,
                                  const LAObject& indexinfo,
                                  const LAPricePayOff& payoff)
{

	LAPriceIndexToolBase::setUp(basedate, paydate, trade, indexinfo, payoff);

	mX.clear();
	mpObservationOperator = NULL;
	mpIndex = NULL;
	mCompoundTerms.clear();
	mMargins.clear();





	const LADataHolder* dh;
	dh = &(indexinfo.getData(PRICING_DATA_INDEXENTITY, ISNOTNULL));
	const LADataReference& ref = dynamic_cast<const LADataReference&>(dh->get());
	mpIndex = &dynamic_cast<const LAMathIndexEntity&>(ref.get().get());
	const DateVector& grid = mpIndex->getGrid();





	
    flagForCompoundAllDays = (dh = &indexinfo.getData(PRICING_DATA_COMPOUND_ON_ALL_DAYS))->isDefined() && !dh->isNull() ? dynamic_cast<const LADataBool&>(dh->get()).get() : false;
    setUpFixingDates(indexinfo);
    setUpStartAndEndDates(indexinfo);
	setUpIndexPosAndFixedRate(basedate, indexinfo, trade);






    




	


	
	unsigned int pos;
	const bool isNeedPastRate = fixingDates[0] < basedate;
	const bool isSavePFixing = isSavePastFixing(trade);



	

    

	
	if(flagForCompoundAllDays){
		unsigned int comp_size = 0;
		for(unsigned int i = 0; i < fixingDates.size(); i++){
			comp_size += cfCalcStartDates[i].intervalDays(cfCalcEndDates[i]);
		}
		mCompoundTerms.resize(comp_size);
		unsigned int temp_pos = 0;
		for(unsigned int i = 0; i < fixingDates.size(); i++){
			LADate temp_start, temp_end;
			temp_start = temp_end = cfCalcStartDates[i];
			temp_end.addDays(1);
			while(temp_start != cfCalcEndDates[i]){
				mCompoundTerms.at(temp_pos++) = mCompoundDC.getTerm(temp_start, temp_end, false);
				temp_start.addDays(1);
				temp_end.addDays(1);
			}
		}
	}
	else{
		unsigned int outSize = fixingDates.size();
		mCompoundTerms.resize(outSize);
		for (unsigned int j = 0; j < outSize; j++)
		{
			mCompoundTerms[j] = mCompoundDC.getTerm(cfCalcStartDates[j],cfCalcEndDates[j],false);
		}
	}
	unsigned int chksize = mFixedRates.size() + mIndexPos.size();
	if (chksize != mCompoundTerms.size())
		throw LACoreInvalidData("Compounding size error",__FILE__,__LINE__);
	
	//add margin rate
	double basemargin = 0.0;
	dh = &(indexinfo.getData(PRICING_DATA_COMPOUNDINGMARGIN, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		basemargin = dynamic_cast<const LADataDouble &>(dh->get()).get();
	
	mMargins.resize(chksize,basemargin);
	//chk compounding margins
	dh = &(indexinfo.getData(PRICING_DATA_COMPOUNDINGDATES, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		const DateVector& compdates = dynamic_cast<const LADataDates &>(dh->get()).get();
		dh = &(indexinfo.getData(PRICING_DATA_COMPOUNDINGMARGINS, ISNOTNULL));
		const DoubleVector& compmargins = dynamic_cast<const LADataDoubles &>(dh->get()).get();

		if (compdates.size() != compmargins.size())
			throw LACoreInvalidData("Compounding dates size error",__FILE__,__LINE__);

		for (unsigned int i = 0; i < compdates.size(); i++)
		{
			if (!LAAlgorithm::find<DateVector, LADate>(fixingDates, compdates[i], 0, fixingDates.size() - 1, pos))
			{
				//error
				LAString msg = "Compounding dates error";
				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
			mMargins[pos] = compmargins[i];
		}
	}

	//chk compounding past margins
	//past rate
	if (isNeedPastRate)
	{
		dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONMARGINS, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			const DoubleArray& observationmargins = dynamic_cast<const LADataDoubles&>(dh->get()).get();
			dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONDATES, ISNOTNULL));
			const DateVector& observationdates = dynamic_cast<const LADataDates&>(dh->get()).get();
			if (observationdates.size() != observationmargins.size())
			{
				throw LACoreInvalidData("ObservationDates size and ObservationMargins size are not same.", __FILE__, __LINE__);
			}
			for (unsigned int i = 0; i < fixingDates.size(); i++)
			{
				if (basedate > fixingDates[i]) //past
				{
					if (!LAAlgorithm::find<DateVector, LADate>(observationdates, fixingDates[i], 0, observationdates.size() - 1, pos))
					{
						if (!isSavePFixing || (!observationdates.empty() && observationdates.back() > fixingDates[i]))
						{
							//error
							LAString msg = "Index rate at ";
							msg += LADataDate(fixingDates[i]).convertToString();
							msg += " is needed.";
							throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
						}
					}
					mMargins[i]= observationmargins[pos];
				}
				else
				{
					break;
				}
			}
		}			
	}

	//cap
	mIsCap = false;
	dh = &(indexinfo.getData(PRICING_DATA_MAXINDEX, NOCHECK));	 
	if (dh->isDefined() && !dh->isNull())
	{
		const LAPriceDataFunction& method = dynamic_cast<const LAPriceDataFunction&>(dh->get());
		if (!method.isTypeOf(FN_CONSTANT))
		{
			//error
			LAString msg = "Only LAConstant function is available";
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);		
		}

		mCap = dynamic_cast<const LAConstant&>(method.getFunction()).getParam().at(0);
		mIsCap = true;
	}
	//floor
	mIsFloor = false;
	dh = &(indexinfo.getData(PRICING_DATA_MININDEX, NOCHECK));	 
	if (dh->isDefined() && !dh->isNull())
	{
		const LAPriceDataFunction& method = dynamic_cast<const LAPriceDataFunction&>(dh->get());
		if (!method.isTypeOf(FN_CONSTANT))
		{
			//error
			LAString msg = "Only LAConstant function is available";
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);		
		}

		mFloor = dynamic_cast<const LAConstant&>(method.getFunction()).getParam().at(0);
		mIsFloor = true;	
	}

	mX.resize(chksize * 3);
}


/*!
	@brief set up this class
	@param[in] basedate basedate
	@param[in] trade trade object
	@param[in] indexinfo index information object
	@param[in] fixingdate fixingdate
	@param[in] payoff LAPricePayOff object
*/
void
LAPriceIndexToolCompound::setUp(const LADate& basedate,	
						const LAObject& trade,
						const LAObject& indexinfo,
						const LADate& fixingdate,
						const LAPricePayOff& payoff)
{
	
}

void 
LAPriceIndexToolCompound::setUpIndexPosAndFixedRate(const LADate& basedate,
													  const LAObject& indexInfo,
													  const LAObject& trade)
{
    const LADataHolder* dh;

    const DateVector& grid = mpIndex->getGrid();

	unsigned int pos;
	bool isNeedPastRate = false;
	bool isBaseContain = false;
	bool IsPastRateCompounded = false;
	for (unsigned int i = 0; i < fixingDates.size(); i++)
	{
        const unsigned int pushed_num = flagForCompoundAllDays ? cfCalcStartDates[i].intervalDays(cfCalcEndDates[i]) : 1;
		
		if (basedate <= fixingDates[i]) //future
		{
			if (!LAAlgorithm::find<DateVector, LADate>(grid, fixingDates[i], 0, grid.size() - 1, pos))
			{
				//error
				LAString msg = "LAMathIndexEntity grid is inconsistent with index observation dates";
				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}

			if (basedate == fixingDates[i])
			{
				isBaseContain = true;
			}
            for(unsigned int j = 0; j < pushed_num; j++) mIndexPos.push_back(pos);
		}
		else
		{
			isNeedPastRate = true;
		}
	}	

	dh = &(indexInfo.getData(PRICING_DATA_OBSERVATIONRATESTYPE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		LAString ortype = dynamic_cast<const LADataString&>(dh->get()).get();
		if (ortype == "Compounded")
		{
			IsPastRateCompounded = true;
		}
		else
		{
			//error
			LAString msg = "ObservationRatesType ";
			msg += LADataString(ortype).convertToString();
			msg += " is not defined.";
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
	}


	//past rate
	const bool isSavePFixing = isSavePastFixing(trade);
	if (isNeedPastRate && !IsPastRateCompounded)
	{

		if (isSavePFixing)
		{
			DateVector observationdates;
			DoubleArray observationrates;
			dh = &(indexInfo.getData(PRICING_DATA_OBSERVATIONDATES, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				observationdates = dynamic_cast<const LADataDates&>(dh->get()).get();
				dh = &(indexInfo.getData(PRICING_DATA_OBSERVATIONRATES, ISNOTNULL));
				observationrates = dynamic_cast<const LADataDoubles&>(dh->get()).get();
			}
			for (unsigned int i = 0; i < fixingDates.size(); i++)
			{
                const unsigned int pushed_num = flagForCompoundAllDays ? cfCalcStartDates[i].intervalDays(cfCalcEndDates[i]) : 1;

				if (basedate > fixingDates[i]) //past
				{
					if (!LAAlgorithm::find<DateVector, LADate>(observationdates, fixingDates[i], 0, observationdates.size() - 1, pos))
					{
						if (!observationdates.empty() && observationdates.back() > fixingDates[i])
						{
							//error
							LAString msg = "Index rate at ";
							msg += LADataDate(fixingDates[i]).convertToString();
							msg += " is needed.";
							throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
						}
						for(unsigned int j = 0; j < pushed_num; j++) mIndexPos.insert(mIndexPos.begin(), 0);
					}
					else
					{
						for(unsigned int j = 0; j < pushed_num; j++) mFixedRates.push_back(observationrates.at(pos));
					}
				}
				else
				{
					break;
				}
			}

		}
		else
		{
			dh = &(indexInfo.getData(PRICING_DATA_OBSERVATIONDATES, ISNOTNULL));
			const DateVector& observationdates = dynamic_cast<const LADataDates&>(dh->get()).get();
			dh = &(indexInfo.getData(PRICING_DATA_OBSERVATIONRATES, ISNOTNULL));
			const DoubleArray& observationrates = dynamic_cast<const LADataDoubles&>(dh->get()).get();
			if (observationdates.size() != observationrates.size())
			{
				throw LACoreInvalidData("ObservationDates size and ObservationRates size are not same.", __FILE__, __LINE__);
			}
			for (unsigned int i = 0; i < fixingDates.size(); i++)
			{
                const unsigned int pushed_num = flagForCompoundAllDays ? cfCalcStartDates[i].intervalDays(cfCalcEndDates[i]) : 1;
				if (basedate > fixingDates[i]) //past
				{
					if (!LAAlgorithm::find<DateVector, LADate>(observationdates, fixingDates[i], 0, observationdates.size() - 1, pos))
					{
						//error
						LAString msg = "Index rate at ";
						msg += LADataDate(fixingDates[i]).convertToString();
						msg += " is needed.";
						throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
					}
					for(unsigned int j = 0; j < pushed_num; j++) mFixedRates.push_back(observationrates.at(pos));
				}
				else
				{
					break;
				}
			}
		}	
	}
	else if (isNeedPastRate && IsPastRateCompounded)
	{
		if (isSavePFixing)
		{
			DateVector observationdates;
			DoubleArray observationrates;
			dh = &(indexInfo.getData(PRICING_DATA_OBSERVATIONDATES, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				observationdates = dynamic_cast<const LADataDates&>(dh->get()).get();
				dh = &(indexInfo.getData(PRICING_DATA_OBSERVATIONRATES, ISNOTNULL));
				observationrates = dynamic_cast<const LADataDoubles&>(dh->get()).get();

				//check the size of observation dates and observation rates
				if (observationdates.size() != 1 || observationrates.size() != 1)
				{
					//error
					LAString msg = "observationdates.size() != 1 || observationrates.size() != 1";
					throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
				}
			}

			for (unsigned int i = 0; i < fixingDates.size(); i++)
			{
				const unsigned int pushed_num = flagForCompoundAllDays ? cfCalcStartDates[i].intervalDays(cfCalcEndDates[i]) : 1;

				if (basedate > fixingDates[i]) //past
				{
					if (!LAAlgorithm::find<DateVector, LADate>(observationdates, fixingDates[i], 0, observationdates.size() - 1, pos))
					{
						if (!observationdates.empty() && observationdates.back() > fixingDates[i])
						{
							mFixedRates.push_back(0);
							////error
							//LAString msg = "Index rate at ";
							//msg += LADataDate(fixingDates[i]).convertToString();
							//msg += " is needed.";
							//throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
						}
						else
						{
							for (unsigned int j = 0; j < pushed_num; j++) mIndexPos.insert(mIndexPos.begin(), 0);
						}
					}
					else
					{
						const LAPriceDataDayCount& dc = dynamic_cast<const LAPriceDataDayCount&>(indexInfo.getData(PRICING_DATA_COMPOUNDINGDAYCOUNT, ISNOTNULL).get());
						double compoundingterm = dc.getTerm(cfCalcStartDates[i], cfCalcEndDates[i], false);
						if (compoundingterm < DBL_MIN)
						{
							//error
							LAString msg = "compoundingterm < DBL_MIN";
							throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
						}
						for (unsigned int j = 0; j < pushed_num - 1; j++) mFixedRates.push_back(0);
						mFixedRates.push_back(observationrates.at(pos) / compoundingterm);
					}
				}
				else
				{
					break;
				}
			}
		}
		else
		{
			dh = &(indexInfo.getData(PRICING_DATA_OBSERVATIONDATES, ISNOTNULL));
			const DateVector& observationdates = dynamic_cast<const LADataDates&>(dh->get()).get();
			dh = &(indexInfo.getData(PRICING_DATA_OBSERVATIONRATES, ISNOTNULL));
			const DoubleArray& observationrates = dynamic_cast<const LADataDoubles&>(dh->get()).get();
			if (observationdates.size() != 1 || observationrates.size() != 1)
			{
				throw LACoreInvalidData("ObservationDates size or ObservationRates size are not one.", __FILE__, __LINE__);
			}
			for (unsigned int i = 0; i < fixingDates.size(); i++)
			{
				const unsigned int pushed_num = flagForCompoundAllDays ? cfCalcStartDates[i].intervalDays(cfCalcEndDates[i]) : 1;
				if (basedate > fixingDates[i]) //past
				{
					if (!LAAlgorithm::find<DateVector, LADate>(observationdates, fixingDates[i], 0, observationdates.size() - 1, pos))
					{
						mFixedRates.push_back(0);
						////error
						//LAString msg = "Index rate at ";
						//msg += LADataDate(fixingDates[i]).convertToString();
						//msg += " is needed.";
						//throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
					}
					else 
					{
						const LAPriceDataDayCount& dc = dynamic_cast<const LAPriceDataDayCount&>(indexInfo.getData(PRICING_DATA_COMPOUNDINGDAYCOUNT, ISNOTNULL).get());
						double compoundingterm = dc.getTerm(cfCalcStartDates[i], cfCalcEndDates[i], false);
						if (compoundingterm < DBL_MIN)
						{
							//error
							LAString msg = "compoundingterm < DBL_MIN";
							throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
						}
						for (unsigned int j = 0; j < pushed_num - 1; j++) mFixedRates.push_back(0);
						mFixedRates.push_back(observationrates.at(pos) / compoundingterm);
					}
				}
				else
				{
					break;
				}
			}
		}
	}
	
	// basedate rate
	if (isBaseContain && !IsPastRateCompounded)
	{
		dh = &(indexInfo.getData(PRICING_DATA_OBSERVATIONDATES, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			const DateVector& observationdates = dynamic_cast<const LADataDates&>(dh->get()).get();
			if (LAAlgorithm::find<DateVector, LADate>(observationdates, basedate, 0, observationdates.size() - 1, pos))
			{
				dh = &(indexInfo.getData(PRICING_DATA_OBSERVATIONRATES, ISNOTNULL));
				const DoubleArray& observationrates = dynamic_cast<const LADataDoubles&>(dh->get()).get();
				if (observationdates.size() != observationrates.size())
				{
					throw LACoreInvalidData("ObservationDates size and ObservationRates size are not same.", __FILE__, __LINE__);
				}

                unsigned int pos2;
                LAAlgorithm::find(fixingDates, basedate, 0, fixingDates.size()-1, pos2);
                const unsigned int pushed_num = flagForCompoundAllDays ? cfCalcStartDates[pos2].intervalDays(cfCalcEndDates[pos2]) : 1;
                for(unsigned int i = 0; i < pushed_num; i++){
                    mFixedRates.push_back(observationrates.at(pos));
                    mIndexPos.erase(mIndexPos.begin());
                }
			}
		}
	}
	else if (isBaseContain && IsPastRateCompounded)
	{
		dh = &(indexInfo.getData(PRICING_DATA_OBSERVATIONDATES, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			const DateVector& observationdates = dynamic_cast<const LADataDates&>(dh->get()).get();
			if (LAAlgorithm::find<DateVector, LADate>(observationdates, basedate, 0, observationdates.size() - 1, pos))
			{
				dh = &(indexInfo.getData(PRICING_DATA_OBSERVATIONRATES, ISNOTNULL));
				const DoubleArray& observationrates = dynamic_cast<const LADataDoubles&>(dh->get()).get();
				if (observationdates.size() != 1 || observationrates.size() != 1)
				{
					throw LACoreInvalidData("ObservationDates size or ObservationRates size are not one.", __FILE__, __LINE__);
				}

				unsigned int pos2;
				LAAlgorithm::find(fixingDates, basedate, 0, fixingDates.size() - 1, pos2);
				const unsigned int pushed_num = flagForCompoundAllDays ? cfCalcStartDates[pos2].intervalDays(cfCalcEndDates[pos2]) : 1;
				for (unsigned int i = 0; i < pushed_num - 1; i++) {
					mFixedRates.push_back(0);
					mIndexPos.erase(mIndexPos.begin());
				}
				const LAPriceDataDayCount& dc = dynamic_cast<const LAPriceDataDayCount&>(indexInfo.getData(PRICING_DATA_COMPOUNDINGDAYCOUNT, ISNOTNULL).get());
				double compoundingterm = dc.getTerm(cfCalcStartDates[pos2], cfCalcEndDates[pos2], false);
				if (compoundingterm < DBL_MIN)
				{
					//error
					LAString msg = "compoundingterm < DBL_MIN";
					throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
				}
				mFixedRates.push_back(observationrates.at(pos) / compoundingterm);
				mIndexPos.erase(mIndexPos.begin());
			}
		}
	}
}

void 
LAPriceIndexToolCompound::setFixingInfo(DateVector& fixing_date, LAStringVector& fixing_flag) const
{
	if(flagForCompoundAllDays){
		for(size_t i = 0; i < fixingDates.size(); i++){
			for(LADate d = cfCalcStartDates[i]; d < cfCalcEndDates[i]; d.addDays(1)){
				fixing_date.push_back(fixingDates[i]);
			}
		}
	}
	else{
        for(DateVector::const_iterator it = fixingDates.begin(); it != fixingDates.end(); ++it){ fixing_date.push_back(*it); }
	}
	
    for(DoubleVector::const_iterator it = mFixedRates.begin(); it != mFixedRates.end(); ++it){ fixing_flag.push_back("Y"); }
	for(size_t i = 0; i < mIndexPos.size(); i++){ fixing_flag.push_back("N"); }
}

void LAPriceIndexToolCompound::setUpStartAndEndDates(const LAObject& indexInfo) 
{
    const LADataHolder* dh;
    
    
    
    
    dh = &indexInfo.getData(PRICING_DATA_CFCALCSTARTDATES);
    if(dh->isDefined() && !dh->isNull()){
        cfCalcStartDates = dynamic_cast<const LADataDates&>(dh->get()).get();
        try{
            dh = &indexInfo.getData(PRICING_DATA_CFCALCENDDATES, ISNOTNULL);
            cfCalcEndDates = dynamic_cast<const LADataDates&>(dh->get()).get();
        } catch(LACoreInvalidData& e){
            stringstream sst;
            sst << "There is CFCalcStartDates but is not CFCalcEndDates." << endl;
            e.addMsg(sst.str().c_str());
            throw e;
        }

        if(fixingDates.size() != cfCalcStartDates.size() ||
            fixingDates.size() != cfCalcEndDates.size()){
                throw LACoreInvalidData("FixingDates and CFCalcStart/EndDates sizes are inconsistent", __FILE__, __LINE__);
        }
        return;
    }




    cfCalcStartDates = fixingDates;
    cfCalcEndDates.resize(fixingDates.size());
    copy(++fixingDates.begin(), fixingDates.end(), cfCalcEndDates.begin());



	const LAString& acstr = dynamic_cast<const LADataString&>(indexInfo.getData(PRICING_DATA_ACCESSORY, ISNOTNULL).get()).get();
	const LAPriceDataSlidingRule* psrule;
	const LAPriceDataCalendar* pcal ;
	LAPriceCFGenUtility::getBusDayRuleAndCalendar(indexInfo, 
                                               PRICING_DATA_OBSERVATIONSLIDINGRULE,
                                               PRICING_DATA_OBSERVATIONCALENDAR,
                                               indexInfo,
                                               CALIBRATION_DATA_SLIDINGRULE,
                                               CALIBRATION_DATA_CALENDAR,
                                               psrule, pcal);
	const LADate finaldate = LAMathDateCalculations::getDate(fixingDates.back(), acstr, *psrule, pcal, true);
    cfCalcEndDates.back() = finaldate;
}

void LAPriceIndexToolCompound::calcIndices(DoubleVector& indices, const size_t start_pos, const size_t end_pos) const
{
    if(end_pos-start_pos != mFixedRates.size() + mIndexPos.size()) throw LACoreInvalidData("compound term size is inconsistent.", __FILE__, __LINE__);

    size_t pos = start_pos;
    for(size_t i = 0; i < mFixedRates.size(); i++) indices[pos++] = mFixedRates[i];
    for(size_t i = 0; i < mIndexPos.size(); i++) indices[pos++] = mpIndex->getIndex()[mIndexPos[i]];

    pos = start_pos;
    for(size_t i = 0; i < mFixedRates.size() + mIndexPos.size(); i++){
        round(indices[pos]);
        discountadjust(indices[pos]);
        capfloor(indices[pos]);
        pos++;
    }
}

void
LAPriceIndexToolCompound::setFixingInfo(LADate& fixing_date, LAString& fixing_flag) const
{
    fixing_date = fixingDates.back();
    fixing_flag = mFixedRates.size() == mCompoundTerms.size() ? "Y" : "N";
}
