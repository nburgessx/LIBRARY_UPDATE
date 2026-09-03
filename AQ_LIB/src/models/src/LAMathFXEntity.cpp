/*! @file
    @brief Source code for class to represent FX.

			Following dataValues are registered automatically to data master<BR>
			1.CALIBRATION_DATA_NAME(LADataString)<BR>
			2.CALIBRATION_DATA_ASOFDATE(LADataDate)<BR>
			3.IR_MODEL_DATA_FXTYPE(LADataString)<BR>
			4.IR_MODEL_DATA_PATHENTITY(LADataReference)<BR>
			5.IR_MODEL_DATA_YIELDCURVES(LADataMultiReference)<BR>
			6.IR_MODEL_DATA_CURRENCYS(LADataStrings)<BR>
			7.IR_MODEL_DATA_SPOTRATES(LADataDoubles)<BR>
			8.IR_MODEL_DATA_CALENDARNAMES(LADataStrings)<BR>


*/
//  2006, Mizuho International London..

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LAMathFXEntity.h"
#include "LAMathFXUtility.h"
#include "LAMathDateCalculations.h"
#include "LAMathDefine.h"
#include "LABasic.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataMatrix.h"
#include "LADataReference.h"
#include "LADataMultiReference.h"
#include "LAPriceDataManager.h"
#include "LAPriceDataCalendar.h"
#include "LAPriceDataSlidingRule.h"
#include "LAMathYieldCurve.h"
#include "LAMathPathEntity.h"
#include "LAMathAttrSDE.h"
#include "LAModelDynamicsScalar.h"
#include "LAModelDynamicsCurve.h"
#include "LARatesSDEBase.h"

#include "LADataInstance.h"
#include <cmath>

#define USD			"USD"

#define FORWARDRATE	"FORWARDRATE"
#define FIXEDRATE	"FIXEDRATE"
#define	USEMODEL	"USEMODEL"
using namespace std;

/*!
    @brief default constructor

	@param[in] dataInstance pointer of LADataInstance object

*/
LAMathFXEntity::LAMathFXEntity(LADataInstance* dataInstance) : 
LAObject(), mFXVersion(0), mFXType(-1)
{
	setDataInstance(dataInstance);
  
	LAPriceDataManager& dm = dataInstance->getDataMaster();
	dm.setData(CALIBRATION_DATA_NAME, DATA_STRING);
	dm.setData(CALIBRATION_DATA_ASOFDATE, DATA_DATE);
	dm.setData(IR_MODEL_DATA_FXTYPE, DATA_STRING);
	dm.setData(IR_MODEL_DATA_PATHENTITY, DATA_REFERENCE);
	dm.setData(IR_MODEL_DATA_YIELDCURVES, DATA_MULTIREFERENCE);
	dm.setData(IR_MODEL_DATA_CURRENCYS, DATA_STRINGS);
	dm.setData(IR_MODEL_DATA_SPOTRATES, DATA_DOUBLES);
	dm.setData(IR_MODEL_DATA_CALENDARNAMES, DATA_STRINGS);
	
	mpName			= &add(CALIBRATION_DATA_NAME);
	mpAsOfDate		= &add(CALIBRATION_DATA_ASOFDATE);
	mpFXType		= &add(IR_MODEL_DATA_FXTYPE);
	mpPathEntity	= &add(IR_MODEL_DATA_PATHENTITY);
	mpYieldCurves	= &add(IR_MODEL_DATA_YIELDCURVES);
	mpCurrencys		= &add(IR_MODEL_DATA_CURRENCYS);
	mpSpotRates		= &add(IR_MODEL_DATA_SPOTRATES);
	mpCalendarNames	= &add(IR_MODEL_DATA_CALENDARNAMES);

}
/*!
    @brief copy constructor

	@param[in] fx LAMathFXEntity object
*/
LAMathFXEntity::LAMathFXEntity(
	const LAMathFXEntity& fx) : 
	LAObject(fx), mFXVersion(fx.mFXVersion), mCurrencyPosMap(fx.mCurrencyPosMap)
		, mFXType(fx.mFXType), mTodayRates(fx.mTodayRates), mFXSDEPosInfo(fx.mFXSDEPosInfo), mIRSDEPosInfo(fx.mIRSDEPosInfo), mDCPath(fx.mDCPath), mSpotLag(fx.mSpotLag)
{
	mpName			= &getData(CALIBRATION_DATA_NAME);
	mpAsOfDate		= &getData(CALIBRATION_DATA_ASOFDATE);
	mpFXType		= &getData(IR_MODEL_DATA_FXTYPE);
	mpPathEntity	= &getData(IR_MODEL_DATA_PATHENTITY);
	mpYieldCurves	= &getData(IR_MODEL_DATA_YIELDCURVES);
	mpCurrencys		= &getData(IR_MODEL_DATA_CURRENCYS);
	mpSpotRates		= &getData(IR_MODEL_DATA_SPOTRATES);
	mpCalendarNames	= &getData(IR_MODEL_DATA_CALENDARNAMES);

}

