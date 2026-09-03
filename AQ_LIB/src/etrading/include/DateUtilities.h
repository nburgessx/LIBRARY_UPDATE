//
// @File: DateUtilities.h
// @Description: Utilties for handling time related functionality
// @Created: 27 Feb 2016
// @Author: Hans Roggeman
// @Department: ISO Front Office Development
//
// The copyright to the computer program(s) herein
// is the property of AlgoQuantHub.

#pragma once

// Includes: Standard Library
#include <string>
#include <vector>
#include <tuple>
#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/date_time/date_facet.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/regex.hpp>
#include <boost/assign.hpp>

#include "AQLMathCalendar.h"
#include "CoreEnumerations.h"
#include "AQLDate.h"

// Namespaces

namespace etrading
{
    // ===========================================================================================================================
    // *** Supported REGEX Date Formats ***

    // *** VERY IMPORTANT *** Please do not change the order of the DATE_REGEX list. If adding new types add to the end of the list, this is
    // becuase the "toGregorianDateFromREGEX" method in DateUtilities.cpp operates on the specific position of elements REGEX expression list.
    static const std::vector<boost::regex> DATE_REGEX  =
		boost::assign::list_of("\\d{5}")( "\\d{8}" )( "\\d{4}-\\d{2}-\\d{2}" )( "(\\d{2,4})\\/(\\d{2})(?:\\/?(\\d{2}))?" )
                              ( "\\d{4}\\\\\\d{2}\\\\\\d{2}" )( "(\\d{2})\\/(\\d{2})(?:\\/?(\\d{2,4}))?" )
                              ( "\\d{2}-\\d{2}-\\d{4}" )( "\\d{2}\\\\\\d{2}\\\\\\d{4}" );

    // ===========================================================================================================================
    
    // returns the current date and time as a string
    std::string getCurrentDateTime();

    // returns the current date as an AlgoQuantLib AQLDate
    AQLDate getCurrentMLibDate();

    // returns the current decade
    unsigned int getCurrentDecade();

    // returns the current century
    unsigned int getCurrentCentury();
    
    // returns the current time as a string
    std::string getCurrentTime();

    // converts a boost::gregorianDate to AQLDate
    AQLDate toLADateFromGregorianDate( const boost::gregorian::date& gregorian_date );
    
    // converts a vector of boost::gregorianDates to a vector of LADates
	std::vector<AQLDate> toLADatesFromGregorianDates( const std::vector<boost::gregorian::date>& gregorian_dates );

    // returns a date string "YYYYMMDD" from a gregorian::date
    std::string toYYYYMMDDFromGregorianDate( const boost::gregorian::date& gregorian_date ) ;

    // returns a date string "YYYYMMDD" from an AQLDate
    std::string toYYYYMMDDFromDate( const AQLDate& date );

	// returns a boost::gregorian::date from AQLDate
    boost::gregorian::date toGregorianDateFromLADate( const AQLDate& );
    
	// returns a boost::gregorian::date from String Date "YYYYMMDD"
    boost::gregorian::date toGregorianDateFromYYYYMMDD( const std::string& inputDate );
    
	// checks wheter the supplied can be interpreted as a date by checking the REGEX above
    bool couldBeDate( const std::string& inString );
    
	// Converts a std::string to a boost:gregorian::date using boost regular expression logic.
    // *** VERY IMPORTANT *** Please do not change the order of the DATE_REGEX list. If adding new types add to the end of the list, this is
    // becuase the "toGregorianDateFromREGEX" method in DateUtilities.cpp operates on the specific position of elements REGEX expression list.
    boost::gregorian::date toGregorianDateFromREGEX( const std::string& inString );
    
    // Converts a std::string to an AQLDate using boost regular expression logic.
    // *** VERY IMPORTANT *** Please do not change the order of the DATE_REGEX list. If adding new types add to the end of the list, this is
    // becuase the "toGregorianDateFromREGEX" method in DateUtilities.cpp operates on the specific position of elements REGEX expression list.
    AQLDate toLADateFromREGEX( const std::string& inString );

	// returns an Excel date from a boost::gregorian::date 
    const int toExcelDateFromGregorianDate( const boost::gregorian::date& gregorian_date );
    
    // returns a boost::gregorian::date from an Excel Date Integer
	boost::gregorian::date toGregorianDateFromExcelDate( int excelDate );

    double getYearFractionFromDayCount( const DayCountEnum dayCountEnum,
										const boost::gregorian::date& fromDate,
										const boost::gregorian::date& toDate,
										bool include_toDate = false );

    std::tuple<int, int, int, int> tenorStringToYMDW( const std::string& tenorString );
    
	const bool canBeTenorString(const std::string& tenorString);

    boost::gregorian::date addTenorString( const boost::gregorian::date& input, const std::string& tenorString );
    
	boost::gregorian::date addYearFraction( const boost::gregorian::date& input,
											const double yearFraction,
											const etrading::DayCountEnum dayCount = etrading::ACT_365_DAYCOUNT); // assumes ACT_365

    double getRateFromReturn( const double returnValue, const double yearFraction, const CompoundingFrequencyEnum compFreq );

    const bool isWeekend( const boost::gregorian::date& date );

    const bool isBusinessDay( const boost::gregorian::date& d, const AQLMathCalendar& cal ) ;

    const int firstNonBusinessDayIdx( const std::vector<boost::gregorian::date>& dates, const AQLMathCalendar& cal );

    boost::gregorian::date dayAdjust( const boost::gregorian::date& d,
									  const BusinessDayAdjustmentEnum busDayAdjust,
									  const AQLMathCalendar& cal );

    std::vector<boost::gregorian::date> dayAdjust( const std::vector<boost::gregorian::date>& inputDates,
                                                   const BusinessDayAdjustmentEnum bdAdj,
                                                   const AQLMathCalendar& cal );

    const bool isLeapYear( int year );

    int daysBetween( const boost::gregorian::date& startDate, const boost::gregorian::date& endDate );

    double convertRateFromAnnualTo( const double inputRate, const CompoundingFrequencyEnum targetCompoundingFreq );

	// Brief    Function to round a double to the nearest integer 
    int roundToNearest(double number);

    /* @brief			Parses a tenor string of the format "nY" and return the number of years
    *  @param [in]		tenor			The tenor string to parse
    *  @param [in]		throwOnFailure	Controls whether the function throws an exception if there is a parsing failure
	*  @return			The length of the tenor in years
    */
	unsigned int parseTenorYears( const std::string& tenor, const bool throwOnFailure );

    /* @brief			Function to populate Date and Value vectors from a DateValue matrix with 2 columns of type AQLStringMatrix. Dates must be in ascending order with no duplicates.
    *  @param [out]		dateOutput			The date results output vector
    *  @param [out]		valueOutput	        The value results output vector
    *  @param [in]		inputMatrix	        Input String Matrix: Must have 2 columns with the first column containing dates and the second containing double values
    */
    void populateDateValueVectorsFromStringMatrix( std::vector<AQLDate> & dateOutput,
                                                   std::vector<double> & valueOutput,
                                                   const AQLStringMatrix & inputMatrix );

}

