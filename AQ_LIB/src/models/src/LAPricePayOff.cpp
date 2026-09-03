/*! @file
    @brief source code of payoff calculator class for mc simulation.

*/
//  2006, Mizuho International London..
///
#ifdef __GNUG__
#pragma implementation
#endif
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include "LAPricePayOff.h"

#include "LADataHolder.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataMatrix.h"
#include "LADataProcedure.h"
#include "LADataReference.h"
#include "LADataMultiReference.h"
#include "LAPriceDataManager.h"
#include "LAObject.h"
#include "LAObjectHolder.h"
#include "LAObjectPool.h"
#include "LADataInstance.h"


#include "LAAlgorithm.h"

#include "LAMathDateCalculations.h"
#include "LAPriceDataCalendar.h"
#include "LAPriceDataSlidingRule.h"
#include "LAPriceDataFunction.h"
#include "LAPriceDataDayCount.h"
#include "LAPriceDataInterpolation.h"
#include "LAMathIndexEntity.h"
#include "LAMathPathEntity.h"
#include "LAMathDefine.h"
#include "LAPriceCFGenUtility.h"
#include "LAPriceEventBase.h"
#include "LAPriceEventTool.h"
#include "LAPriceIndexTool.h"
#ifndef VISUAL_STUDIO_2010_ANALYTICS
#include "LAPricePayOffToolRangeAccrue.h"
#include "LAMathIndexEntityInterpolation.h"
#endif
#include <algorithm>


using namespace std;



#define LIBOR	"LIBOR"
#define CMS		"CMS"
#define FIXED_RATE	"FIXEDRATE"
#define FX_RATE	"FXRATE"
#define CMT	"CMT"
#define CUSTOM	"CUSTOM"
#define CPN		"CPN"
#define CPNCF	"CPNCF"
#define OIS "OIS"



/*!
    @brief default constructor
*/
LAPricePayOff::LAPricePayOff()
: LACoreFunctionBase(), mIsLSMC(false), mIsTrigger(false)
{

}
/*!
    @brief destructor

*/
LAPricePayOff::~LAPricePayOff()
{
	for (unsigned int i = 0; i < mTriggers.size(); i++)
		delete mTriggers[i];
	mTriggers.clear();
}

/*!
    @brief copy constructor

	@param[in] v LAPricePayOff object
*/
LAPricePayOff::LAPricePayOff(const LAPricePayOff& v)
: LACoreFunctionBase(v), /*mIsLSMC(v.mIsLSMC), */mPayOffMaster(v.mPayOffMaster)
, mPayOff(v.mPayOff), mTimes(v.mTimes), mIsTrigger(v.mIsTrigger)
{
	mTriggers.resize(v.mTriggers.size());
	for (unsigned int i = 0; i < mTriggers.size(); i++)
	{
#ifndef VISUAL_STUDIO_2010_ANALYTICS
		try 
		{		
			mTriggers[i] = new LAPriceEventTool(*v.mTriggers[i]);
		}
		catch (bad_alloc & e)
		{
			throw LACoreSystemError(e.what(), __FILE__, __LINE__);
		}
#endif
	}

	for (unsigned int i = 0; i < mTriggers.size(); i++)
		mActions.insert(mActions.end(),	mTriggers[i]->getAction().begin(), 	mTriggers[i]->getAction().end());

	sort(mActions.begin(), mActions.end(), Comp_pTriggerActionHolder());

}




/*
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LAPricePayOff::isTypeOf(function_t id) const
{
	return (id == FN_IR_PAYOFF ? true : LACoreFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this class type
    @return function type
*/
function_t			
LAPricePayOff::getType() const
{
	return FN_IR_PAYOFF;
}