/*!
    @brief destructor
*/
LAMathFXEntity::~LAMathFXEntity()
{

}

// QUERY
/*!
    @brief Return this class type
	
	@return this function type
*/
object_t	
LAMathFXEntity::getType(void) const
{
	return ENTITY_FX;
}
/*!
    @brief Check function for this Object class ID
    @param[in] id ID to check Object type
    @return True or False
*/
bool
LAMathFXEntity::isTypeOf(object_t id) const
{
	return (id == ENTITY_FX ? true : LAObject::isTypeOf(id));
}
/*!
    @brief get this FX Object-name.
	@return name
*/
const LADataString&	
LAMathFXEntity::getName() const	
{
	return dynamic_cast<const LADataString&>(mpName->get());
}
/*!
    @brief get this FX Object-name.The setting of name is also possible.
	@return name
*/
LADataString&	
LAMathFXEntity::getName()
{
	return dynamic_cast<LADataString&>(mpName->get());
}
/*!
    @brief get basedate
	@return basedate
*/
const LADataDate&  
LAMathFXEntity::getAsOfDate(void) const
{
	return dynamic_cast<const LADataDate&>(mpAsOfDate->get());
}
/*!
    @brief get basedate.The setting of basedate is also possible.
	@return basedate
*/
LADataDate&  
LAMathFXEntity::getAsOfDate(void)
{
	return dynamic_cast<LADataDate&>(mpAsOfDate->get());
}
/*!
    @brief get fx type
	@return fx type
*/
const LADataString&
LAMathFXEntity::getFXType(void) const
{
	return dynamic_cast<const LADataString&>(mpFXType->get());
}
/*!
    @brief get get fx type. The setting of fx type is also possible. 
	@return fx type
*/
LADataString&
LAMathFXEntity::getFXType(void)
{
	return dynamic_cast<LADataString&>(mpFXType->get());
}
/*!
    @brief get path object
	@return path object
*/
const LADataReference&
LAMathFXEntity::getPathEntity() const
{
	return dynamic_cast<const LADataReference&>(mpPathEntity->get());
}
/*!
    @brief get path object. The setting of path object is also possible. 
	@return path object
*/
LADataReference&
LAMathFXEntity::getPathEntity()
{
	return dynamic_cast<LADataReference&>(mpPathEntity->get());
}
/*!
    @brief get currency names
	@return currency names
*/
const LADataStrings&
LAMathFXEntity::getCurrencys() const
{
	return dynamic_cast<const LADataStrings&>(mpCurrencys->get());
}
/*!
    @brief get currency names. The setting of get currency names is also possible. 
	@return currency names
*/
LADataStrings&
LAMathFXEntity::getCurrencys()
{
	return dynamic_cast<LADataStrings&>(mpCurrencys->get());
}


/*!
    @brief get yield entities
	@return yield entities
*/
const LADataMultiReference&
LAMathFXEntity::getYieldCurves() const
{
	return dynamic_cast<const LADataMultiReference&>(mpYieldCurves->get());
}
/*!
    @brief get yiled entities. The setting of yiled entities is also possible. 
	@return yield entities
*/
LADataMultiReference&
LAMathFXEntity::getYieldCurves()
{
	return dynamic_cast<LADataMultiReference&>(mpYieldCurves->get());
}
/*!
    @brief get spot rates
	@return spot rates
*/
const LADataDoubles&
LAMathFXEntity::getSpotRates() const
{
	return dynamic_cast<const LADataDoubles&>(mpSpotRates->get());
}
/*!
    @brief get spot rates. The setting of spot rates is also possible. 
	@return spot rates
*/
LADataDoubles&
LAMathFXEntity::getSpotRates()
{
	return dynamic_cast<LADataDoubles&>(mpSpotRates->get());
}
/*!
    @brief get calendar names
	@return calendar names
*/
const LADataStrings&
LAMathFXEntity::getCalendarNames() const
{
	return dynamic_cast<const LADataStrings&>(mpCalendarNames->get());
}
/*!
    @brief get calendar names. The setting of calendar names is also possible. 
	@return calendar names
*/
LADataStrings&
LAMathFXEntity::getCalendarNames()
{
	return dynamic_cast<LADataStrings&>(mpCalendarNames->get());
}

// set spot lag
void 
LAMathFXEntity::setSpotLag(const LAString &ccy, unsigned int lag)
{
	mSpotLag.insert(pair<LAString, unsigned int>(ccy, lag));
}

