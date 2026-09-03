/*
 * @brief			parameter validation and helper methods used within validation_api
 * @Created:		17 March 2016
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */


#include "ParameterValidation.h"

#include "LAMarketData.h"
#include "LAStaticData.h"
#include "LAPriceDataCalendar.h"
#include "LAPriceDataInterpolation.h"
#include "LACurveForwardRateHelpers.h"
#include "LADateScheduleHelpers.h"

#include "CommonConstants.h"
#include "ContainerUtilities.h"
#include "CurveValidation.h"        // getCurveAsOfDate
#include "InitializeMLibETrading.h"
#include "LWOUtilities.h"

#include <cctype>
#include <utility>
#include <boost/algorithm/string.hpp>


namespace etrading
{

    /* @brief				return default value for empty string
    * @param [in]			val		Input value
    * @param [in]			defVal  Default value
    * @output				non-empty value
    */
    LAString getDefaultValueForEmptyString( const LAString& val, const LAString& defVal )
    {
        if( val == LAString( "" ) || !val.isDefined() )
        {
            return defVal;
        }
        return val;
    }

    /* @brief				Validate if a string is empty and throw if yes
    * @param [in]			str		String under validation
    * @param [in]			err		If err is not empty, throw it
    */
    void validateStringEmptiness( const LAString& str, const LAString& err )
    {
        if( str == LAString( "" ) && err.size() != 0 )
        {
            throw LACoreInvalidData( err.getCString(), __FILE__, __LINE__ );
        }
    }

    /* @brief			check if the date is the last business day of the month
    *  @param [in]		date Given date
    *  @param [in]		cal Calendar
    */
    bool isLastDayOfMonth( const LADate& date, const LAString& cal )
    {
        const LADate nextDay = LADateScheduleHelpers::getDate( date, "1d", "FOLLOWING", cal );
        return nextDay.monthOfYear() != date.monthOfYear();
    }