/*
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*
LAPricePayOff::clone() const    
{
    try 
	{
        return new LAPricePayOff(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }	
}
/*!
	@brief register dataValues that this class uses

	@param[in, out] dm data master 
*/
void
LAPricePayOff::registerData(LAPriceDataManager& dm) const
{
	dm.setData(IR_CALIBRATION_DATA_DAYCOUNT, DATA_DAYCOUNT);
	dm.setData(PRICING_DATA_CFGENERATOR, DATA_PROCEDURE);
	dm.setData(PRICING_DATA_CALLINFO, DATA_REFERENCE);
	dm.setData(PRICING_DATA_TRIGGERINFOS, DATA_MULTIREFERENCE);
	dm.setData(PRICING_DATA_CASHLETS, DATA_MULTIREFERENCE);
	dm.setData(PRICING_DATA_PAYMENTDATE, DATA_DATE);
	dm.setData(PRICING_DATA_ENDDATE, DATA_DATE);
	dm.setData(PRICING_DATA_ACTIONDATES, DATA_DATES);
	dm.setData(PRICING_DATA_EXPIRYDATES, DATA_DATES);
	dm.setData(PRICING_DATA_EXTRACF, DATA_DOUBLE);
	dm.setData(PRICING_DATA_EXTRACFS, DATA_DOUBLES);
	dm.setData(PRICING_DATA_EXTRACFDATES, DATA_DATES);
	dm.setData(PRICING_DATA_EXTRACFINDEXINFOS, DATA_MULTIREFERENCE);
	dm.setData(PRICING_DATA_COUPONINFOS, DATA_MULTIREFERENCE);
	dm.setData(PRICING_DATA_INDEXINFOS, DATA_MULTIREFERENCE);
	dm.setData(PRICING_DATA_FRAINDEXINFO, DATA_REFERENCE);
	dm.setData(PRICING_DATA_INDEXENTITY, DATA_REFERENCE);
	dm.setData(PRICING_DATA_TARGETLEG, DATA_STRING);
	dm.setData(PRICING_DATA_TRIGGERTARGETS, DATA_STRINGS);
	dm.setData(PRICING_DATA_INDEXTYPE, DATA_STRING);
	dm.setData(PRICING_DATA_INDEXNAME, DATA_STRING);
	dm.setData(PRICING_DATA_LSMCVARIABLES, DATA_STRINGS);
	dm.setData(PRICING_DATA_FIXINGDATE, DATA_DATE);
	dm.setData(PRICING_DATA_OBSERVATIONSTARTDATE, DATA_DATE);
	dm.setData(PRICING_DATA_OBSERVATIONENDDATE, DATA_DATE);
	dm.setData(PRICING_DATA_OBSERVATIONSLIDINGRULE, DATA_SLIDINGRULE);
	dm.setData(PRICING_DATA_OBSERVATIONCALENDAR, DATA_CALENDAR);
	dm.setData(PRICING_DATA_OBSERVATIONFREQUENCY, DATA_STRING);
	dm.setData(PRICING_DATA_OBSERVATIONDAY, DATA_INT);
	dm.setData(PRICING_DATA_OBSERVATIONSTARTTERM, DATA_STRING);
	dm.setData(PRICING_DATA_OBSERVATIONENDTERM, DATA_STRING);
	dm.setData(PRICING_DATA_PATHENTITY, DATA_REFERENCE);
	dm.setData(PRICING_DATA_DAYCOUNT, DATA_DAYCOUNT);
	dm.setData(PRICING_CALIBRATION_DATAOTIONAL, DATA_DOUBLE);
	dm.setData(PRICING_DATA_CURRENCY, DATA_STRING);
	dm.setData(PRICING_DATA_EXTRACFCURRENCY, DATA_STRING);
	dm.setData(PRICING_DATA_EXTRACFCURRENCIES, DATA_STRINGS);
	dm.setData(PRICING_DATA_ISMULTIEXTRACF, DATA_BOOL);
	dm.setData(PRICING_DATA_FXRATE, DATA_REFERENCE);
	dm.setData(PRICING_DATA_EXTRACFFXRATE, DATA_REFERENCE);
	dm.setData(PRICING_DATA_CFCALCSTARTDATE, DATA_DATE);
	dm.setData(PRICING_DATA_CFCALCENDDATE, DATA_DATE);
	dm.setData(PRICING_DATA_COUPONSELECTOPERATOR, DATA_FUNCTION);
	dm.setData(PRICING_DATA_ROUNDFUNCTION, DATA_STRING);
	dm.setData(PRICING_DATA_ROUNDDIGIT, DATA_INT);
	dm.setData(PRICING_DATA_OBSERVATIONOPERATOR, DATA_FUNCTION);
	dm.setData(PRICING_DATA_FIXEDRATE, DATA_DOUBLE);
	dm.setData(PRICING_DATA_ACCESSORY, DATA_STRING);
	dm.setData(PRICING_DATA_TRIGGERTYPE, DATA_STRING);
	dm.setData(PRICING_DATA_OPERATOR, DATA_FUNCTION);
	dm.setData(PRICING_DATA_COEFFICIENT, DATA_DOUBLES);
	dm.setData(PRICING_DATA_COEFFICIENTS, DATA_DOUBLE_MATRIX);
	dm.setData(PRICING_DATA_OPTION, DATA_REFERENCE);
	dm.setData(PRICING_DATA_ISTERMINATE, DATA_BOOL);
	dm.setData(PRICING_DATA_COUPONCHANGEINFO, DATA_REFERENCE);
	dm.setData(PRICING_CALIBRATION_DATAOTIONALCHANGERATIO, DATA_DOUBLE);
	dm.setData(PRICING_DATA_MAXCOUPON, DATA_FUNCTION);
	dm.setData(PRICING_DATA_MINCOUPON, DATA_FUNCTION);
	dm.setData(PRICING_DATA_MAXINDEX, DATA_FUNCTION);
	dm.setData(PRICING_DATA_MININDEX, DATA_FUNCTION);
	dm.setData(PRICING_DATA_PAYMENTTIMING, DATA_STRING);
	dm.setData(PRICING_DATA_ISAMORTIZE, DATA_BOOL);
	dm.setData(PRICING_DATA_AMORTIZETYPE, DATA_STRING);
	dm.setData(PRICING_DATA_AMORTIZEAMOUNT, DATA_DOUBLE);
	dm.setData(PRICING_DATA_AMORTIZEROUNDFUNCTION, DATA_STRING);
	dm.setData(PRICING_DATA_AMORTIZEROUNDDIGIT, DATA_INT);
	dm.setData(PRICING_DATA_ISAMORTIZE1STFRACTION, DATA_BOOL);
	dm.setData(PRICING_DATA_ISNOTIONALEXCHANGEATEND, DATA_BOOL);
	dm.setData(PRICING_DATA_OBSERVATIONDATES, DATA_DATES);
	dm.setData(PRICING_DATA_OBSERVATIONRATES, DATA_DOUBLES);
	dm.setData(PRICING_DATA_OBSERVATIONRATESTYPE, DATA_STRING);
	dm.setData(PRICING_DATA_PAYMENTDATES, DATA_DATES);
	dm.setData(PRICING_DATA_COUPONS, DATA_DOUBLES);
	dm.setData(PRICING_DATA_COUPONPAYOFFS, DATA_DOUBLES);
	dm.setData(PRICING_DATA_ISACCRUAL, DATA_BOOL);
	dm.setData(PRICING_DATA_STARTDATE, DATA_DATE);
	dm.setData(CALIBRATION_DATA_UNDERLYINGS, DATA_MULTIREFERENCE);
	dm.setData(CALIBRATION_DATA_SLIDINGRULE, DATA_SLIDINGRULE);
	dm.setData(CALIBRATION_DATA_CALENDAR, DATA_CALENDAR);
	dm.setData(PRICING_DATA_DENOMINATOR, DATA_DOUBLE);
	dm.setData(PRICING_DATA_SELECTSIDE, DATA_STRING);
	dm.setData(PRICING_DATA_FIXEDDATES, DATA_DATES);
	dm.setData(PRICING_DATA_FIXEDRATES, DATA_DOUBLES);
	dm.setData(PRICING_CALIBRATION_DATAOTIONALCF, DATA_DOUBLE);
	dm.setData(PRICING_DATA_EXTRACFFUNC, DATA_FUNCTION);
	dm.setData(PRICING_DATA_EXTRACFFUNCINPUTS, DATA_STRINGS);
	dm.setData(PRICING_DATA_EXTRACFFUNCCOEFFICIENTS, DATA_DOUBLE_MATRIX);
	dm.setData(PRICING_DATA_EXTRACFFIXINGOFFSET, DATA_INT);
	dm.setData(PRICING_CALIBRATION_DATAOTIONALEXCHANGECURRENCY, DATA_STRING);
	dm.setData(PRICING_CALIBRATION_DATAOTIONALEXCHANGEFXRATE, DATA_REFERENCE);
	dm.setData(PRICING_DATA_FXRATEFIXINGDATE, DATA_DATE);
	dm.setData(PRICING_DATA_FXRATEFIXINGTERM, DATA_STRING);
	dm.setData(PRICING_DATA_FXRATEFIXINGSLIDINGRULE, DATA_SLIDINGRULE);
	dm.setData(PRICING_DATA_FXRATEFIXINGCALENDAR, DATA_CALENDAR);
	dm.setData(PRICING_CALIBRATION_DATAOTIONALCFS, DATA_DOUBLES);
	dm.setData(PRICING_DATA_FXRATESTRUCTUREFUNC, DATA_FUNCTION);
	dm.setData(PRICING_DATA_POLYNOMIAL, DATA_FUNCTION);
	dm.setData(PRICING_CALIBRATION_DATAOTIONALCFCOUPONINFOS, DATA_MULTIREFERENCE);
	dm.setData(PRICING_CALIBRATION_DATAOTIONALCFCOUPONSELECTOPERATOR, DATA_FUNCTION);
	dm.setData(CALIBRATION_DATA_FEEAMOUNTS, DATA_DOUBLES);
	dm.setData(CALIBRATION_DATA_FEEPAYMENTDATES, DATA_DATES);
	dm.setData(CALIBRATION_DATA_FEECURRENCIES, DATA_STRINGS);
	dm.setData(CALIBRATION_DATA_FEEDISCOUNTCURVES, DATA_STRINGS);
	
	//for range accrue
	dm.setData(PRICING_DATA_ISRANGEACCRUE, DATA_BOOL);
	dm.setData(PRICING_DATA_RANGEACCRUEINDEXINFOS, DATA_MULTIREFERENCE);
	dm.setData(PRICING_DATA_RANGEACCRUEOPERATOR, DATA_FUNCTION);
	dm.setData(PRICING_DATA_RANGEACCRUESTRIKE, DATA_DOUBLE);
	dm.setData(PRICING_DATA_RANGEACCRUECOEFFICIENT, DATA_DOUBLES);
	dm.setData(PRICING_DATA_RANGEACCRUEMAX, DATA_DOUBLE);
	dm.setData(PRICING_DATA_RANGEACCRUEMIN, DATA_DOUBLE);
	dm.setData(PRICING_DATA_RANGEACCRUESAMEOBSERVATIONDAYS, DATA_INT);
	dm.setData(PRICING_DATA_RANGEACCRUESAMEOBSERVATIONDATE,DATA_DATE);
	dm.setData(PRICING_DATA_RANGEACCRUEBOUNDARYINDEXINFOS,DATA_MULTIREFERENCE);
	dm.setData(PRICING_DATA_RANGEACCRUEMAXBOUNDARYOPERATOR,DATA_FUNCTION);
	dm.setData(PRICING_DATA_RANGEACCRUEMINBOUNDARYOPERATOR,DATA_FUNCTION);
	dm.setData(PRICING_DATA_RANGEACCRUEMAXBOUNDARYCOEFFICIENT,DATA_DOUBLES);
	dm.setData(PRICING_DATA_RANGEACCRUEMINBOUNDARYCOEFFICIENT,DATA_DOUBLES);
	dm.setData(PRICING_DATA_RANGEACCRUEOBSERVATIONSTARTDATE,DATA_DATE);
	dm.setData(PRICING_DATA_RANGEACCRUEOBSERVATIONENDDATE,DATA_DATE);
	dm.setData(PRICING_DATA_RANGEACCRUEINFOS, DATA_MULTIREFERENCE);
	dm.setData(PRICING_DATA_ISANDCONDITION, DATA_BOOL);
	dm.setData(PRICING_DATA_ISNOTCONDITION, DATA_BOOL);
	dm.setData(PRICING_DATA_ISEXCLUDEANDCONDITION, DATA_BOOL);
	dm.setData(PRICING_DATA_BUSINESSDAYSBASE, DATA_BOOL);
	dm.setData(PRICING_DATA_HITRATEOPERATOR, DATA_FUNCTION);
	dm.setData(PRICING_DATA_HITRATECOEFFICIENT, DATA_DOUBLES);
	dm.setData(PRICING_DATA_HITRATEMAX, DATA_DOUBLE);
	dm.setData(PRICING_DATA_HITRATEMIN, DATA_DOUBLE);

	dm.setData(PRICING_DATA_ROLLDAYFORINDEXGENERATE, DATA_INT);
	dm.setData(PRICING_DATA_SPOTLAG, DATA_INT);
	dm.setData(PRICING_DATA_DATESFORINDEXGENERATE, DATA_DATES);
	dm.setData(PRICING_DATA_ISLEGBASECURRENCY, DATA_BOOL);

	dm.setData(PRICING_DATA_COMPOUNDINGDAYCOUNT,	DATA_DAYCOUNT);
	dm.setData(PRICING_DATA_COMPOUNDINGMARGIN,		DATA_DOUBLE);
	dm.setData(PRICING_DATA_COMPOUNDINGMARGINS,	DATA_DOUBLES);
	dm.setData(PRICING_DATA_COMPOUNDINGDATES,		DATA_DATES);
	dm.setData(PRICING_DATA_OBSERVATIONMARGINS,	DATA_DOUBLES);
	dm.setData(PRICING_DATA_DISCOUNTSTARTDATE,		DATA_DATE);
	dm.setData(PRICING_DATA_DISCOUNTENDDATE,		DATA_DATE);

	dm.setData(PRICING_DATA_UPFRONTFEES,			DATA_DOUBLES);
	dm.setData(PRICING_DATA_UPFRONTPAYMENTDATES,	DATA_DATES);

	dm.setData(PRICING_DATA_FIXINGDATES,	DATA_DATES);
	dm.setData(PRICING_DATA_CFCALCSTARTDATES,	DATA_DATES);
	dm.setData(PRICING_DATA_CFCALCENDDATES,	DATA_DATES);

	dm.setData(PRICING_DATA_COMPOUND_ON_ALL_DAYS, DATA_BOOL);
	dm.setData(PRICING_DATA_ISCOMPOUNDINGCOUPON, DATA_BOOL);

    dm.setData(PRICING_DATA_FIRSTSTUBCOUPON, DATA_STRING);
    dm.setData(PRICING_DATA_LASTSTUBCOUPON, DATA_STRING);
    dm.setData(PRICING_DATA_COMPOUNDING_FUNCTION, DATA_FUNCTION);

	dm.setData(PRICING_DATA_FUNDINGSPREADENTITY, DATA_REFERENCE);

	dm.setData(PRICING_DATA_COUPONCFCURRENCY, DATA_STRING);
	dm.setData(PRICING_CALIBRATION_DATAOTIONALCFCURRENCY, DATA_STRING);
	dm.setData(PRICING_DATA_COUPONCFFXRATEVALUE, DATA_DOUBLE);
	dm.setData(PRICING_CALIBRATION_DATAOTIONALCFFXRATEVALUE, DATA_DOUBLE);

	dm.setData(PRICING_DATA_ISREDEMPTION, DATA_BOOL);
	dm.setData(PRICING_DATA_LEGNUMBER, DATA_INT);

	dm.setData(PRICING_DATA_SETTLEMENTCURRENCY, DATA_STRING);
	dm.setData(PRICING_DATA_SETTLEMENTFIXINGDATE, DATA_DATE);
	dm.setData(PRICING_DATA_SETTLEMENTADJUSTRATIO, DATA_DOUBLE);
	dm.setData(PRICING_DATA_SETTLEMENTFIXINGCALENDAR, DATA_CALENDAR);
	dm.setData(PRICING_DATA_SETTLEMENTFIXINGSLIDINGRULE, DATA_SLIDINGRULE);
	dm.setData(PRICING_DATA_SETTLEMENTFIXINGOFFSET, DATA_INT);
	dm.setData(PRICING_DATA_SETTLEMENTFIXINGDATES, DATA_DATES);
	dm.setData(PRICING_DATA_SETTLEMENTADJUSTRATIOS, DATA_DOUBLES);
}
/*!
	@brief calculate payoff

	@param[out] time payoff time
	@param[out] payoff payoff
	@param[in] trigger consider trigger and call or not
	@param[out] triggerhit trigger call hit information
	@param[in] includecall consider call or not
	@param[in] islsmc lsmc mode or not
	@param[out] prebate rebate of call (use for LSMC)
	@param[out] prebate2 rebate of call (use for LSMC)
	@param[out] pexplanatory explanatory  (use for LSMC)
	@param[out] extracf pos for payoff
*/
void
LAPricePayOff::calcPayOff(DoubleMatrix& time, DoubleMatrix& payoff, bool trigger, 
					   vector<pair<unsigned int, LADate> >& triggerhit,
					   /*bool includecall,*/
					   bool islsmc, DoubleArray* prebate, DoubleMatrix* pexplanatory, vector<UintArray>* extracfpos) const
{
	payoff.resize(mPayOffMaster.size());
	
	if (extracfpos)
	{
		extracfpos->clear();
		extracfpos->resize(mPayOffMaster.size());
	}

	if (!mIsTrigger || !trigger || mTriggers.size() == 0)
	{
		// non trigger case
		time = mTimes;
		for (unsigned int i = 0; i < mPayOffMaster.size(); i++)
		{
			payoff[i].resize(mPayOffMaster[i].size());
			for (unsigned int j = 0; j < payoff[i].size(); j++)
				payoff[i][j] = mPayOffMaster[i][j].getPayOff().calcPayOff();
		}
		return;
	}
	
	triggerhit.clear();
	time.resize(mPayOffMaster.size());

	///////////////////
	//case of trigger//
	///////////////////
	// prepare parameters
	vector<LAPriceEventHolder*>	futureaction = mActions;
	vector<PayOffToolHolderVector> extrapayoffmat(mPayOffMaster.size());
	mPayOff = mPayOffMaster;
	vector<LAPriceEventHolder*>  pastaction;

	vector<PayOffToolHolderIter> iter(mPayOff.size());
	for (unsigned int i = 0; i < mPayOff.size(); i++)
		iter[i] = mPayOff[i].begin();

	for (unsigned int i = 0; i < mTriggers.size(); i++)
		mTriggers[i]->setTriggerOn();

	

	// excecute action
	unsigned int count = 0;
	while(futureaction.size() > 0)
	{
		if (islsmc && futureaction[0]->isCall())
		{
#ifndef VISUAL_STUDIO_2010_ANALYTICS
			(*prebate)[count] = futureaction[0]->calcCallRebate(mPayOff, extrapayoffmat, futureaction, pastaction, iter);
			futureaction[0]->calcIndex((*pexplanatory)[count++]);
			futureaction.erase(futureaction.begin());
#endif
		}
		else if (/*!includecall && */futureaction[0]->isCall())
			futureaction.erase(futureaction.begin());
		else 
			futureaction[0]->doAction(mPayOff, extrapayoffmat, futureaction, pastaction, iter);
	}

	triggerhit.resize(pastaction.size());
	for (unsigned int i = 0; i < triggerhit.size(); i++)
	{	
		triggerhit[i].first = pastaction[i]->getID();
		triggerhit[i].second = pastaction[i]->getActionDates()[0];
	}
	// calc remain payoff
	for (unsigned int i = 0; i < mPayOff.size(); i++)
		for (unsigned int j = 0; j < mPayOff[i].size(); j++)
			mPayOff[i][j].getPayOff().calcPayOff();

	//output
	for (unsigned int i = 0; i < mPayOff.size(); i++)
	{
		unsigned int j;
		if (extrapayoffmat[i].size() == 0)// no extra cf case
		{
			time[i].resize(mPayOff[i].size());
			payoff[i].resize(mPayOff[i].size());
			for (j = 0; j < payoff[i].size(); j++)
			{
				time[i][j] = mTimes[i][j];
				payoff[i][j] = mPayOff[i][j].getPayOff().getPayOff();
			}
			continue;
		}
		
		//extra cf case
		vector<const LAPricePayOffTool*> payoffvec(mPayOff[i].size() + extrapayoffmat[i].size());
		map<const LAPricePayOffTool*, bool> exMap;
		for (j = 0; j < mPayOff[i].size(); j++)
		{
			payoffvec[j] = &mPayOff[i][j].getPayOff();
			exMap[payoffvec[j]] = false;
		}
		for (; j < payoffvec.size(); j++)
		{
			payoffvec[j] = &extrapayoffmat[i][j - mPayOff[i].size()].getPayOff();
			exMap[payoffvec[j]] = true;
		}
		sort(payoffvec.begin(), payoffvec.end(), Comp_pLAPricePayOffTool());
		
		time[i].resize(payoffvec.size());
		payoff[i].resize(payoffvec.size());
		
		if (extracfpos)
		{
			for (j = 0; j < payoffvec.size(); j++)
			{
				time[i][j] = payoffvec[j]->mPaymentTime;
				payoff[i][j] = payoffvec[j]->getPayOff();
				// check is extra cf
				if (exMap[payoffvec[j]])
				{
					(*extracfpos)[i].push_back(j);
				}
			}
		}
		else
		{
			for (j = 0; j < payoffvec.size(); j++)
			{
				time[i][j] = payoffvec[j]->mPaymentTime;
				payoff[i][j] = payoffvec[j]->getPayOff();
			}
		}

	}
	

}