// get spot lag
unsigned int 
LAMathFXEntity::getSpotLag(const LAString &fx)
{
	unsigned int ret = 0;
	LAString fxcur = fx;
	fxcur.toUpper();
	LAStringVector ccys = fxcur.toToken('/');
	if (ccys.size() != 2)
		throw LACoreInvalidData("input fx error",__FILE__,__LINE__);

	std::map<LAString, unsigned int>::const_iterator it1 = mSpotLag.find(ccys[0]);
	if (it1 == mSpotLag.end())
		throw LACoreInvalidData("SpotLag Error",__FILE__,__LINE__);

	ret = it1->second;
	
	std::map<LAString, unsigned int>::const_iterator it2 = mSpotLag.find(ccys[1]);
	if (it2 == mSpotLag.end())
		throw LACoreInvalidData("SpotLag Error",__FILE__,__LINE__);

	ret = (it2->second > ret) ? it2->second : ret;
	return ret;
}



/*!
    @brief get rate (today base).
    @param[in] from from currency
    @param[in] to to currency
    @param[in] date basedate
	
	@return rate
*/
double
LAMathFXEntity::getRate(const LAString& from, const LAString& to,
								const LADate& date) const
{
	if (mFXType == 0) return getRate(from, to, 0);
	LAPriceDataDayCount dc(ACT_365_ISDA);
	double t = dc.getTerm(getAsOfDate().get(), date);
	return getRate(from, to, t, ACT_365_ISDA);
}
/*!
    @brief get rate(today base).
    @param[in] from from currency
    @param[in] to to currency
    @param[in] t basetime
    @param[in] dc daycount of t	
	@return rate
*/
double
LAMathFXEntity::getRate(const LAString& from, const LAString& to,
								const double t, DayCount dc) const
{
	if (mFXVersion != getModel()) setUp();
	
	map<LAString, unsigned int>::const_iterator it_from = mCurrencyPosMap.find(from);
	if (it_from == mCurrencyPosMap.end())
	{
		//error
		LAString msg = "Currency:";
		msg += from;
		msg += " is not registered";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);			
	}	
	map<LAString, unsigned int>::const_iterator it_to = mCurrencyPosMap.find(to);
	if (it_to == mCurrencyPosMap.end())
	{
		//error
		LAString msg = "Currency:";
		msg += to;
		msg += " is not registered";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);			
	}

	unsigned int posfrom = it_from->second;
	unsigned int posto = it_to->second;

	if (mFXType == 0)
		return mTodayRates[posfrom] / mTodayRates[posto];
	else if (mFXType == 1)
	{
		double rate = mTodayRates[posfrom] / mTodayRates[posto]; 
		if (t == 0)
			return rate;

		const LAMathYieldCurve& yield_from = 
			dynamic_cast<const LAMathYieldCurve&>(getYieldCurves().get(posfrom).get());
		const LAMathYieldCurve& yield_to = 
			dynamic_cast<const LAMathYieldCurve&>(getYieldCurves().get(posto).get());

		rate *= yield_from.getBasisDF(t)/yield_to.getBasisDF(t);
		return rate;
		/*LAMathPathYieldCurve curve_from = yield_from.getCurve(0, dc);
		LAMathPathYieldCurve curve_to = yield_to.getCurve(0, dc);
		
		return LAMathFXUtility::getForwardRate(rate, curve_from, curve_to, t);*/

	}
	else
	{
#ifndef VISUAL_STUDIO_2010_ANALYTICS
		double tmp_t;
		if (mDCPath == dc) tmp_t = t;
		else
		{
			//tmp_t =
			LAPriceDataDayCount attrdc(mDCPath);
			LAPriceDataDayCount attrdc2(dc);
			const LADate& asOf = getAsOfDate().get();
			tmp_t = attrdc.getTerm(asOf, attrdc2.getDayTerm(asOf, t));
		}

		const vector<pair<unsigned int, bool> >& fxs = mFXSDEPosInfo[posfrom][posto];
		if (fxs.size() == 0)
		{
			//error
			throw LACoreInvalidData("Some FX SDE is missing", __FILE__, __LINE__);			
		}
		
		const LAMathPathEntity& path = 	
			dynamic_cast<const LAMathPathEntity& >(getPathEntity().get().get());

		double ret = 1.0;
		for (unsigned int i = 0; i < fxs.size(); i++)
		{
//			SCALAR rate = *reinterpret_cast<const SCALAR*>
//				(dynamic_cast<const LARatesPathElementScalar&>(path.getPath(fxs[i].first, tmp_t)).get());
			SCALAR rate = dynamic_cast<const LARatesPathElementScalar&>(path.getPath(fxs[i].first, tmp_t)).get()[0];

			if (fxs[i].second == true)
				ret *= static_cast<double>(rate);
			else
				ret /= static_cast<double>(rate);
		}		
		return ret;	
#endif
	}

    // Added below line to fix a warning / bug - not all control paths return a value
    // We Should never reach here, so throw an error
    throw LACoreInvalidData("#Error: Unable to calculate the rate. Unknown error.", __FILE__, __LINE__);
}
/*!
    @brief get spot rate 
    @param[in] from from currency
    @param[in] to to currency
    @param[in] date basedate
	
	@return spot rate
*/
/*double
LAMathFXEntity::getSpotRate(const LAString& from, const LAString& to,
								const LADate& date) const
{
	LAPriceDataDayCount dc(ACT_365);
	double t = dc.getTerm(getAsOfDate().get(), date);
	return getSpotRate(from, to, t, ACT_365);
}*/
/*!
    @brief get spot rate 
    @param[in] from from currency
    @param[in] to to currency
    @param[in] t basetime
    @param[in] dc daycount of t

	@return spot rate
*/
/*double
LAMathFXEntity::getSpotRate(const LAString& from, const LAString& to,
								double t, DayCount dc) const
{
	return 0;
}*/