    /* @brief			Validate if a date is in the expected format if not throw error
    *  @param [in]		inDate  Date under validation
    *  @param [in]		err		error string
    *  @output			date with expected format
    */
    LADate stringToDate( const LAString& in, const LAString& err )
    {
        LAString inDate( in );
        LADate outDate;

        int forwardSlashCheck = inDate.findString( "/" );


        // Skip straight to Step 3. if our date string contains a forward slash
        // -------------------------------------------------------------------------
        if( forwardSlashCheck == -1 )
        {
            // 1. If our date string is not!!! 8 chars in length try ISO and Excel Format only
            if( inDate.size() != 8 )
            {
				if (inDate.size() == 10)
				{
					// For example YYYY-MM-DD
					// Ask boost to convert the string to date
					try
					{
						if (couldBeDate( inDate.c_str() ))
						{
							auto gregDate = toGregorianDateFromREGEX( inDate.c_str() );
							outDate = toLADateFromGregorianDate( gregDate );
							return outDate;
						}
					}
					catch ( LACoreError& ){};
				}
                try
                {
                    outDate =  LADateScheduleHelpers::getLADate( inDate );
                
                    // Return Valid Dates only
                    if ( LADateScheduleHelpers::isValidDate( outDate ) )
                    {
                        return outDate;
                    }
                }
                catch( LACoreError& ) {}
            }

            // 2. If our date string is 8 chars in length try YYYYMMDD string format first then Excel Format
            // -------------------------------------------------------------------------
            else
            {
                // 2a) Try MLIBQ String YYYYMMDD Date Format
                try
                {
                    outDate = LADate( inDate.getCString(), "YYYYMMDD" );

                    // Return Valid Dates only
                    if ( LADateScheduleHelpers::isValidDate( outDate ) )
                    {
                        return outDate;
                    }
                }
                catch( LACoreError& ) {}

                // 2b) Try Excel Date Format
                try
                {
                    outDate =  LADateScheduleHelpers::getLADate( inDate );
                
                    // Return Valid Dates only
                    if ( LADateScheduleHelpers::isValidDate( outDate ) )
                    {
                        return outDate;
                    }
                }
                catch( LACoreError& ) {}
            }
        }

        // 3. Try Cast Date String from other String formats containing a forward slash '/' symbol
        // ----------------------------------------------------------
        if ( forwardSlashCheck != -1 )
        {
            // Check for Date String Format: DD/MM/YYYY
            if ( forwardSlashCheck == 2)
            {
                inDate.remove( 2, 1 );
                    
                forwardSlashCheck = inDate.findString( "/" );
                if ( forwardSlashCheck == 4 )
                {
                    inDate.remove( 4, 1 );
                        
                    MLIB_REQUIRE( inDate.size() == 8, "Invalid Date, unable to convert date string '" + in + "' to date. Acceptable Date String Formats: DD/MM/YYYY, YYYY/MM/DD and YYYYMMDD." );
                    try
                    {
                        outDate = LADate( inDate.getCString(), "DDMMYYYY" );
                    }
                    catch( LACoreError& )
                    {
                            
                        MLIB_THROW("Invalid Date, unable to convert date string '" + in + "' to date. Acceptable Date String Formats: DD/MM/YYYY, YYYY/MM/DD and YYYYMMDD." );
                    }
                }
            }
            else
            {
                // Check for Date String Formats: YYYY/MM/DD and YYYY/M/DD
                if( forwardSlashCheck == 4 )
                {
                    inDate.remove( 4, 1 );
                }
                else
                {
                    MLIB_THROW("Invalid Date, unable to convert date string '" + in + "' to date. Acceptable Date String Formats: DD/MM/YYYY, YYYY/MM/DD and YYYYMMDD." );
                }

                forwardSlashCheck = inDate.findString( "/" );
                if( forwardSlashCheck == 6 )
                {
                    inDate.remove( 6, 1 );
                }
                else if( forwardSlashCheck == 5 )
                {
                    inDate.remove( 5, 1 );
                    inDate.insert( 4, "0" );
                }
                else
                {
                    MLIB_THROW("Invalid Date, unable to convert date string '" + in + "' to date. Acceptable Date String Formats: DD/MM/YYYY, YYYY/MM/DD and YYYYMMDD." );
                }

                if( inDate.size() == 7 )
                {
                    inDate.insert( 6, "0" );
                }
                else if( inDate.size() != 8 )
                {
                    MLIB_THROW("Invalid Date, unable to convert date string '" + in + "' to date. Acceptable Date String Formats: DD/MM/YYYY, YYYY/MM/DD and YYYYMMDD." );
                }

                LADate ret_( inDate.getCString() );
                outDate = ret_;
            }

            // Check if Date is Valid and throw when Invalid
            if( !LADateScheduleHelpers::isValidDate( outDate ) )
            {
                if ( err.size() != 0 )
                {
                    throw LACoreInvalidData( err.getCString(), __FILE__, __LINE__ );
                }
                else
                {
                    MLIB_THROW("Invalid Date, unable to convert date string '" + in + "' to date. Acceptable Date String Formats: DD/MM/YYYY, YYYY/MM/DD and YYYYMMDD." );
                }
            }

            return outDate;
        }

        // If we reach here we have an invalid date and we should throw and error
        MLIB_THROW("Invalid Date, unable to convert date string '" + in + "' to date. Acceptable Date String Formats: DD/MM/YYYY, YYYY/MM/DD and YYYYMMDD." );
        return outDate;
    }

    /* @brief			Converts a string to a boost::gregorian::date or throws an error if the string format is invalid
    *  @param [in]		inDate  Date under validation
    *  @param [in]		err		error string
    *  @output			date with expected format
    */
    boost::gregorian::date validateAndConvertStringToGregorianDate( const LAString & inDate, const LAString& err )
    {
        LADate laDate = stringToDate( inDate, err );
        boost::gregorian::date gregorianDate = toGregorianDateFromLADate( laDate );
        return gregorianDate;
    }