/*!
	@brief calcDerivationOfLibor

*/
void
LAPricePayOff::calcDerivationOfLibor(std::map<LAString, std::map<double, double> >& delivationLiborMap, std::vector<const LAInterpolationBase*>& pNumeInterpVec, BoolVector& isRecVec) const
{
	for (unsigned int i = 0; i < mPayOffMaster.size(); i++)
		for (unsigned int j = 0; j < mPayOffMaster[i].size(); j++)
		{
			mPayOffMaster[i][j].getPayOff().calcDerivationOfLibor(delivationLiborMap, pNumeInterpVec[i], isRecVec[i]);
		}
	return;
}


/*!
	@brief calculate payoff

	@param[out] time1 payoff time (trigger considered if there are triggers)
	@param[out] payoff1 payoff (trigger considered if there are triggers)
	@param[out] time2 payoff time (not trigger considered)
	@param[out] payoff2 payoff (not trigger considered)
	@param[out] prebate rebate of call (use for LSMC)
	@param[out] prebate2 rebate of call (use for LSMC)


*/
/*void	
LAPricePayOff::calcPayOff(DoubleMatrix& time, DoubleMatrix& payoff,
					   DoubleMatrix& time2, DoubleMatrix& payoff2,
					   std::vector<std::pair<unsigned int, LADate> >& triggerhit,
					   bool islsmc, DoubleArray* prebate, DoubleMatrix* pexplanatory, vector<UintArray>* extracfpos) const
{
	calcPayOff(time, payoff, true, triggerhit, true, islsmc, prebate, pexplanatory, extracfpos);
	std::vector<std::pair<unsigned int, LADate> > dummy;
	calcPayOff(time2, payoff2, false, dummy, false);
}*/

/*!
	@brief set up for payoff calculation

	@param[in] basedate basedate
	@param[in, out] trade trade object 

*/
void	            
LAPricePayOff::setUp(const LADate& basedate, LAObject& trade) 
{
	clearIndexEntityGrid(trade);
	setUpIndexEntity(basedate, trade);
	
	setUpPayOff(basedate, trade);
	mPayOff = mPayOffMaster;

	setUpTrigger(basedate, trade);
}

/*!
	@brief set up for accrued payoff calculation

	@param[in] basedate basedate
	@param[in, out] trade trade object 
	@param[in] leg number
*/
void	            
LAPricePayOff::setUpAccrued(const LADate& basedate, LAObject& trade, int number) 
{
	clearIndexEntityGrid(trade);
	setUpIndexEntity(basedate, trade);
	
	setUpPayOffAccrued(basedate, trade, number);
	mPayOff = mPayOffMaster;

//	setUpTrigger(basedate, trade);
}