/*!
    @brief get forward fx rate 
    @param[in] from from currency
    @param[in] to to currency
    @param[in] date basedate
    @param[in] date forward_date
	
	@return forward fx rate 
*/
double
LAMathFXEntity::getForwardRate(const LAString& from, const LAString& to,
								const LADate& date, const LADate& forward_date) const
{
	//if (date > forward_date)
	//{
	//	//error
	//	throw LACoreInvalidData("forward_date is before date", __FILE__, __LINE__);
	//}	
	if (mFXType == 0) return getRate(from, to, 0);
	LAPriceDataDayCount dc(ACT_365_ISDA);
	double t = dc.getTerm(getAsOfDate().get(), date);
	double forward_t = dc.getTerm(getAsOfDate().get(), forward_date);
	return getForwardRate(from, to, t, forward_t, ACT_365_ISDA);
}
/*!
    @brief get forward fx rate 
    @param[in] from from currency
    @param[in] to to currency
    @param[in] t basetime
    @param[in] forward_t forward time
	
	@return forward fx rate 
*/
double
LAMathFXEntity::getForwardRate(const LAString& from, const LAString& to,
								double t, double forward_t, DayCount dc) const
{
	//if (t > forward_t)
	//{
	//	//error
	//	throw LACoreInvalidData("forward_t is before t", __FILE__, __LINE__);
	//}
	if (t < 0 || forward_t < 0)
	{
		//error
		throw LACoreInvalidData("t and forward_t must be positive", __FILE__, __LINE__);
	}
	if (mFXVersion != getModel()) setUp();

	map<LAString, unsigned int>::const_iterator it_from = mCurrencyPosMap.find(from);
	if (it_from == mCurrencyPosMap.end())
	{
		//error
		LAString msg = "Currency:";
		msg += from;
		msg += " is not registered";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);			
	}	
	map<LAString, unsigned int>::const_iterator it_to = mCurrencyPosMap.find(to);
	if (it_to == mCurrencyPosMap.end())
	{
		//error
		LAString msg = "Currency:";
		msg += to;
		msg += " is not registered";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);			
	}

	unsigned int posfrom = it_from->second;
	unsigned int posto = it_to->second;

	if (mFXType == 0)
		return mTodayRates[posfrom] / mTodayRates[posto];
	else if (mFXType == 1)
	{
		double rate = mTodayRates[posfrom] / mTodayRates[posto]; 
		if (forward_t == 0.0)
			return rate;

		const LAMathYieldCurve& yield_from = 
			dynamic_cast<const LAMathYieldCurve&>(getYieldCurves().get(posfrom).get());
		const LAMathYieldCurve& yield_to = 
			dynamic_cast<const LAMathYieldCurve&>(getYieldCurves().get(posto).get());

		rate *= yield_from.getBasisDF(forward_t)/yield_to.getBasisDF(forward_t);
		return rate;
		
		/*LAMathPathYieldCurve curve_from = yield_from.getCurve(0, dc);
		LAMathPathYieldCurve curve_to = yield_to.getCurve(0, dc);
		
		return LAMathFXUtility::getForwardRate(rate, curve_from, curve_to, forward_t);*/

	}
	else
	{
#ifndef VISUAL_STUDIO_2010_ANALYTICS
		double tmp_t, tmp_forward_t;
		if (mDCPath == dc){ tmp_t = t, tmp_forward_t = forward_t;}
		else
		{
			//tmp_t =
			LAPriceDataDayCount attrdc(mDCPath);
			LAPriceDataDayCount attrdc2(dc);
			const LADate& asOf = getAsOfDate().get();
			tmp_t = attrdc.getTerm(asOf, attrdc2.getDayTerm(asOf, t));
			tmp_forward_t = attrdc.getTerm(asOf, attrdc2.getDayTerm(asOf, forward_t));
		}		
		
		double rate = getRate(from, to, tmp_t, mDCPath);
		if (t == forward_t) return rate;

		const LAMathPathEntity& path = 	
			dynamic_cast<const LAMathPathEntity& >(getPathEntity().get().get());

		
		const LAMathYieldCurve& yield_from = 
			dynamic_cast<const LAMathYieldCurve&>(getYieldCurves().get(posfrom).get());
		LAMathPathYieldCurve curvefrom = yield_from.getCurve(t, dc);
		
		const LAMathYieldCurve& yield_to = 
		dynamic_cast<const LAMathYieldCurve&>(getYieldCurves().get(posto).get());
		LAMathPathYieldCurve curveto = yield_to.getCurve(t, dc);

		const LARatesPathElementCurve *pcurvefrom = NULL, *pcurveto = NULL;
		if (mIRSDEPosInfo[posfrom] != -1)
			pcurvefrom = &dynamic_cast<const LARatesPathElementCurve&>(path.getPath(mIRSDEPosInfo[posfrom], tmp_t));
		else
			pcurvefrom = &curvefrom; 
		if (mIRSDEPosInfo[posto] != -1)
			pcurveto = &dynamic_cast<const LARatesPathElementCurve&>(path.getPath(mIRSDEPosInfo[posto], tmp_t));
		else
			pcurveto = &curveto;
	
		return LAMathFXUtility::getForwardRate(rate, *pcurvefrom, *pcurveto, tmp_forward_t);		
#endif
	}	

    // Added below line to fix a warning / bug - not all control paths return a value
    // We Should never reach here, so throw an error
    throw LACoreInvalidData("#Error: Unable to calculate the forward rate. Unknown error.", __FILE__, __LINE__);
}