	/* @brief			Check if a string is a number
	*  @param [in]		inputString        The input string
	*  @output			boolean; True if the string is a number and False otherwise
	*/
	bool isNumber(const std::string& s)
	{
		std::string::const_iterator it = s.begin();
		// interate over and check if every charater is numeric
		while (it != s.end() && std::isdigit(*it))
		{
			++it;
		}
		return !s.empty() && it == s.end();
	}

    /* @brief			Check if a string is a number
    *  @param [in]		inputString        The input string
    *  @output			boolean; True if the string is a number and False otherwise
    */
    bool isNumber( const LAString& inputString )
    {
        std::string s = inputString.getCString();
		return isNumber(s);
    }

    /* @brief			Get MLIBQ dataInstance object
    * @output			MLIBQ dataInstance
    */
    LADataInstance* getDataInstance()
    {
        return etrading::InitializeMLibETrading::instance( false ).dataInstance();

    }

    /* @brief			Get default calendar based on the curveCollection if calendar is empty
    * @param [in]		calendar		Calendar
    * @param [in]		curveCollection	curve collection set id
    * @output			Non-empty calendar string
    */
    LAString getDefaultCalendarForEmptyString( const LAString& calendar, const LAString& curveCollection )
    {
        LAString cal = calendar;
        if( calendar == LAString( "" ) || calendar == nullptr )
        {
            LAPriceDataCalendar calAttr = etrading::LACurveForwardRateHelpers::getYieldCurveForCurveID( getDataInstance(), curveCollection ).getCalendar();
            if ( !calAttr.isNull() )
            {
                cal  = calAttr.convertToString();
            }
            else
            {
                // Allow null calendar
                //throw LACoreInvalidData( "#Error: 'Calendar' must be specified.", __FILE__, __LINE__ );
            }
        }
        return cal;
    }

    std::string addIndexToSchemaName(const std::string& schemaName, const int& i)
    {
		std::string name = schemaName;
		if (i >= 0)
		{
			name = schemaName + "_" + boost::lexical_cast<std::string>(i);
		}
        return name;
    }


    /* @brief			Check if two string values are the same
    * @param [in]		value1	first value
    * @param [in]		value2	second value
    * @output			True if two values are the same
    */
    bool same(const LAString& value1, const LAString& value2)
    {
        return boost::iequals(value1.getCString(), value2.getCString());
    }

	/* @brief			Check if two string values are the same
    * @param [in]		value1	first value
    * @param [in]		value2	second value
    * @output			True if two values are the same
    */
	bool same(const std::string& value1, const  std::string& value2)
	{
        return boost::iequals(value1.c_str(), value2.c_str());
	}

   	/* @brief Check if an header should be shown or not
	*
    *  @param [in] toSearch			The item to search
	*  @param [in] searchSet		The search set specified by the user, this set takes higher priority than the allowColumns
	*  @param [in] allowColumns		The pre-defined allowed columns
	*  @param [in] predicate		Default to true, when searchSet is empty, only show the headers in allowColumns with predicate as true
	*  @Return     True if the item is found
    */
    bool includeCashflowColumn(const CashflowHeaderEnum& toSearch, const std::unordered_set<CashflowHeaderEnum,EnumClassHash>& searchSet, const std::unordered_set<CashflowHeaderEnum,EnumClassHash>& allowColumns, const bool& predicate)
    {
		bool includedCol = false;
		if (searchSet.empty() && predicate)
		{
			// Show headers in allowColumns
			includedCol = (allowColumns.find(toSearch) != allowColumns.end());
		}
		else 
		{
			// Show headers in searchSet only
			includedCol = (searchSet.find(toSearch) != searchSet.end());
		}
        return includedCol;
    }