/*!
	@brief clear time grid of index object 

	
	@param[in, out] trade trade object 

*/
void
LAPricePayOff::clearIndexEntityGrid (LAObject& trade) const
{
	LADataHolder* dh;
	//leg object
	dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const LADataMultiReference& legs = dynamic_cast<LADataMultiReference&>(dh->get());
	for (unsigned int i = 0; i < legs.getSize(); i++)
	{
		//cashlets
		dh = &(legs.get(i).getData(PRICING_DATA_CASHLETS, NOCHECK));
		if (!dh->isDefined() || dh->isNull()) continue;
		LADataMultiReference& cashlets = dynamic_cast<LADataMultiReference&>(dh->get());
		
		for (unsigned int j = 0; j < cashlets.getSize(); j++)
		{
			//coupon info
			dh = &(cashlets.get(j).getData(PRICING_DATA_COUPONINFOS, NOCHECK));
			if (!dh->isDefined() || dh->isNull()) continue;
			LADataMultiReference& couponinfos = dynamic_cast<LADataMultiReference&>(dh->get());

			for (unsigned int k = 0; k < couponinfos.getSize(); k++)
			{
				//index info
				dh = &(couponinfos.get(k).getData(PRICING_DATA_INDEXINFOS, NOCHECK));
				if (!dh->isDefined() || dh->isNull()) continue;

				LADataMultiReference& indexinfos = dynamic_cast<LADataMultiReference&>(dh->get());
				for (unsigned int l = 0; l < indexinfos.getSize(); l++)
				{
					//index object
					dh = &(indexinfos.get(l).getData(PRICING_DATA_INDEXENTITY, NOCHECK));
					if (!dh->isDefined() || dh->isNull()) continue;
					LADataReference& ref = dynamic_cast<LADataReference&>(dh->get());
					LAMathIndexEntity& index = dynamic_cast<LAMathIndexEntity&>(ref.get().get());
					index.clearGrid();
				}
			}
		}
	}
	//trigger
	dh = &(trade.getData(PRICING_DATA_TRIGGERINFOS, NOCHECK));
	if (dh->isDefined() && !dh->isNull()) 
	{
		LADataMultiReference& triggers = dynamic_cast<LADataMultiReference&>(dh->get());
		for (unsigned int i = 0; i < triggers.getSize(); i++)
		{
			//index info(trigger reference)
			dh = &(triggers.get(i).getData(PRICING_DATA_INDEXINFOS, NOCHECK));
			if (dh->isDefined() && !dh->isNull()) 
			{
				LADataMultiReference& indexinfos = dynamic_cast<LADataMultiReference&>(dh->get());
				for (unsigned int j = 0; j < indexinfos.getSize(); j++)
					{
						//index object
						dh = &(indexinfos.get(j).getData(PRICING_DATA_INDEXENTITY, NOCHECK));
						if (!dh->isDefined() || dh->isNull()) continue;
						LADataReference& ref = dynamic_cast<LADataReference&>(dh->get());
						LAMathIndexEntity& index = dynamic_cast<LAMathIndexEntity&>(ref.get().get());
						index.clearGrid();
					}
			}
			dh = &(triggers.get(i).getData(PRICING_DATA_TRIGGERTARGETS, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				const LAStringVector& targets = dynamic_cast<const LADataStrings&>(dh->get()).get();	
				//object pool
				LAObjectPool& objPool = triggers.get(i).getDataInstance()->getObjectPool();
				for (unsigned int j = 0; j < targets.size(); j++)
				{
					LAObjectHolder objHolder = objPool.getObject(targets[j], ENCHKTYPE_NOCHECK);
					if (!objHolder.isDefined()) continue;
					dh = &(objHolder.getData(PRICING_DATA_INDEXTYPE, NOCHECK));
					if (!dh->isDefined() || dh->isNull()) continue;// not indexinfo
					dh = &(objHolder.getData(PRICING_DATA_INDEXENTITY, NOCHECK));
					if (!dh->isDefined() || dh->isNull()) continue;
					LADataReference& ref = dynamic_cast<LADataReference&>(dh->get());
					LAMathIndexEntity& index = dynamic_cast<LAMathIndexEntity&>(ref.get().get());
					index.clearGrid();
				}
			}			
			//cpn change case			
			//dh = &(triggers.get(i).getData(PRICING_DATA_COUPONCHANGEINFO, NOCHECK));	
			//if (!dh->isDefined() || dh->isNull()) continue;			
			
			for (unsigned int j = 0; j < legs.getSize(); j++)
			{				
				//coupon info
				dh = &(triggers.get(i).getData(PRICING_DATA_COUPONINFOS + LADataInt(j + 1).convertToString(), NOCHECK));
				if (!dh->isDefined() || dh->isNull()) continue;
				
				LADataMultiReference& couponinfos = dynamic_cast<LADataMultiReference&>(dh->get());
				for (unsigned int k = 0; k < couponinfos.getSize(); k++)
				{
					//index info
					dh = &(couponinfos.get(k).getData(PRICING_DATA_INDEXINFOS, NOCHECK));
					if (!dh->isDefined() || dh->isNull()) continue;

					LADataMultiReference& indexinfos = dynamic_cast<LADataMultiReference&>(dh->get());
					for (unsigned int l = 0; l < indexinfos.getSize(); l++)
					{
						//index object
						dh = &(indexinfos.get(l).getData(PRICING_DATA_INDEXENTITY, NOCHECK));
						if (!dh->isDefined() || dh->isNull()) continue;
						LADataReference& ref = dynamic_cast<LADataReference&>(dh->get());
						LAMathIndexEntity& index = dynamic_cast<LAMathIndexEntity&>(ref.get().get());
						index.clearGrid();
					}
				}
			}
		}
	}
	//call
	dh = &(trade.getData(PRICING_DATA_CALLINFO, NOCHECK));
	if (dh->isDefined() && !dh->isNull()) 
	{
		LADataReference& call = dynamic_cast<LADataReference&>(dh->get());	
		dh = &(call.get().getData(PRICING_DATA_INDEXINFOS, NOCHECK));
		if (dh->isDefined() && !dh->isNull()) 
		{
			LADataMultiReference& indexinfos = dynamic_cast<LADataMultiReference&>(dh->get());
			for (unsigned int i = 0; i < indexinfos.getSize(); i++)
			{
				//index object
				dh = &(indexinfos.get(i).getData(PRICING_DATA_INDEXENTITY, NOCHECK));
				if (!dh->isDefined() || dh->isNull()) continue;
				LADataReference& ref = dynamic_cast<LADataReference&>(dh->get());
				LAMathIndexEntity& index = dynamic_cast<LAMathIndexEntity&>(ref.get().get());
				index.clearGrid();
			}
		}
		dh = &(call.get().getData(PRICING_DATA_LSMCVARIABLES, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			const LAStringVector& targets = dynamic_cast<const LADataStrings&>(dh->get()).get();	
			//object pool
			LAObjectPool& objPool = call.get().getDataInstance()->getObjectPool();
			for (unsigned int i = 0; i < targets.size(); i++)
			{
				LAObjectHolder objHolder = objPool.getObject(targets[i], ENCHKTYPE_NOCHECK);
				if (!objHolder.isDefined()) continue;
				dh = &(objHolder.getData(PRICING_DATA_INDEXTYPE, NOCHECK));
				if (!dh->isDefined() || dh->isNull()) continue;// not indexinfo
				dh = &(objHolder.getData(PRICING_DATA_INDEXENTITY, NOCHECK));
				if (!dh->isDefined() || dh->isNull()) continue;
				LADataReference& ref = dynamic_cast<LADataReference&>(dh->get());
				LAMathIndexEntity& index = dynamic_cast<LAMathIndexEntity&>(ref.get().get());
				index.clearGrid();
			}
		}
	}
}


/*!
	@brief setup index object 

	@param[in] basedate basedate
	@param[in, out] trade trade object 

*/
void
LAPricePayOff::setUpIndexEntity(const LADate& basedate, LAObject& trade) const
{
	LADataHolder* dh;
	//leg object
	dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	LADataMultiReference& legs = dynamic_cast<LADataMultiReference&>(dh->get());

	for (unsigned int i = 0; i < legs.getSize(); i++)
	{
		//cashlets
		dh = &(legs.get(i).getData(PRICING_DATA_CASHLETS, NOCHECK));
		if (!dh->isDefined()) continue;
		LADataMultiReference& cashlets = dynamic_cast<LADataMultiReference&>(dh->get());
		
		for (unsigned int j = 0; j < cashlets.getSize(); j++)
			setUpIndexEntityOfCashlet(basedate, cashlets.get(j).get(), trade);					
	}
	
	//trigger
	dh = &(trade.getData(PRICING_DATA_TRIGGERINFOS, NOCHECK));
	if (dh->isDefined() && !dh->isNull()) 
	{
		LADataMultiReference& triggers = dynamic_cast<LADataMultiReference&>(dh->get());
		for (unsigned int i = 0; i < triggers.getSize(); i++)
			setUpIndexEntityOfTrigger(basedate, triggers.get(i).get(), trade);
	}
	//call
	dh = &(trade.getData(PRICING_DATA_CALLINFO, NOCHECK));
	if (dh->isDefined() && !dh->isNull()) 
	{
		LADataReference& call = dynamic_cast<LADataReference&>(dh->get());
		setUpIndexEntityOfCall(basedate, call.get().get(), trade);
	}
}
/*!
	@brief setup index object for coupon calculation

	@param[in] basedate basedate
	@param[in, out] cashlet cashlet object 
	@param[in] trade trade object 

*/
void
LAPricePayOff::setUpIndexEntityOfCashlet(const LADate& basedate, LAObject& cashlet, const LAObject& trade) const
{

	LADataHolder* dh;

	//coupon info
	dh = &(cashlet.getData(PRICING_DATA_COUPONINFOS, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
	    LADataMultiReference& couponinfos = dynamic_cast<LADataMultiReference&>(dh->get());

		for (unsigned int i = 0; i < couponinfos.getSize(); i++)
		{
			//index info
			dh = &(couponinfos.get(i).getData(PRICING_DATA_INDEXINFOS, NOCHECK));
			if (!dh->isDefined() || dh->isNull()) continue;
			LADataMultiReference& indexinfos = dynamic_cast<LADataMultiReference&>(dh->get());
			for (unsigned int j = 0; j < indexinfos.getSize(); j++)
			{
				dh = &(indexinfos.get(j).getData(PRICING_DATA_OBSERVATIONOPERATOR, NOCHECK));
				if(dh->isDefined() && !dh->isNull())
					setUpIndexEntityOfIndex(basedate, indexinfos.get(j).get(), trade, NULL, true);
				else
					setUpIndexEntityOfIndex(basedate, indexinfos.get(j).get(), trade);

				dh = &(indexinfos.get(j).getData(PRICING_DATA_INDEXENTITY, NOCHECK));
				if (!dh->isDefined() || dh->isNull()){
					continue;
				}
				LADataReference& ref = dynamic_cast<LADataReference&>(dh->get());
				LAMathIndexEntity& index = dynamic_cast<LAMathIndexEntity&>(ref.get().get());

				dh = &(indexinfos.get(j).getData(PRICING_DATA_FIXINGDATE, NOCHECK));
				if (dh->isDefined() && !dh->isNull())
				{
					//PaymentDate
					dh = &(cashlet.getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL));
					const LADate& paymentdate = dynamic_cast<const LADataDate&>(dh->get()).get();

					// CFCalcEndDate
					dh = &(cashlet.getData(PRICING_DATA_CFCALCENDDATE, ISNOTNULL));
					const LADate& enddate = dynamic_cast<const LADataDate&>(dh->get());

					const LADate& fixingdate = dynamic_cast<const LADataDate&>(dh->get());
					index.addConvexityGrids(fixingdate, paymentdate, enddate);
				}
			}
		}
	}
	//range accrue index
	if (isRangeAccrueCashlet(cashlet))
	{
		dh = &(cashlet.getData(PRICING_DATA_RANGEACCRUEINFOS, NOCHECK));
		if (dh->isDefined() && !dh->isNull()) // multi index case
		{
			const LADataMultiReference& infos = dynamic_cast<LADataMultiReference&>(dh->get());
			for (unsigned int i = 0; i < infos.getSize(); i++)
			{
				dh = &(infos.get(i).get().getData(PRICING_DATA_RANGEACCRUEINDEXINFOS, NOCHECK));
				if (dh->isDefined() && !dh->isNull())
				{
					LADataMultiReference& rangeAccrualIndexInfos = dynamic_cast<LADataMultiReference&>(dh->get());
					for (unsigned int j = 0; j < rangeAccrualIndexInfos.getSize(); j++)
					{
						setUpIndexEntityOfIndex(basedate, rangeAccrualIndexInfos.get(j).get(), trade, NULL, true);
					}
				}
				dh = &(infos.get(i).get().getData(PRICING_DATA_RANGEACCRUEBOUNDARYINDEXINFOS, NOCHECK));
				if (dh->isDefined() && !dh->isNull())
				{
					LADataMultiReference& rangeAccrualBoundaryIndexInfos = dynamic_cast<LADataMultiReference&>(dh->get());
					for (unsigned int j = 0; j < rangeAccrualBoundaryIndexInfos.getSize(); j++)
					{
						setUpIndexEntityOfIndex(basedate, rangeAccrualBoundaryIndexInfos.get(j).get(), trade, NULL, true);
					}
				}
			}
		}
		else // single index case
		{
			dh = &(cashlet.getData(PRICING_DATA_RANGEACCRUEINDEXINFOS, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				LADataMultiReference& rangeAccrualIndexInfos = dynamic_cast<LADataMultiReference&>(dh->get());
				for (unsigned int i = 0; i < rangeAccrualIndexInfos.getSize(); i++)
				{
					setUpIndexEntityOfIndex(basedate, rangeAccrualIndexInfos.get(i).get(), trade, NULL, true);
				}
			}
			dh = &(cashlet.getData(PRICING_DATA_RANGEACCRUEBOUNDARYINDEXINFOS, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				LADataMultiReference& rangeAccrualBoundaryIndexInfos = dynamic_cast<LADataMultiReference&>(dh->get());
				for (unsigned int i = 0; i < rangeAccrualBoundaryIndexInfos.getSize(); i++)
				{
					setUpIndexEntityOfIndex(basedate, rangeAccrualBoundaryIndexInfos.get(i).get(), trade, NULL, true);
				}
			}
		}
	}

    //// stub for compounding
    //if(isCompoundingCashlet(cashlet)){
    //    if((dh=&cashlet.getData(PRICING_DATA_FIRSTSTUBCOUPON))->isDefined() && !dh->isNull()){
    //        LAObject& first_stub_coupon = dynamic_cast<LADataReference&>(dh->get()).get().get();
    //        if((dh=&first_stub_coupon.getData(PRICING_DATA_INDEXINFOS))->isDefined() && !dh->isNull()){
    //            LADataMultiReference& first_stub_indices = dynamic_cast<LADataMultiReference&>(dh->get());
    //            for(size_t i = 0; i < first_stub_indices.getSize(); i++){
    //                setUpIndexEntityOfIndex(basedate, first_stub_indices.get(i).get(), trade);
    //            }
    //        }            
    //    }
    //    if((dh=&cashlet.getData(PRICING_DATA_LASTSTUBCOUPON))->isDefined() && !dh->isNull()){
    //        LAObject& last_stub_coupon = dynamic_cast<LADataReference&>(dh->get()).get().get();
    //        if((dh=&last_stub_coupon.getData(PRICING_DATA_INDEXINFOS))->isDefined() && !dh->isNull()){
    //            LADataMultiReference& last_stub_indices = dynamic_cast<LADataMultiReference&>(dh->get());
    //            for(size_t i = 0; i < last_stub_indices.getSize(); i++){
    //                setUpIndexEntityOfIndex(basedate, last_stub_indices.get(i).get(), trade);
    //            }
    //        }            
    //    }
    //}

	//notionalcf info
	dh = &(cashlet.getData(PRICING_CALIBRATION_DATAOTIONALCFCOUPONINFOS, NOCHECK));
	if(!dh->isDefined() || dh->isNull()) return;
	LADataMultiReference& notionalcfinfos = dynamic_cast<LADataMultiReference&>(dh->get());

	for (unsigned int i = 0; i < notionalcfinfos.getSize(); i++)
	{
		//index info
		dh = &(notionalcfinfos.get(i).getData(PRICING_DATA_INDEXINFOS, NOCHECK));
		if (!dh->isDefined() || dh->isNull()) continue;
		LADataMultiReference& indexinfos = dynamic_cast<LADataMultiReference&>(dh->get());
		for (unsigned int j = 0; j < indexinfos.getSize(); j++)
			setUpIndexEntityOfIndex(basedate, indexinfos.get(j).get(), trade);
	}
}
/*!
	@brief setup index object for call judge

	@param[in] basedate basedate
	@param[in, out] callinfo call infromation object 
	@param[in] trade trade object 

*/
void
LAPricePayOff::setUpIndexEntityOfCall (const LADate& basedate, LAObject& callinfo, const LAObject& trade) const
{
	LADataHolder* dh;
	//expirydates
	dh = &(callinfo.getData(PRICING_DATA_EXPIRYDATES, ISNOTNULL));
	const DateVector& fixingdates = dynamic_cast<const LADataDates&>(dh->get()).get();	
	
	//lsmc variables
	dh = &(callinfo.getData(PRICING_DATA_LSMCVARIABLES, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		const LAStringVector& targets = dynamic_cast<const LADataStrings&>(dh->get()).get();	
		//object pool
		LAObjectPool& objPool = callinfo.getDataInstance()->getObjectPool();
		for (unsigned int i = 0; i < targets.size(); i++)
		{
			LAObjectHolder objHolder = objPool.getObject(targets[i], ENCHKTYPE_NOCHECK);
			if (!objHolder.isDefined()) continue;
			dh = &(objHolder.getData(PRICING_DATA_INDEXTYPE, NOCHECK));
			if (!dh->isDefined() || dh->isNull()) continue;// not indexinfo
			setUpIndexEntityOfIndex(basedate, objHolder.get(), trade, &fixingdates);
		}
	}
	//indexinfo
	dh = &(callinfo.getData(PRICING_DATA_INDEXINFOS, NOCHECK));
	if (!dh->isDefined() || dh->isNull()) return;
	LADataMultiReference& indexinfos = dynamic_cast<LADataMultiReference&>(dh->get());
	for (unsigned int i = 0; i < indexinfos.getSize(); i++)
	{
		setUpIndexEntityOfIndex(basedate, indexinfos.get(i).get(), trade, &fixingdates);
	}

	dh = &(callinfo.getData(PRICING_DATA_EXTRACFINDEXINFOS, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		LADataMultiReference& extracfindexinfos = dynamic_cast<LADataMultiReference&>(dh->get());
		dh = &(callinfo.getData(PRICING_DATA_EXTRACFDATES, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			const DateVector& extradates = dynamic_cast<const LADataDates&>(dh->get()).get();
			for (unsigned int i = 0; i < extracfindexinfos.getSize(); i++)
				setUpIndexEntityOfIndex(basedate, extracfindexinfos.get(i).get(), trade, &extradates);
		}
		else
		{
			dh = &(callinfo.getData(PRICING_DATA_ACTIONDATES, ISNOTNULL));
			const DateVector& actiondates = dynamic_cast<const LADataDates&>(dh->get()).get();
			for (unsigned int i = 0; i < extracfindexinfos.getSize(); i++)
				setUpIndexEntityOfIndex(basedate, extracfindexinfos.get(i).get(), trade, &actiondates);
		}
	}
}
/*!
	@brief setup index object for trigger judge

	@param[in] basedate basedate
	@param[in, out] triggerinfo trigger infromation object 
	@param[in] trade trade object 

*/
void
LAPricePayOff::setUpIndexEntityOfTrigger (const LADate& basedate, LAObject& triggerinfo, const LAObject& trade) const
{
	LADataHolder* dh;
	//expirydates
	dh = &(triggerinfo.getData(PRICING_DATA_EXPIRYDATES, ISNOTNULL));
	const DateVector& fixingdates = dynamic_cast<const LADataDates&>(dh->get()).get();	
	
	//triggertargets
	dh = &(triggerinfo.getData(PRICING_DATA_TRIGGERTARGETS, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		const LAStringVector& targets = dynamic_cast<const LADataStrings&>(dh->get()).get();	
		//object pool
		LAObjectPool& objPool = triggerinfo.getDataInstance()->getObjectPool();
		for (unsigned int i = 0; i < targets.size(); i++)
		{
			LAObjectHolder objHolder = objPool.getObject(targets[i], ENCHKTYPE_NOCHECK);
			if (!objHolder.isDefined()) continue;
			dh = &(objHolder.getData(PRICING_DATA_INDEXTYPE, NOCHECK));
			if (!dh->isDefined() || dh->isNull()) continue;// not indexinfo
			setUpIndexEntityOfIndex(basedate, objHolder.get(), trade, &fixingdates);
		}
	}
	
	//indexinfo
	dh = &(triggerinfo.getData(PRICING_DATA_INDEXINFOS, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		LADataMultiReference& indexinfos = dynamic_cast<LADataMultiReference&>(dh->get());
		for (unsigned int i = 0; i < indexinfos.getSize(); i++)
			setUpIndexEntityOfIndex(basedate, indexinfos.get(i).get(), trade, &fixingdates);
	}

	dh = &(triggerinfo.getData(PRICING_DATA_EXTRACFINDEXINFOS, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		dh = &(triggerinfo.getData(PRICING_DATA_ACTIONDATES, ISNOTNULL));
		const DateVector& actiondates = dynamic_cast<const LADataDates&>(dh->get()).get();
		dh = &(triggerinfo.getData(PRICING_DATA_EXTRACFINDEXINFOS, NOCHECK));
		LADataMultiReference& extracfindexinfos = dynamic_cast<LADataMultiReference&>(dh->get());
		for (unsigned int i = 0; i < extracfindexinfos.getSize(); i++)
			setUpIndexEntityOfIndex(basedate, extracfindexinfos.get(i).get(), trade, &actiondates);
	}

	//////////////////////////////////
	//trigger action cpn change case//
	//////////////////////////////////
	dh = &(triggerinfo.getData(PRICING_DATA_COUPONCHANGEINFO, NOCHECK));	
	if (!dh->isDefined() || dh->isNull()) return;
	
	//leg object
	const LADataHolder* ah_c = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	const LADataMultiReference& legs = dynamic_cast<const LADataMultiReference&>(ah_c->get());
    
	for (unsigned int i = 0; i < legs.getSize(); i++)
	{
		dh = &(triggerinfo.getData(PRICING_DATA_COUPONINFOS + LADataInt(i + 1).convertToString(), NOCHECK));	
		if (!dh->isDefined() || dh->isNull()) continue;

	
		LADataMultiReference& couponinfos = dynamic_cast<LADataMultiReference&>(dh->get());

		for (unsigned int j = 0; j < couponinfos.getSize(); j++)
		{
			//index info
			dh = &(couponinfos.get(j).getData(PRICING_DATA_INDEXINFOS, NOCHECK));
			if (!dh->isDefined() || dh->isNull()) continue;
			LADataMultiReference& indexinfos = dynamic_cast<LADataMultiReference&>(dh->get());
			for (unsigned int k = 0; k < indexinfos.getSize(); k++)
			{
				setUpIndexEntityOfIndex(basedate, indexinfos.get(k).get(), trade);
			}
		}
			
	}

}

/*!
	@brief setup index object refered from index information object

	@param[in] basedate basedate
	@param[in, out] indexinfo index infromation object 
	@param[in] trade trade object 
	@param[in] pfixingdates fixing dates 

*/
void
LAPricePayOff::setUpIndexEntityOfIndex (const LADate& basedate, LAObject& indexinfo, 
									const LAObject& trade,
									const DateVector* pfixingdates, bool isInter) const
{
	LADataHolder* dh;
	//index type
	dh = &(indexinfo.getData(PRICING_DATA_INDEXTYPE, ISNOTNULL));
	LAString indextype = dynamic_cast<const LADataString&>(dh->get());
	indextype.toUpper();
	if (indextype != LIBOR && indextype != CMS && indextype != CMT && indextype != OIS 
		&& indextype != FX_RATE	&& indextype != CUSTOM) return;

	set<LADate> grid;
	std::map<LADate, DateVector> datesforindexgenerate;
    // dates to set index object 
	if (pfixingdates == NULL)
	{	
		dh = &indexinfo.getData(PRICING_DATA_OBSERVATIONSTARTDATE, NOCHECK);
		const bool has_obs_start_date = dh->isDefined() && !dh->isNull();

		dh = &indexinfo.getData(PRICING_DATA_FIXINGDATES, NOCHECK);
		const bool has_fixing_dates = dh->isDefined() && !dh->isNull();

		dh = &indexinfo.getData(PRICING_DATA_FIXINGDATE, NOCHECK);
		const bool has_fixing_date = dh->isDefined() && !dh->isNull();

		if(has_fixing_dates){
			dh = &indexinfo.getData(PRICING_DATA_FIXINGDATES, NOCHECK);
			const DateVector& fixingdates = dynamic_cast<const LADataDates&>(dh->get()).get();

			//if exists, index generation method but use cfstartdates and cfenddates
			dh = &(indexinfo.getData(PRICING_DATA_DATESFORINDEXGENERATE,NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{

				dh = &(indexinfo.getData(PRICING_DATA_CFCALCSTARTDATES, ISNOTNULL));
				const DateVector& startvec = dynamic_cast<const LADataDates&>(dh->get()).get();

				dh = &(indexinfo.getData(PRICING_DATA_CFCALCENDDATES, ISNOTNULL));
				const DateVector& endvec = dynamic_cast<const LADataDates&>(dh->get()).get();

				for (unsigned int i = 0; i < fixingdates.size(); i++)
				{
					if (basedate <= fixingdates[i]) 
					{
						grid.insert(fixingdates[i]);

						DateVector vec(2);
						vec[0] = startvec[i];
						vec[1] = endvec[i];
						datesforindexgenerate.insert(std::make_pair(fixingdates[i],vec));
					}
				}
			}
			else
			{
				for (unsigned int i = 0; i < fixingdates.size(); i++)
				{
					if (basedate <= fixingdates[i]) 
						grid.insert(fixingdates[i]);
				}
			}
		}
		else if(has_obs_start_date){
			dh = &indexinfo.getData(PRICING_DATA_OBSERVATIONSTARTDATE, NOCHECK);
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



			if (pcal != NULL)
			{
				LADate tmp = psrule->getDate(end, *pcal);
				if (out.at(out.size() - 1) != tmp) out.push_back(tmp);
			}
			else
				if (out.at(out.size() - 1) != end) out.push_back(end);

			for (unsigned int i = 0; i < out.size(); i++)
				if (basedate <= out[i]) grid.insert(out[i]);	
		}
		else if(has_fixing_date){
			dh = &indexinfo.getData(PRICING_DATA_FIXINGDATE, NOCHECK);
			const LADate& fixingdate = dynamic_cast<const LADataDate&>(dh->get());
			if (basedate <= fixingdate)
			{
				grid.insert(fixingdate);
				dh = &(indexinfo.getData(PRICING_DATA_DATESFORINDEXGENERATE,NOCHECK));
				if (dh->isDefined() && !dh->isNull())
				{
					const DateVector& vec = dynamic_cast<const LADataDates&>(dh->get()).get();
					datesforindexgenerate.insert(std::make_pair(fixingdate,vec));
				}
			}
		}
		else{
			throw LACoreInvalidData("libor compounding fixing date error",__FILE__,__LINE__);
		}
	}
	else
	{
		//observation end
		dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONENDTERM, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			const LAString& endterm = dynamic_cast<const LADataString&>(dh->get());
		
			//slidingrule & calendar
			const LAPriceDataSlidingRule* psrule;
			const LAPriceDataCalendar* pcal;
			LAPriceCFGenUtility::getBusDayRuleAndCalendar(indexinfo, 
														PRICING_DATA_OBSERVATIONSLIDINGRULE,
														PRICING_DATA_OBSERVATIONCALENDAR,
														indexinfo,
														CALIBRATION_DATA_SLIDINGRULE,
														CALIBRATION_DATA_CALENDAR,
														psrule, pcal);			
			
			//observation start
			LADate start;
			LAString startterm;
			bool isstartterm = false;
			dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONSTARTDATE, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				start = dynamic_cast<const LADataDate&>(dh->get());
				if (psrule->getSlidingRule() != SLIDING_RULE_NO_CHANGE)
					start = psrule->getDate(start, *pcal);			
			}
			else
			{
				dh = &(indexinfo.getData(PRICING_DATA_OBSERVATIONSTARTTERM, ISNOTNULL));
				startterm = dynamic_cast<const LADataString&>(dh->get());
				isstartterm = true;
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

			for (unsigned int i = 0; i <pfixingdates->size(); i++)
			{
				if (isstartterm)
					start = LAMathDateCalculations::getDate((*pfixingdates)[i], startterm, 
										*psrule, 
										pcal, 
										false);
				LADate end = LAMathDateCalculations::getDate((*pfixingdates)[i], endterm, 
										*psrule, 
										pcal, 
										false);

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
				else
					if (out.at(out.size() - 1) != end) out.push_back(end);

				for (unsigned int i = 0; i < out.size(); i++)
					if (basedate <= out[i]) grid.insert(out[i]);
			}
			
		}
		else
		{
			for (unsigned int i = 0; i <pfixingdates->size(); i++)
				if (basedate <= (*pfixingdates)[i]) grid.insert((*pfixingdates)[i]);		
		}
	}
	const LADataHolder *c_ah = &(trade.getData(PRICING_DATA_ISSAVEPASTFIXING, NOCHECK));
	if (c_ah->isDefined() && !c_ah->isNull())
	{
		if (dynamic_cast<const LADataBool &>(c_ah->get()))
		{
			grid.insert(basedate);
		}
	}



	if (grid.size() == 0 && indextype != OIS)
		return;
    
	// index object
	LAMathIndexEntity* pIndex;
	if (indextype == CUSTOM)
		dh = &(indexinfo.getData(PRICING_DATA_INDEXENTITY, ISNOTNULL)); 
	else
		dh = &(indexinfo.getData(PRICING_DATA_INDEXENTITY, NOCHECK)); 
	
	if (!dh->isDefined())
		dh = &indexinfo.add(PRICING_DATA_INDEXENTITY, new LADataReference());

	bool isSetAttr = false;
	if (dh->isNull())   
	{
		//trade name
		const LADataHolder* cah = &(trade.getData(CALIBRATION_DATA_NAME, ISNOTNULL));
		const LAString& tradename = dynamic_cast<const LADataString&>(cah->get()).get();
		//currency
		dh = &(indexinfo.getData(IR_MODEL_DATA_CURRENCY, ISNOTNULL));
		const LAString& currency = dynamic_cast<const LADataString&>(dh->get());
		//accessory
		LAString accessory;
		dh = &(indexinfo.getData(IR_MODEL_DATA_ACCESSORY, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
			accessory = dynamic_cast<const LADataString&>(dh->get());
		//frequency
		LAString frequency;
		dh = &(indexinfo.getData(IR_MODEL_DATA_FREQUENCY, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
			frequency = dynamic_cast<const LADataString&>(dh->get());
		//is range accrue
		dh = &(indexinfo.getData(PRICING_DATA_ISRANGEACCRUE, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			bool isRangeAccrue = dynamic_cast<const LADataBool&>(dh->get());
			isInter = isRangeAccrue;
		}

		const LADataReference* ref = &dynamic_cast<const LADataReference&>(trade.getData(PRICING_DATA_PATHENTITY, ISNOTNULL).get());
		LAString pathname = ref->get().getName();

		// name of index object
		LAString name;
		name = tradename + "_" + indextype + "_" + currency + "_" + accessory + "_" + frequency + "_" + LAString(isInter) + "_" + pathname;

		// daycount
		if (indextype == LIBOR || indextype == CMS || indextype == CMT || indextype == OIS)
		{
			dh = &(indexinfo.getData(IR_MODEL_DATA_DAYCOUNT, ISNOTNULL));
			LAString daycount = dh->convertToString();
			name += "_" + daycount;
		}

		// basis
		dh = &(indexinfo.getData(PRICING_DATA_BASISCURVE, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			const LAString &basisCurve = dynamic_cast<const LADataString&>(dh->get()).get();
			name += "_" + basisCurve;
		}
	
		//for plain vanilla
		LAString rollday;
		dh = &(indexinfo.getData(PRICING_DATA_ROLLDAYFORINDEXGENERATE, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
			rollday = LAString(dynamic_cast<const LADataInt&>(dh->get()));
		if (rollday.isDefined())
			name += "_" + rollday;

		//for plain vanilla
		LAString spotlagstr;
		dh = &(indexinfo.getData(PRICING_DATA_SPOTLAG, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
			spotlagstr =  LAString(dynamic_cast<const LADataInt&>(dh->get()));
		if (spotlagstr.isDefined())
			name += "_" + spotlagstr;
			
		
		LAObjectPool& objPool = indexinfo.getDataInstance()->getObjectPool();
		LAObjectHolder objHolder = objPool.getObject(name);
		if (!objHolder.isDefined())
		{
			LAObjectHolder tmph;
			try
			{
				if (isInter)
				{
#ifndef VISUAL_STUDIO_2010_ANALYTICS
					tmph.setEntity(new LAMathIndexEntityInterpolation(indexinfo.getDataInstance()), true);
#endif
				}
				else
				{
					tmph.setEntity(new LAMathIndexEntity(indexinfo.getDataInstance()), true);
				}
				isSetAttr = true;
			}
			catch (bad_alloc & e)
			{
				throw LACoreSystemError(e.what(), __FILE__, __LINE__);
			}	
			objPool.set(name, &tmph.get());			
			tmph.setDeleteFlag(false);
			pIndex = &dynamic_cast<LAMathIndexEntity&>(tmph.get());
			pIndex->getName().convertFromString(name);
		}
		else
			pIndex = &dynamic_cast<LAMathIndexEntity&>(objHolder.get());

		indexinfo.getData(PRICING_DATA_INDEXENTITY, ISDEFINED).convertFromString(name);
	}
	else
	{
		LADataReference& ref = dynamic_cast<LADataReference&>(dh->get());
		pIndex = &dynamic_cast<LAMathIndexEntity&>(ref.get().get());
	}

	if (indextype != CUSTOM && isSetAttr)
	{
		//indextype
		pIndex->getIndexType().convertFromString(indextype);
		//accessory
		dh = &(indexinfo.getData(IR_MODEL_DATA_ACCESSORY, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
			pIndex->getAccessory().convertFromString(dh->convertToString());
		else
			pIndex->getAccessory().convertFromString(NULL_STR);
		//currecny
		dh = &(indexinfo.getData(IR_MODEL_DATA_CURRENCY, ISNOTNULL));
		pIndex->getCurrency().convertFromString(dh->convertToString());
		//calendar
		dh = &(indexinfo.getData(CALIBRATION_DATA_CALENDAR, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
			pIndex->getCalendar().convertFromString(dh->convertToString());
		else
			pIndex->getCalendar().convertFromString(NULL_STR);
		//sliding rule
		dh = &(indexinfo.getData(CALIBRATION_DATA_SLIDINGRULE, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
			pIndex->getSlidingRule().convertFromString(dh->convertToString());
		else
			pIndex->getSlidingRule().convertFromString(NULL_STR);
		//frequency
		dh = &(indexinfo.getData(IR_MODEL_DATA_FREQUENCY, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
			pIndex->getFrequency().convertFromString(dh->convertToString());
		else
			pIndex->getFrequency().convertFromString(NULL_STR);
		//daycount
		dh = &(indexinfo.getData(IR_MODEL_DATA_DAYCOUNT, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
			pIndex->getDayCount().convertFromString(dh->convertToString());
		else
			pIndex->getDayCount().convertFromString(NULL_STR);
		//path object
		const LADataHolder* cah = &trade.getData(PRICING_DATA_PATHENTITY, ISNOTNULL);
		const LADataReference* ref = &dynamic_cast<const LADataReference&>(cah->get());
		pIndex->getPathEntity().convertFromString(ref->get().getName());
		//FX object
		if (indextype == FX_RATE)
		{
			cah = &trade.getData(PRICING_DATA_FXRATE, ISNOTNULL);
			ref = &dynamic_cast<const LADataReference&>(cah->get());
			pIndex->getFXEntity().convertFromString(ref->get().getName());
		}

		dh = &(indexinfo.getData(PRICING_DATA_ROLLDAYFORINDEXGENERATE, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			pIndex->remove(PRICING_DATA_ROLLDAYFORINDEXGENERATE);
			pIndex->LAObject::add(PRICING_DATA_ROLLDAYFORINDEXGENERATE,new LADataInt()).convertFromString(dh->convertToString());
		}

		dh = &(indexinfo.getData(PRICING_DATA_SPOTLAG, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			unsigned int spotlag = dynamic_cast<const LADataInt&>(dh->get()).get();
			pIndex->setSpotLag(pIndex->getCurrency().get(), spotlag);
		}

		dh = &(indexinfo.getData(PRICING_DATA_BASISCURVE, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			pIndex->getBasis().convertFromString(dh->convertToString());

			dh = &(indexinfo.getData(PRICING_DATA_BASISINTERPOLATION, ISNOTNULL));
			pIndex->getBasisInterpolation().convertFromString(dh->convertToString());
		}
		
		dh = &(indexinfo.getData(PRICING_DATA_DISCOUNTCURVE, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			pIndex->getDiscountCurve().convertFromString(dh->convertToString());
		}

		dh = &(indexinfo.getData(PRICING_DATA_ISFWDINTERPOLATION, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			pIndex->getIsFWDInterpolation().convertFromString(dh->convertToString());

			dh = &(indexinfo.getData(PRICING_DATA_FWDINTERPOLATION, ISNOTNULL));
			pIndex->getFWDInterpolation().convertFromString(dh->convertToString());
		}

		dh = &(indexinfo.getData(PRICING_DATA_VOLATILITYUNDERLYING, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			pIndex->getVolUnderlying().convertFromString(dh->convertToString());
		}

		dh = &(indexinfo.getData(PRICING_DATA_CFFREQUENCY, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			pIndex->remove(PRICING_DATA_CFFREQUENCY);
			pIndex->LAObject::add(PRICING_DATA_CFFREQUENCY, new LADataString()).convertFromString(dh->convertToString());
		}

		dh = &(indexinfo.getData(PRICING_DATA_PAYMENTLAG, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			pIndex->remove(PRICING_DATA_PAYMENTLAG);
			pIndex->LAObject::add(PRICING_DATA_PAYMENTLAG, new LADataString()).convertFromString(dh->convertToString());
		}

		dh = &(indexinfo.getData(PRICING_DATA_CFDAYCOUNT, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			pIndex->remove(PRICING_DATA_CFDAYCOUNT);
			pIndex->LAObject::add(PRICING_DATA_CFDAYCOUNT, new LAPriceDataDayCount()).convertFromString(dh->convertToString());
		}
		
		dh = &(indexinfo.getData(PRICING_DATA_CFCALCSTARTDATES, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			pIndex->remove(PRICING_DATA_CFCALCSTARTDATES);
			pIndex->LAObject::add(PRICING_DATA_CFCALCSTARTDATES, new LADataDates()).convertFromString(dh->convertToString());
		}

		dh = &(indexinfo.getData(PRICING_DATA_CFCALCENDDATES, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			pIndex->remove(PRICING_DATA_CFCALCENDDATES);
			pIndex->LAObject::add(PRICING_DATA_CFCALCENDDATES, new LADataDates()).convertFromString(dh->convertToString());
		}

		dh = &(indexinfo.getData(PRICING_DATA_CAINTEGRALCONDITION, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			pIndex->remove(PRICING_DATA_CAINTEGRALCONDITION);
			pIndex->LAObject::add(PRICING_DATA_CAINTEGRALCONDITION, new LADataDoubles()).convertFromString(dh->convertToString());
		}

		dh = &(indexinfo.getData(PRICING_DATA_CAPREMIUMINTEGRALSTEP, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			pIndex->remove(PRICING_DATA_CAPREMIUMINTEGRALSTEP);
			pIndex->LAObject::add(PRICING_DATA_CAPREMIUMINTEGRALSTEP, new LADataInt()).convertFromString(dh->convertToString());
		}

		dh = &(indexinfo.getData(PRICING_DATA_CAMODEL, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			pIndex->remove(PRICING_DATA_CAMODEL);
			pIndex->LAObject::add(PRICING_DATA_CAMODEL, new LADataString()).convertFromString(dh->convertToString());
		}

		dh = &(indexinfo.getData(PRICING_DATA_CATHRESHOLD, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			pIndex->remove(PRICING_DATA_CATHRESHOLD);
			pIndex->LAObject::add(PRICING_DATA_CATHRESHOLD, new LADataDouble()).convertFromString(dh->convertToString());
		}
	}

	set<LADate>::const_iterator it;
	for (it = grid.begin(); it != grid.end(); it++)
		pIndex->addGrid(*it);

	pIndex->setDatesForIndexGenerates(datesforindexgenerate);

}



/*!
	@brief setup payoff

	@param[in] basedate basedate
	@param[in, out] trade trade object 

*/
void
LAPricePayOff::setUpPayOff(const LADate& basedate, LAObject& trade)
{
	LADataHolder* dh;
	mPayOffMaster.clear();
	mPayOff.clear();

	//leg object
	dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	LADataMultiReference& legs = dynamic_cast<LADataMultiReference&>(dh->get());

	mPayOffMaster.resize(legs.getSize());
	mTimes.resize(legs.getSize());

	std::vector<DateVector> datemat(legs.getSize());

    for (unsigned int i = 0; i < legs.getSize(); i++)
	{
		bool isupfrontmode = false;
		dh = &(legs.get(i).getData(PRICING_DATA_UPFRONTFEES, NOCHECK));
		isupfrontmode = (dh->isDefined() && !dh->isNull());

		//cashlets
		dh = &(legs.get(i).getData(PRICING_DATA_CASHLETS, NOCHECK));
		if (!dh->isDefined()) continue;
		LADataMultiReference& cashlets = dynamic_cast<LADataMultiReference&>(dh->get());


		BoolVector isFixedCF;

		unsigned int k = 0;
		for (unsigned int j = 0; j < cashlets.getSize(); j++)
		{
			//paymentdate
			dh = &(cashlets.get(j).getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL));
			const LADate& date = dynamic_cast<const LADataDate&>(dh->get()).get();
			if (date <= basedate) continue;
			//Add a flag for checking notional CF is fixed
			if (j > 0)
			{
				dh = &cashlets.get(j - 1).getData(PRICING_CALIBRATION_DATAOTIONALCFISFIXED, NOCHECK);
				if (dh->isDefined() && !dh->isNull())
				{
					const bool notionaCF_IsFixed = dynamic_cast<const LADataBool&>(dh->get()).get();

					if (notionaCF_IsFixed)
					{
						isFixedCF.push_back(true);
					}
					else
					{
						isFixedCF.push_back(false);
					}
				}
				else
				{
					isFixedCF.push_back(true);
				}
			}
			else
			{
				isFixedCF.push_back(true);
			}


			LAPricePayOffToolHolder h;
			//is range accurual
			if (isRangeAccrueCashlet(cashlets.get(j).get()))
			{
#ifndef VISUAL_STUDIO_2010_ANALYTICS
				h.setPayOff(new LAPricePayOffToolRangeAccrue());
#endif
			}
            else if(isCompoundingCashlet(cashlets.get(j).get()))
            {
                h.setPayOff(new LAPricePayOffToolCompound());
            }
			else
			{
				h.setPayOff(new LAPricePayOffTool());
			}
			mPayOffMaster[i].push_back(h);
			h.getPayOff().setUp(basedate, trade, i, cashlets.get(j).get(), *this, k++);
			h.getPayOff().setFlagForUpFrontFee(false);
			mTimes[i].push_back(h.getPayOff().mPaymentTime);


			if (isupfrontmode)
				datemat[i].push_back(date);

		}
		legs.get(i).remove(PRICING_DATA_IS_FIXED_CF);
		legs.get(i).add(PRICING_DATA_IS_FIXED_CF, new LADataBools(isFixedCF));
	}

	//upfrontfees
	 for (unsigned int i = 0; i < legs.getSize(); i++)
	{
		dh = &(legs.get(i).getData(PRICING_DATA_UPFRONTFEES, NOCHECK));
		if (!dh->isDefined() || dh->isNull())
			continue;

		const DoubleVector& feevec = dynamic_cast<const LADataDoubles &>(dh->get()).get();

		dh = &(legs.get(i).getData(PRICING_DATA_UPFRONTPAYMENTDATES, ISNOTNULL));
		const DateVector& feedates = dynamic_cast<const LADataDates &>(dh->get()).get();

		if (feevec.size() != feedates.size())
			throw LACoreInvalidData("UpfrontSize Error", __FILE__,__LINE__);

		for (unsigned int j = 0; j < feedates.size(); j++)
		{
			if (feedates[j] <= basedate) continue;

			unsigned int pos = 0;
			LAAlgorithm::locate<DateVector, LADate>(datemat[i],feedates[j], datemat[i].size(),pos);

			LAPricePayOffToolHolder h;
			h.setPayOff(new LAPricePayOffTool());
			mPayOffMaster[i].insert(mPayOffMaster[i].begin() + pos, h);
			
			LAObject e;
			e.add(PRICING_CALIBRATION_DATAOTIONAL, new LADataDouble(0.0));
			e.add(PRICING_DATA_EXTRACF, new LADataDouble(feevec[j]));
			e.add(PRICING_DATA_PAYMENTDATE, new LADataDate(feedates[j]));
			
			h.getPayOff().setUp(basedate, trade, i, e, *this, 0);
			h.getPayOff().setFlagForUpFrontFee(true);

			mTimes[i].insert(mTimes[i].begin() + pos, h.getPayOff().mPaymentTime);
			datemat[i].insert(datemat[i].begin() + pos, feedates[j]);
		}
	}
}

/*!
	@brief setup payoff accrued

	@param[in] basedate basedate
	@param[in, out] trade trade object 

*/
void
LAPricePayOff::setUpPayOffAccrued(const LADate& basedate, LAObject& trade, int legNum)
{
	LADataHolder* dh;
	mPayOffMaster.clear();
	mPayOff.clear();
	
	//leg object
	dh = &(trade.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	LADataMultiReference& legs = dynamic_cast<LADataMultiReference&>(dh->get());

	mPayOffMaster.resize(legs.getSize());
	mTimes.resize(legs.getSize());

    for (unsigned int i = 0; i < legs.getSize(); i++)
	{
		if (i == legNum)
		{
			//cashlets
			dh = &(legs.get(i).getData(PRICING_DATA_CASHLETS, NOCHECK));
			if (!dh->isDefined()) continue;
			LADataMultiReference& cashlets = dynamic_cast<LADataMultiReference&>(dh->get());

			unsigned int k = 0;
			for (unsigned int j = 0; j < cashlets.getSize(); j++)
			{
				//paymentdate
				dh = &(cashlets.get(j).getData(PRICING_DATA_PAYMENTDATE, ISNOTNULL));
				const LADate& date = dynamic_cast<const LADataDate&>(dh->get()).get();
				if (date <= basedate) continue;


				LAPricePayOffToolHolder h;
				//is range accurual
				if (isRangeAccrueCashlet(cashlets.get(j).get()))
				{
#ifndef VISUAL_STUDIO_2010_ANALYTICS
					h.setPayOff(new LAPricePayOffToolRangeAccrue());
#endif
				}
				else
				{
					h.setPayOff(new LAPricePayOffTool());
}
				mPayOffMaster[i].push_back(h);
				h.getPayOff().setUp(basedate, trade, i, cashlets.get(j).get(), *this, k++);
				mTimes[i].push_back(h.getPayOff().mPaymentTime);
			}
		}
	}
}

/*!
	@brief setup trigger and call

	@param[in] basedate basedate
	@param[in, out] trade trade object 
*/
void
LAPricePayOff::setUpTrigger(const LADate& basedate, LAObject& trade)
{
#ifndef VISUAL_STUDIO_2010_ANALYTICS
	//initialize
	for (unsigned int i = 0; i < mTriggers.size(); i++)
		delete mTriggers[i];
	mTriggers.clear();
	mIsTrigger = false;

	LADataHolder* dh;
	dh = &(trade.getData(PRICING_DATA_CALLINFO, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
			
		LADataReference& call = dynamic_cast<LADataReference&>(dh->get());
			
		//expiry dates
		dh = &(call.get().getData(PRICING_DATA_EXPIRYDATES, ISNOTNULL));
		const DateVector& expirydates = dynamic_cast<const LADataDates&>(dh->get()).get();
		if (expirydates.size() > 0 && expirydates.back() > basedate)
		{
			mIsTrigger = true;			
			mTriggers.push_back(new LAPriceEventTool());
			mTriggers.back()->setUp(basedate, trade, call.get().get(),
								*this, 
								0,
								true, mIsLSMC);
		}

	}

	
	//trigger info 
	dh = &(trade.getData(PRICING_DATA_TRIGGERINFOS, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		
		const LADataMultiReference& triggers = dynamic_cast<const LADataMultiReference&>(dh->get());
		for (unsigned int i = 0; i < triggers.getSize(); i++)
		{
			//expiry dates
			dh = &(triggers.get(i).getData(PRICING_DATA_EXPIRYDATES, ISNOTNULL));
			const DateVector& expirydates = dynamic_cast<const LADataDates&>(dh->get()).get();
			if (expirydates.size() == 0 || expirydates.back() <= basedate) continue;

			mIsTrigger = true;
			mTriggers.push_back(new LAPriceEventTool());
			mTriggers.back()->setUp(basedate, trade, triggers.get(i).get(),
									  *this, 
									  i + 1,
									  false);
		}
	}


	
	for (unsigned int i = 0; i < mTriggers.size(); i++)
		mActions.insert(mActions.end(),	mTriggers[i]->getAction().begin(), 	mTriggers[i]->getAction().end());


	sort(mActions.begin(), mActions.end(), Comp_pTriggerActionHolder());
#endif
}

/*!
	@brief check range accrue cashlet 

	@param[in] object
*/
bool
LAPricePayOff::isRangeAccrueCashlet(const LAObject &object) const
{
	const LADataHolder *dh = 0;
	dh = &(object.getData(PRICING_DATA_ISRANGEACCRUE, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		if (dynamic_cast<const LADataBool &>(dh->get()).get())
		{
			return true;
		}
	}
	return false;
}

bool 
LAPricePayOff::isCompoundingCashlet(const LAObject& object) const
{
    const LADataHolder* dh;
    return (dh=&object.getData(PRICING_DATA_ISCOMPOUNDINGCOUPON))->isDefined() && !dh->isNull() && dynamic_cast<const LADataBool&>(dh->get()).get();
}

void 
LAPricePayOff::getCompoundedRateInfo(const size_t leg, DateVector& start, DateVector& end, DoubleVector& term, DateVector& fixing_date, LAStringVector& fixing_flag, DoubleVector& rate) const
{
	const PayOffToolHolderVector& v = getPayOff().at(leg);
	for(size_t i = 0; i < v.size(); i++){
		DateVector temp_start, temp_end, temp_fixd;
        LAStringVector temp_fixf;
		DoubleVector temp_term, temp_rate;
		v[i].getPayOff().getCompoundedRateInfo(temp_start, temp_end, temp_term, temp_fixd, temp_fixf, temp_rate);
		start.insert(start.end(), temp_start.begin(), temp_start.end());
		end.insert(end.end(), temp_end.begin(), temp_end.end());
		term.insert(term.end(), temp_term.begin(), temp_term.end());
		fixing_date.insert(fixing_date.end(), temp_fixd.begin(), temp_fixd.end());
        fixing_flag.insert(fixing_flag.end(), temp_fixf.begin(), temp_fixf.end());
		rate.insert(rate.end(), temp_rate.begin(), temp_rate.end());
	}
}