/*!
    @brief get spot date
    @param[in] cur1 currency1
    @param[in] cur2 currency2
    @param[in] basedate basedate
	
	@return spot date of currency pair of cur1 and cur2 
*/

LADate
LAMathFXEntity::getSpotDate(const LAString& cur1, const LAString& cur2,
									const LADate& basedate) const
{
	const LAStringVector& currencys = getCurrencys().get();
	int pos1 = -1, pos2 = -1;
	for (unsigned int i = 0 ; i < currencys.size(); i++)
	{
		if (pos1 == -1 && currencys[i] == cur1)
		{	
			pos1 = i;
			if (pos2 != -1) break;
			continue;
		}
		if (pos2 == -1 && currencys[i] == cur2)
		{
			pos2 = i;
			if (pos1 != -1) break;
		}		
	}

	if (pos1 == -1 || pos2 == -1)
	{
		//error
		throw LACoreInvalidData("Input currency is not registerd", __FILE__, __LINE__);
	}
	
	const LAStringVector& calendars = getCalendarNames().get();

	if (cur1 == USD)
	{
		map<LAString, unsigned int>::const_iterator it = mSpotLag.find(cur2);
		if (it == mSpotLag.end())
		{
			return LAMathFXUtility::getSpotDate_IncludedUSD(cur2, basedate, calendars[pos2], calendars[pos1]);
		}
		else
		{
			unsigned int lag = it->second;
			return LAMathFXUtility::getSpotDate_IncludedUSD(cur2, basedate, calendars[pos2], calendars[pos1], lag);
		}
	}
	else if (cur2 == USD)
	{
		map<LAString, unsigned int>::const_iterator it = mSpotLag.find(cur1);
		if (it == mSpotLag.end())
		{
			return LAMathFXUtility::getSpotDate_IncludedUSD(cur1, basedate, calendars[pos1], calendars[pos2]);
		}
		else
		{
			unsigned int lag = it->second;
			return LAMathFXUtility::getSpotDate_IncludedUSD(cur1, basedate, calendars[pos1], calendars[pos2], lag);
		}
	}
	else
	{
		int pos3 = -1;
		for (unsigned int i = 0 ; i < currencys.size(); i++)
			if (currencys[i] == USD)
			{
				pos3 = i;
				break;
			}
		if (pos3 == -1)
		{
			//error
			throw LACoreInvalidData("USD is not registerd", __FILE__, __LINE__);
		}
		map<LAString, unsigned int>::const_iterator it1 = mSpotLag.find(cur1);
		if (it1 == mSpotLag.end())
		{
			map<LAString, unsigned int>::const_iterator it2 = mSpotLag.find(cur2);
			if (it2 == mSpotLag.end())
			{
				return LAMathFXUtility::getSpotDate_NotIncludedUSD(cur1, cur2, basedate, calendars[pos1], calendars[pos2], calendars[pos3]);
			}
			else
			{
				unsigned int lag2 = it2->second;
				return LAMathFXUtility::getSpotDate_NotIncludedUSD(cur1, cur2, basedate, calendars[pos1], calendars[pos2], calendars[pos3], 0, lag2);
			}
		}
		else
		{
			unsigned int lag1 = it1->second;
			map<LAString, unsigned int>::const_iterator it2 = mSpotLag.find(cur2);
			if (it2 == mSpotLag.end())
			{
				return LAMathFXUtility::getSpotDate_NotIncludedUSD(cur1, cur2, basedate, calendars[pos1], calendars[pos2], calendars[pos3], lag1);
			}
			else
			{
				unsigned int lag2 = it2->second;
				return LAMathFXUtility::getSpotDate_NotIncludedUSD(cur1, cur2, basedate, calendars[pos1], calendars[pos2], calendars[pos3], lag1, lag2);
			}
		}
	}
}