   	/* @brief Check if an item exists in the set
    *  @param [in] toSearch The item to search
    *  @Return     True if the item is found
    */
    bool includeFXPriceColumn(const FXPriceEnum& toSearch, const std::unordered_set<FXPriceEnum, EnumClassHash>& searchSet)
    {
        return searchSet.empty() || (searchSet.find(toSearch) != searchSet.end());
    }

	/* @brief Check if the scheduleType is fixed or fixedBond
	*  @param [in]	legScheduleType		scheduleType
	*  @Return		True if it's fixed
	*/
	bool isFixedLeg(const ScheduleTypeEnum& scheduleType)
	{
		return scheduleType == FIXED_SCHEDULE_TYPE || scheduleType == SWAPSCHEDULE_FIXEDBOND;
	}

	/* @brief Check if the scheduleType is float or floatBond
	*  @param [in]	legScheduleType		scheduleType
	*  @Return		True if it's float
	*/
	bool isFloatLeg(const ScheduleTypeEnum& scheduleType)
	{
		return scheduleType == FLOAT_SCHEDULE_TYPE || scheduleType == SWAPSCHEDULE_FLOATBOND ;
	}

	/* @brief Check if the scheduleType is inflation
	*  @param [in]	legScheduleType		scheduleType
	*  @Return		True if it's an inflation leg
	*/
	bool isInflationLeg( const ScheduleTypeEnum& scheduleType )
	{
		return scheduleType == INFLATION_SCHEDULE_TYPE;
	}

	/* @brief Validate the display cashflow headers and body size 
	*  @param [in]	showColumnHeaders	True to display headers
	*  @param [in]	headers				The display cashflow Headers
	*  @param [in]	bodyBlock			The display cashflow body
	*  @Return		True if it's float
	*/
	void validateDisplayCashflowHeaderAndBody(const bool& showColumnHeaders, const AnyTypeVector& headers, const AnyTypeMatrix& bodyBlock)
	{
		unsigned int numberOfRows = bodyBlock.size();
		MLIB_REQUIRE(numberOfRows != 0, "Unable to build the schedule. The results schedule is empty.");

		unsigned int numberOfColumns = bodyBlock[0].size();
		MLIB_REQUIRE(numberOfColumns != 0, "Unable to build the schedule. The results schedule has no columns.");

		// Check header and body column number match 
		if (showColumnHeaders)
		{
			MLIB_REQUIRE(headers.size() == numberOfColumns, "The column numbers of Display headers and body  not matched.");
		}
	}

	/* @brief			Validate endDate string and convert it to LADate. 
	* @param [in]		startDate				Start date
	* @param [in]		endDateStr				End Date in string format, can be a date or tenor
	* @param [in]		businessDayAdjustment	Business day adjustment (MOD_FOLLOWING, NO_CHANGE, etc)
	* @param [in]		calendar				Calendar
	* @param [in]		errorStr				Error string
	* @output			End date in LADate format
	*/
	LADate validateDateOrTenor(const LADate& startDate, const LAString& endDateStr, const LAString& businessDayAdjustment, const LAString& calendar, const std::string& errorStr)
	{
		LADate endDate;
		bool isInputStrInDateFormat = true;
		try
		{
			endDate = stringToDate(endDateStr, errorStr);
		}
		catch (LACoreError&)
		{
			isInputStrInDateFormat = false;
		}

		// Check if Maturity End Date is a Tenor or Date and Convert to a Date, if required
		if (!isInputStrInDateFormat)
		{
			MLIB_REQUIRE(LADateScheduleHelpers::isValidDate(startDate), "Invalid Date: Unable to convert tenor to a date");

			//endDateStr is a tenor
			endDate = LADateScheduleHelpers::getDate(startDate, endDateStr, businessDayAdjustment, calendar); 

		}

		MLIB_REQUIRE(LADateScheduleHelpers::isValidDate(endDate), "Invalid Date " + endDateStr)
		return endDate;
	}


}