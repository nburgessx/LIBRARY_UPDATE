#include "TypeUtilities.h"

#include "aqDates.h"
#include "tryAqDates.h"
#include "ExceptionMacros.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros
#include "ParameterValidation.h"


/* @brief			swig interface for aqDatesFromTenor
*  @param [in]		startDate			The start date
*  @param [in]		tenor				Tenor added to the from date
*  @param [in]		businessDayAdj		Rule that decides if end date should roll forward or backward etc, ie, MF
*  @param [in]		calendar			Calendar
*  @param [in]		rollConvention		Roll conventions, ie, Normal, IMM, EOM, Lunar, etc
*  @return			The end date
*/
std::string aqDatesFromTenor(const std::string& startDate, 
							const std::string& tenor, 
							const std::string& businessDayAdj, 
							const std::string& calendar, 
							const std::string& rollConvention)
{
    AQ_API_START
	DateVector resultVector;
    std::string result="";

    // Input marshalling
	AQLDate tmp_startDate( etrading::stringToDate(startDate ) );
	AQLString tmp_tenor(tenor.c_str());
	AQLString tmp_businessDayAdj(businessDayAdj.c_str());
	AQLString tmp_calendar(calendar.c_str());
	AQLString tmp_rollConvention(rollConvention.c_str());

    DateVector startDateVector;
    startDateVector.push_back( tmp_startDate );
		
    resultVector = validation::tryAqDatesFromTenor( startDateVector, 
												        tmp_tenor, 
												        tmp_businessDayAdj, 
												        tmp_calendar,
												        tmp_rollConvention);
        
    AQ_REQUIRE( resultVector.size() > 0, "Empty result vector");
        
    result = resultVector[0].stringWithFormat("YYYYMMDD").getCString();
    
    return result;
	AQ_API_END
}

/* @brief			swig interface for aqDatesFromTenor
*  @param [in]		startDate			The start date
*  @param [in]		tenor				Tenor added to the from date
*  @param [in]		businessDayAdj		Rule that decides if end date should roll forward or backward etc, ie, MF
*  @param [in]		calendar			Calendar
*  @return			The end date
*/
std::string aqDatesFromTenor(const std::string& startDate, 
							const std::string& tenor, 
							const std::string& businessDayAdj, 
							const std::string& calendar)
{
    AQ_API_START
	DateVector resultVector;
    std::string result="";

    // Input marshalling
	AQLDate tmp_startDate( etrading::stringToDate(startDate ) );
	AQLString tmp_tenor(tenor.c_str());
	AQLString tmp_businessDayAdj(businessDayAdj.c_str());
	AQLString tmp_calendar(calendar.c_str());
	
    DateVector startDateVector;
    startDateVector.push_back( tmp_startDate );
		
    resultVector = validation::tryAqDatesFromTenor( startDateVector, 
												        tmp_tenor, 
												        tmp_businessDayAdj, 
												        tmp_calendar,
												        "" ); // rollConvention = ""
        
    AQ_REQUIRE( resultVector.size() > 0, "Empty result vector");
        
    result = resultVector[0].stringWithFormat("YYYYMMDD").getCString();
    
    return result;
	AQ_API_END
}

/* @brief			swig interface for aqDatesFromTenor
*  @param [in]		startDate			The start date
*  @param [in]		tenor				Tenor added to the from date
*  @return			The end date
*/
std::string aqDatesFromTenor(const std::string& startDate, 
							const std::string& tenor)
{
    AQ_API_START
	DateVector resultVector;
    std::string result="";

    // Input marshalling
	AQLDate tmp_startDate( etrading::stringToDate( startDate ) );
	AQLString tmp_tenor(tenor.c_str());
	
    DateVector startDateVector;
    startDateVector.push_back( tmp_startDate );
		
    resultVector = validation::tryAqDatesFromTenor( startDateVector, 
												       tmp_tenor, 
												       "",  // businessDayAdjustment
												       "",  // calendar
												       ""); // rollConvention
        
    AQ_REQUIRE( resultVector.size() > 0, "Empty result vector");
        
    result = resultVector[0].stringWithFormat("YYYYMMDD").getCString();
    
    return result;
	AQ_API_END
}

