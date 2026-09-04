#include "AQLString.h"
#include "TypeUtilities.h"
#include "aqCurvesCompoundRate.h"
#include "tryAqCurvesCompoundRate.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros
#include "ParameterValidation.h"

/* @brief swig interface for aqCurvesCompoundRate method
*  @param [in]		startDates			A vector of start dates
*  @param [in]		endDates			A vector of end dates
*  @param [in]		curveCollection		ID of the curve set
*  @param [in]		forecastCurveIndex	Forecasting curve index
*  @param [in]		frequency			Frequency
*  @param [in]		spread				Spread in basis points.
*  @param [in]		stubType			StubType: None, ShortStart (SS), LongStart (LS), ShortEnd (SE) or LongEnd (LE).
*  @param [in]		rollDayInput		Roll Day conventions, e.g. ENDDATE
*  @param [in]		calendar			Calendar
*  @param [in]		businessDayAdj		Rule that decides if end date should roll forward or backward etc, ie, MF
*  @param [in]		dayCount			Day count convention
*  @param [in]		interpolation		Interpolation method
*  @param [in]		compoundType		Compound Type: NORMAL, FLAT, SIMPLE, SIMPLE, AVERAGE
*  @param [in]		firstStubDate		End date of the front stub period
*  @param [in]		lastStubDate		Start date of the end stub period
*  @return			interest rates calculated based on the startDates, endDates, and compound type
*/
std::vector<double> aqCurvesCompoundRate(const std::vector<std::string>& startDates, 
									const std::vector<std::string>& endDates, 
									const std::string& curveCollection, 
									const std::string& forecastCurveIndex, 
									const std::string& frequency, 
									double spread, 
									const std::string& stubType, 
									const std::string& rollDayInput,
									const std::string& calendar,
									const std::string& businessDayAdj,
									const std::string& dayCount,
									const std::string& interpolation,
									const std::string& compoundType,
									const std::string& firstStubDate,
									const std::string& lastStubDate)
{
    AQ_API_START
	std::vector<double> ret;
		
    // Input marshalling
	DateVector tmp_startDates;
	swig::buildDateVector(tmp_startDates, startDates);
	DateVector tmp_endDates;
	swig::buildDateVector(tmp_endDates, endDates);

	AQLString tmp_curveCollection(curveCollection.c_str());
	AQLString tmp_forecastCurveIndex(forecastCurveIndex.c_str());
	AQLString tmp_frequency(frequency.c_str());
	AQLString tmp_stubType(stubType.c_str());
	AQLString tmp_rollDayInput(rollDayInput.c_str());
	AQLString tmp_calendar(calendar.c_str());
	AQLString tmp_businessDayAdj(businessDayAdj.c_str());
	AQLString tmp_dayCount(dayCount.c_str());
	AQLString tmp_interpolation(interpolation.c_str());
	AQLString tmp_compoundType(compoundType.c_str());
	AQLString tmp_firstStubDate(firstStubDate.c_str());
	AQLString tmp_lastStubDate(lastStubDate.c_str());

	ret = validation::tryAqCurvesCompoundRate(
								tmp_startDates,
								tmp_endDates,
								tmp_curveCollection,
								tmp_forecastCurveIndex,
								tmp_frequency,
								spread,
								tmp_stubType,
								tmp_rollDayInput,
								tmp_calendar,
								tmp_businessDayAdj,
								tmp_dayCount,
								tmp_interpolation,
								tmp_compoundType,
								tmp_firstStubDate,
								tmp_lastStubDate);
	return ret;
    AQ_API_END
}

/* @brief swig interface for aqCurvesCompoundRate method
*  @param [in]		startDate			Start date
*  @param [in]		endDate				End date
*  @param [in]		curveCollection		ID of the curve set
*  @param [in]		forecastCurveIndex	Forecasting curve index
*  @param [in]		frequency			Frequency
*  @param [in]		spread				Spread in basis points.
*  @param [in]		stubType			StubType: None, ShortStart (SS), LongStart (LS), ShortEnd (SE) or LongEnd (LE).
*  @param [in]		rollDayInput		Roll Day conventions, e.g. ENDDATE
*  @param [in]		calendar			Calendar
*  @param [in]		businessDayAdj		Rule that decides if end date should roll forward or backward etc, ie, MF
*  @param [in]		dayCount			Day count convention
*  @param [in]		interpolation		Interpolation method
*  @param [in]		compoundType		Compound Type: NORMAL, FLAT, SIMPLE, SIMPLE, AVERAGE
*  @param [in]		firstStubDate		End date of the front stub period
*  @param [in]		lastStubDate		Start date of the end stub period
*  @return			compounding rate calculated based on the startDate, endDate, and compound type
*/
double aqCurvesCompoundRate(const std::string& startDate, 
							const std::string& endDate, 
							const std::string& curveCollection,
							const std::string& forecastCurveIndex,
							const std::string& frequency,
							double spread, 
							const std::string& stubType,
							const std::string& rollDayInput,
							const std::string& calendar,
							const std::string& businessDayAdj,
							const std::string& dayCount,
							const std::string& interpolation,
							const std::string& compoundType,
							const std::string& firstStubDate,
							const std::string& lastStubDate)
{
    AQ_API_START
	double ret;

    // Input marshalling
	AQLDate tmp_startDate( etrading::stringToDate( startDate ) );
	AQLDate tmp_endDate( etrading::stringToDate( endDate ) );
	AQLString tmp_curveCollection(curveCollection.c_str());
	AQLString tmp_forecastCurveIndex(forecastCurveIndex.c_str());
	AQLString tmp_frequency(frequency.c_str());
	AQLString tmp_stubType(stubType.c_str());
	AQLString tmp_rollDayInput(rollDayInput.c_str());
	AQLString tmp_calendar(calendar.c_str());
	AQLString tmp_businessDayAdj(businessDayAdj.c_str());
	AQLString tmp_dayCount(dayCount.c_str());
	AQLString tmp_interpolation(interpolation.c_str());
	AQLString tmp_compoundType(compoundType.c_str());
	AQLString tmp_firstStubDate(firstStubDate.c_str());
	AQLString tmp_lastStubDate(lastStubDate.c_str());

	ret = validation::tryAqCurvesCompoundRate(
							tmp_startDate,
							tmp_endDate,
							tmp_curveCollection,
							tmp_forecastCurveIndex,
							tmp_frequency,
							spread,
							tmp_stubType,
							tmp_rollDayInput,
							tmp_calendar,
							tmp_businessDayAdj,
							tmp_dayCount,
							tmp_interpolation,
							tmp_compoundType,
							tmp_firstStubDate,
							tmp_lastStubDate);
	return ret;
    AQ_API_END
}