/*!
    @brief get forward date
    @param[in] cur1 currency1
    @param[in] cur2 currency2
    @param[in] spotdate spotdate
    @param[in] term string representaion of forward term like "3M" 
	
	@return forward date
*/
/*LADate
LAMathFXEntity::getForwardDate(const LAString& cur1, const LAString& cur2,
									const LADate& spotdate, const LAString& term) const
{
	const LAStringVector& currencys = getCurrencys().get();
	int pos1 = -1, pos2 = -1;
	for (unsigned int i = 0 ; i < currencys.size(); i++)
	{
		if (pos1 == -1 && currencys[i] == cur1)
		{	
			pos1 = i;
			if (pos2 != -1) break;
			continue;
		}
		if (pos2 == -1 && currencys[i] == cur2)
		{
			pos2 = i;
			if (pos1 != -1) break;
		}		
	}

	if (pos1 == -1 || pos2 == -1)
	{
		//error
		throw LACoreInvalidData("Input currency is not registerd", __FILE__, __LINE__);
	}
	
	
	const LAStringVector& calendars = getCalendarNames().get();
	LAStringVector calnames(2);
	calnames[0] = calendars.at(pos1);
	calnames[1] = calendars.at(pos2);
	
	LAPriceDataCalendar cal(calnames);
	LAPriceDataSlidingRule srule(SLIDING_RULE_FOLLOWING);

	return LAMathDateCalculations::getDate(spotdate, term, srule, &cal, true);

}
*/	