/* @brief			swig interface for aqDatesFromTenor
*  @param [in]		startDates			The list of start dates
*  @param [in]		tenor				Tenor added to the from date
*  @param [in]		businessDayAdj		Rule that decides if end date should roll forward or backward etc, ie, MF
*  @param [in]		calendar			Calendar
*  @param [in]		rollConvention		Roll conventions, ie, Normal, IMM, EOM, Lunar, etc
*  @return			A vector of end dates
*/
std::vector<std::string> aqDatesFromTenor( const std::vector<std::string>& startDates, 
										  const std::string& tenor, 
										  const std::string& businessDayAdj, 
										  const std::string& calendar, 
										  const std::string& rollConvention)
{
    AQ_API_START
	std::vector<std::string> resultVector;

    // Input marshalling
	DateVector tmp_startDates;
	swig::buildDateVector(tmp_startDates, startDates);

	AQLString tmp_tenor(tenor.c_str());
	AQLString tmp_businessDayAdj(businessDayAdj.c_str());
	AQLString tmp_calendar(calendar.c_str());
	AQLString tmp_rollConvention(rollConvention.c_str());

	DateVector dates = validation::tryAqDatesFromTenor( tmp_startDates, 
												            tmp_tenor, 
												            tmp_businessDayAdj, 
												            tmp_calendar,
												            tmp_rollConvention);
	for(size_t i=0; i<dates.size(); ++i)
	{
		resultVector.push_back(dates[i].stringWithFormat("YYYYMMDD").getCString());
	}		
	    
    return resultVector;
	AQ_API_END
}

/* @brief			swig interface for aqDatesFromYearFraction
*  @param [in]		startDate			Start date
*  @param [in]		yearFraction		Year fraction
*  @param [in]		dayCount			Day count convention
*  @return			The end date derived from the FromDate and given term
*/
const std::string aqDatesFromYearFraction(const std::string& startDate, 
									double yearFraction, 
									const std::string& dayCount) 
{
    AQ_API_START
	std::string ret;

    // Data type marshalling
	AQLDate tmp_startDate(etrading::stringToDate( startDate ) );
	AQLString tmp_dayCount(dayCount.c_str());

    ret = validation::tryAqDatesFromYearFraction(tmp_startDate, yearFraction, tmp_dayCount).stringWithFormat("YYYYMMDD").getCString();
    return ret;
    AQ_API_END
}

/* @brief			 swig interface for the aqDatesBusinessDays method
*  @param [in]		fromDate	    From Date
*  @param [in]		toDate			To Date
*  @param [in]		calendar		Calendar
*  @return			Number of business days between fromDate and toDate 
*/
int aqDatesBusinessDays(const std::string& fromDate, 
					const std::string& toDate,
					const std::string& calendar) 
{
    AQ_API_START
	int ret = 0;
	
    // Input marshalling
	AQLDate fromDt( etrading::stringToDate( fromDate ) );
	AQLDate toDt ( etrading::stringToDate( toDate ) );
	AQLString cal			(calendar.c_str());

	ret = validation::tryAqDatesBusinessDays(fromDt, toDt, cal);		
	return ret;
    AQ_API_END
}

/* @brief			swig interface for the aqDatesYearFraction method
*  @param [in]		fromDate	    From Date
*  @param [in]		toDate			To Date
*  @param [in]		dayCount		Day count convention
*  @param[in]		includelast		True(default):include the last day and not include start day; False:include start day and not include last day 
*  @return			Term between fromDate and toDate 
*/
double aqDatesYearFraction( const std::string& fromDate, 
						   const std::string& toDate,
						   const std::string& dayCount,
						   bool includeLast)
{
    AQ_API_START

	// Input marshalling
	AQLDate fromAQLDate( etrading::stringToDate( fromDate ) );
	AQLDate toAQLDate( etrading::stringToDate( toDate ) );

	double yearFraction = validation::tryAqDatesYearFraction( fromAQLDate, toAQLDate, dayCount, includeLast );
	return yearFraction;

    AQ_API_END
}

/* @brief			swig interface for the aqDatesYearFraction method
*  @param [in]		fromDate	    From Date
*  @param [in]		toDate			To Date
*  @param [in]		dayCount		Day count convention
*  @return			Term between fromDate and toDate 
*/
double aqDatesYearFraction( const std::string& fromDate, 
						   const std::string& toDate,
						   const std::string& dayCount )
{
    AQ_API_START

	// Input marshalling
	AQLDate fromAQLDate( etrading::stringToDate( fromDate ) );
	AQLDate toAQLDate( etrading::stringToDate( toDate ) );

	double yearFraction = validation::tryAqDatesYearFraction( fromAQLDate, toAQLDate, dayCount );
	return yearFraction;

    AQ_API_END
}


/* @brief			swig interface for the aqObjectsSwapSpotDate method
*  @param [in]		asOfDate			The asOf- or valuation-date
*  @param [in]		spotLag				The spot lag
*  @param [in]		fixingCalendar		Fixing Calendar
*  @param [in]		paymentCalendar		Payment Calendar
*  @param [in]		businessDayAdj		Business Day Adjustment
*  @param [in]		rollConvention		Roll Convention
*  @return			Swap Spot Date 
*/
std::string aqObjectsSwapUSDSpotDate( const std::string & asOfDate,
							      const std::string & spotLag,
							      const std::string & fixingCalendar,
							      const std::string & paymentCalendar,
							      const std::string & businessDayAdj,
							      const std::string & rollConvention )
{
	AQ_API_START

	// Marshall Inputs
	AQLDate asOfDate_( etrading::stringToDate( asOfDate ) );

	// Call Spot Date Method
	const AQLDate spotDate = validation::tryAqCurvesUSDSpotDate( asOfDate_, spotLag.c_str(), fixingCalendar.c_str(), paymentCalendar.c_str(), businessDayAdj.c_str(), rollConvention.c_str() );
	
	// Marshall Outputs
	const std::string spotDateString = spotDate.stringWithFormat("YYYYMMDD").c_str();
	return spotDateString;

    AQ_API_END
}

/* @brief			Method to get the curve spot date by applying multiple date shifts
*  @param [in]		asOfDate		        The curve asOfDate
*  @param [in]		fixingLag			    Tenor added to the from date
*  @param [in]		fixingCalendar		    Fixing Calendar
*  @param [in]		fixingBusDayAdj		    Rule that decides if end date should roll forward or backward etc, ie, MF
*  @param [in]		paymentLag			    Tenor added to the from date
*  @param [in]		paymentCalendar		    Fixing Calendar
*  @param [in]		paymentBusDayAdj        Rule that decides if end date should roll forward or backward etc, ie, MF
*  @return			The curve spot date
*/
std::string aqDatesShiftedSpotDate( const std::string& asOfDate,
						           const std::string& fixingLag,
						           const std::string& fixingCalendar,
                                   const std::string& fixingBusDayAdj,
                                   const std::string& paymentLag,
						           const std::string& paymentCalendar,
                                   const std::string& paymentBusDayAdj )
{
    AQ_API_START

	// Marshall Inputs
	AQLDate asOfDate_( etrading::stringToDate( asOfDate ) );

	// Call Spot Date Method
	const AQLDate spotDate = validation::tryAqDatesShiftedSpotDate( asOfDate_, fixingLag.c_str(), fixingCalendar.c_str(), fixingBusDayAdj.c_str(), paymentLag.c_str(), paymentCalendar.c_str(), paymentBusDayAdj.c_str() );

    // Marshall Outputs
	const std::string spotDateString = spotDate.stringWithFormat("YYYYMMDD").c_str();
	return spotDateString;

    AQ_API_END
}