/*!
    @brief Make copy(clone) of this FX Object object.
    @return pointer of this FX Object object.
*/
LAObject* 
LAMathFXEntity::clone() const
{
    try {
    	LAMathFXEntity*	pIRScenario = new LAMathFXEntity(*this);
    	return pIRScenario;
    }
    catch (bad_alloc & e){
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}


/*!
	@brief remove dataValues except for certain data
	@param[in] dataName data name
*/
void                
LAMathFXEntity::remove(
	const LAString& dataName)
{
	if(dataName == CALIBRATION_DATA_NAME
		|| dataName == CALIBRATION_DATA_ASOFDATE
		|| dataName == IR_MODEL_DATA_FXTYPE
		|| dataName == IR_MODEL_DATA_PATHENTITY
		|| dataName == IR_MODEL_DATA_YIELDCURVES
		|| dataName == IR_MODEL_DATA_CURRENCYS
		|| dataName == IR_MODEL_DATA_SPOTRATES
		|| dataName == IR_MODEL_DATA_CALENDARNAMES)
	{
		return; 
	}
	LAObject::remove(dataName);
}

/*!
    @brief Initialize this Object
*/
void               
LAMathFXEntity::reset(void)
{
	clear();
	mpName			= &add(CALIBRATION_DATA_NAME);
	mpAsOfDate		= &add(CALIBRATION_DATA_ASOFDATE);
	mpFXType		= &add(IR_MODEL_DATA_FXTYPE);
	mpPathEntity	= &add(IR_MODEL_DATA_PATHENTITY);
	mpYieldCurves	= &add(IR_MODEL_DATA_YIELDCURVES);
	mpCurrencys		= &add(IR_MODEL_DATA_CURRENCYS);
	mpSpotRates		= &add(IR_MODEL_DATA_SPOTRATES);
	mpCalendarNames	= &add(IR_MODEL_DATA_CALENDARNAMES);

	mFXVersion = 0;
	mCurrencyPosMap.clear();
	mFXType = -1;
	mTodayRates.clear();
	mFXSDEPosInfo.clear();
	mIRSDEPosInfo.clear();
	mSpotLag.clear();
}

/////////////// PROTECTED METHODS /////////////////////
/*!
	@brief copy LAMathFXEntity
	@param[in] e copy source
	@return reference to this object
*/
LAObject&
LAMathFXEntity::copy(
	const LAObject& e)
{
	if (this == &e) return *this;

	LAObject::copy(e);
	if (!e.isTypeOf(ENTITY_FX))
	{
		LAString err = "Assignement error for LAMathFXEntity : from ";
		err += LAString(e.getType());
		throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
	}

	mpName			= &getData(CALIBRATION_DATA_NAME);
	mpAsOfDate		= &getData(CALIBRATION_DATA_ASOFDATE);
	mpFXType		= &getData(IR_MODEL_DATA_FXTYPE);
	mpPathEntity	= &getData(IR_MODEL_DATA_PATHENTITY);
	mpYieldCurves	= &getData(IR_MODEL_DATA_YIELDCURVES);
	mpCurrencys		= &getData(IR_MODEL_DATA_CURRENCYS);
	mpSpotRates		= &getData(IR_MODEL_DATA_SPOTRATES);
	mpCalendarNames	= &getData(IR_MODEL_DATA_CALENDARNAMES);

	mFXVersion = dynamic_cast<const LAMathFXEntity&>(e).mFXVersion;
	mCurrencyPosMap = dynamic_cast<const LAMathFXEntity&>(e).mCurrencyPosMap;
	mFXType = dynamic_cast<const LAMathFXEntity&>(e).mFXType;
	mTodayRates = dynamic_cast<const LAMathFXEntity&>(e).mTodayRates;
	mFXSDEPosInfo = dynamic_cast<const LAMathFXEntity&>(e).mFXSDEPosInfo;
	mIRSDEPosInfo = dynamic_cast<const LAMathFXEntity&>(e).mIRSDEPosInfo;
	mDCPath = dynamic_cast<const LAMathFXEntity&>(e).mDCPath;
	mSpotLag = dynamic_cast<const LAMathFXEntity&>(e).mSpotLag;
	return *this;
}
/*!
	@brief add certain data
	@param[in] name name of certain data
	@return reference to holder class 
*/
LADataHolder&
LAMathFXEntity::add(const LAString& name)
{
	LADataInstance* dataInstance = getDataInstance();
	LAPriceDataManager& dm = dataInstance->getDataMaster();
	const LADataHolder& dh = dm.getData(name);
	return LAObject::add(name, dh);
}

/*!
	@brief set up this class
*/
void
LAMathFXEntity::setUp(void) const
{
	//fx type
	LAString type = getFXType().get();
	type.toUpper();
	if (type == FIXEDRATE) mFXType = 0;
	else if (type == FORWARDRATE) mFXType = 1;
	else if (type == USEMODEL) mFXType = 2;
	else
	{
		//error
		LAString msg = "FXType : ";
		msg += type;
		msg += " is not support";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	//currency position map
	mCurrencyPosMap.clear();
	const LAStringVector& currencys = getCurrencys().get();
	for (unsigned int i = 0; i < currencys.size(); i++)
		mCurrencyPosMap.insert(pair<LAString, unsigned int>(currencys[i], i));


	if (mFXType != 0)
		if (!mpCalendarNames->isDefined() || getCalendarNames().getSize() != currencys.size())
			throw LACoreInvalidData("CalendarNames size and Currencys size are not same", __FILE__, __LINE__);


	if (mFXType == 0)
	{
		if (!mpSpotRates->isDefined() || getSpotRates().getSize() != currencys.size())
			throw LACoreInvalidData("SpotRates size and Currencys size are not same", __FILE__, __LINE__);
	
		mTodayRates = getSpotRates().get();
	}
	else if (mFXType == 1)
	{
		if (!mpSpotRates->isDefined() || getSpotRates().getSize() != currencys.size())
			throw LACoreInvalidData("SpotRates size and Currencys size are not same", __FILE__, __LINE__);
		
		if (!mpYieldCurves->isDefined() || getYieldCurves().getSize() != currencys.size())
			throw LACoreInvalidData("YieldCurves size and Currencys size are not same", __FILE__, __LINE__);
		
		calcTodayRates();
	}
	else
	{
		LADataReference& ref = dynamic_cast<LADataReference&>(mpPathEntity->get());
		LAMathPathEntity& path = 	
			dynamic_cast<LAMathPathEntity& >(ref.get().get());
		mDCPath = path.getDayCount().getDayCount();
		mIRSDEPosInfo.clear();
		mIRSDEPosInfo.resize(currencys.size(), -1);
		LAStringVector sdenames = path.getSimulationSDEAttrNames().get();
		if (sdenames.size() == 0)
			sdenames = path.getSDEAttrNames().get();

//		unsigned int count = 0;
		for (unsigned int i = 0; i < sdenames.size(); i++)
		{
			const LAMathAttrSDE& sde = dynamic_cast<const LAMathAttrSDE& >(path.getData(sdenames[i], ISNOTNULL).get());
			if (sde.getSDEPathType() != IR) continue;			
			map<LAString, unsigned int>::const_iterator it = mCurrencyPosMap.find(sde.getCurrency());
			if (it == mCurrencyPosMap.end()) continue;
			mIRSDEPosInfo[it->second] = i;
//			count++;
		}
/*		if (count != currencys.size())
		{
			//error
			throw LACoreInvalidData("Some currency of Yield SDE is not exsist", __FILE__, __LINE__);
		}*/


		mFXSDEPosInfo.clear();
		mFXSDEPosInfo.resize(currencys.size());
		for (unsigned int i = 0; i < currencys.size(); i++)
			mFXSDEPosInfo[i].resize(currencys.size());
		
		vector<pair<unsigned int, bool> > vec(1);
		for (unsigned int i = 0; i < sdenames.size(); i++)
		{
			const LAMathAttrSDE& sde = dynamic_cast<const LAMathAttrSDE& >(path.getData(sdenames[i], ISNOTNULL).get());
			if (sde.getSDEPathType() != FX) continue;			
			const LAStringVector& curs = LAMathFXUtility::getCurrencyPair(sde.getCurrency());
						
			unsigned int posto = mCurrencyPosMap.find(curs[0])->second;
			unsigned int posfrom = mCurrencyPosMap.find(curs[1])->second;
				
			vec[0] = pair<unsigned int, bool>(i, true);
			mFXSDEPosInfo[posfrom][posto] = vec;
			vec[0] = pair<unsigned int, bool>(i, false);
			mFXSDEPosInfo[posto][posfrom] = vec;
		}	
		
		unsigned int count;
		do
		{
			count = 0;
			for (unsigned int i = 0; i < currencys.size(); i++)
			{			
				for (unsigned int j = 0; j < currencys.size(); j++)
				{
					if (i == j) continue;
					if (mFXSDEPosInfo[i][j].size() == 0) continue;
					for (unsigned int k = 0; k < currencys.size(); k++)
					{
						if (j == k) continue;
						if (mFXSDEPosInfo[j][k].size() == 0) continue;
						if (mFXSDEPosInfo[i][k].size() != 0) continue;
						mFXSDEPosInfo[i][k] = mFXSDEPosInfo[i][j];
						mFXSDEPosInfo[i][k].insert(mFXSDEPosInfo[i][k].end(), mFXSDEPosInfo[j][k].begin(), mFXSDEPosInfo[j][k].end());
						mFXSDEPosInfo[k][i] = mFXSDEPosInfo[i][k];
						for (unsigned int l = 0; l < mFXSDEPosInfo[k][i].size(); l++)
							mFXSDEPosInfo[k][i][l].second = !mFXSDEPosInfo[k][i][l].second; 
						count++;
					}
				}
			}		
		}
		while (count > 0);
/*		for (unsigned int i = 0; i < currencys.size(); i++)
			for (unsigned int j = 0; j < i; j++)
				if (mFXSDEPosInfo[i][j].size() == 0)
				{
					//error
					LAString msg = currencys[j];
					msg += "/" + currencys[i];
					msg += " can't convert";
					throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);				
				}
*/
	}

	if (mpSpotRates->isDefined())
	{
		DoubleArray spotRates = getSpotRates().get();
		for (unsigned int i = 0; i < spotRates.size(); ++i)
		{
			if (spotRates[i] <= 0.) throw LACoreInvalidData("spot rate is negative!", __FILE__, __LINE__);
		}
	}
	mFXVersion = getModel();
}

/*!
	@brief calulate today rates from spot rates
*/
void
LAMathFXEntity::calcTodayRates() const
{
	
	const LAMathYieldCurve& baseyield = 
		dynamic_cast<const LAMathYieldCurve& >(getYieldCurves().get(0).get());
	const LAStringVector& currencys = getCurrencys().get();
	mTodayRates.clear();
	mTodayRates.resize(currencys.size());
	mTodayRates[0] = getSpotRates().get()[0];
	const LADate& asof = getAsOfDate().get();
	for (unsigned int i = 1; i < currencys.size(); i++)
	{
		const LADate& spot = getSpotDate(currencys[0], currencys[i], asof);
		const LAMathYieldCurve& yield = 
			dynamic_cast<const LAMathYieldCurve& >(getYieldCurves().get(i).get());
		mTodayRates[i] = getSpotRates().get()[i] * baseyield.getBasisDF(asof, spot) / yield.getBasisDF(asof, spot);
	}
}


/*!
    @brief called when updating Data and the version number gets increased.
	
	
    @param[in] type Data(TYPE_NORMAL)
*/
void                
LAMathFXEntity::update(const unsigned int type) 
{
	LAObject::update(type);
    if ((type & TYPE_CACHESIZE_CHANGE) != 0x0000
		|| (type & TYPE_ANTITHETICFLAG_CHANGE) != 0x0000)
		mFXVersion = getModel();
}
