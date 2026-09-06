#include "ParameterValidation.h"

#include "AQLMarketData.h"
#include "AQLStaticData.h"
#include "AQLPriceDataCalendar.h"
#include "AQLPriceDataInterpolation.h"
#include "AQLCurveForwardRateHelpers.h"
#include "AQLDateScheduleHelpers.h"

#include "CommonConstants.h"
#include "ContainerUtilities.h"
#include "CurveValidation.h"        // getCurveAsOfDate
#include "InitializeETrading.h"
#include "AQObjUtilities.h"

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
    AQLString getDefaultValueForEmptyString( const AQLString& val, const AQLString& defVal )
    {
        if( val == AQLString( "" ) || !val.isDefined() )
        {
            return defVal;
        }
        return val;
    }

    /* @brief				Validate if a string is empty and throw if yes
    * @param [in]			str		String under validation
    * @param [in]			err		If err is not empty, throw it
    */
    void validateStringEmptiness( const AQLString& str, const AQLString& err )
    {
        if( str == AQLString( "" ) && err.size() != 0 )
        {
            throw AQLCoreInvalidData( err.getCString(), __FILE__, __LINE__ );
        }
    }

    /* @brief			check if the date is the last business day of the month
    *  @param [in]		date Given date
    *  @param [in]		cal Calendar
    */
    bool isLastDayOfMonth( const AQLDate& date, const AQLString& cal )
    {
        const AQLDate nextDay = AQLDateScheduleHelpers::getDate( date, "1d", "FOLLOWING", cal );
        return nextDay.monthOfYear() != date.monthOfYear();
    }

    /* @brief			Validate if a date is in the expected format if not throw error
    *  @param [in]		inDate  Date under validation
    *  @param [in]		err		error string
    *  @output			date with expected format
    */
    AQLDate stringToDate( const AQLString& in, const AQLString& err )
    {
        AQLString inDate( in );
        AQLDate outDate;

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
							outDate = toAQLDateFromGregorianDate( gregDate );
							return outDate;
						}
					}
					catch ( AQLCoreError& ){};
				}
                try
                {
                    outDate =  AQLDateScheduleHelpers::getAQLDate( inDate );
                
                    // Return Valid Dates only
                    if ( AQLDateScheduleHelpers::isValidDate( outDate ) )
                    {
                        return outDate;
                    }
                }
                catch( AQLCoreError& ) {}
            }

            // 2. If our date string is 8 chars in length try YYYYMMDD string format first then Excel Format
            // -------------------------------------------------------------------------
            else
            {
                // 2a) Try AlgoQuantLib String YYYYMMDD Date Format
                try
                {
                    outDate = AQLDate( inDate.getCString(), "YYYYMMDD" );

                    // Return Valid Dates only
                    if ( AQLDateScheduleHelpers::isValidDate( outDate ) )
                    {
                        return outDate;
                    }
                }
                catch( AQLCoreError& ) {}

                // 2b) Try Excel Date Format
                try
                {
                    outDate =  AQLDateScheduleHelpers::getAQLDate( inDate );
                
                    // Return Valid Dates only
                    if ( AQLDateScheduleHelpers::isValidDate( outDate ) )
                    {
                        return outDate;
                    }
                }
                catch( AQLCoreError& ) {}
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
                        
                    AQ_REQUIRE( inDate.size() == 8, "Invalid Date, unable to convert date string '" + in + "' to date. Acceptable Date String Formats: DD/MM/YYYY, YYYY/MM/DD and YYYYMMDD." );
                    try
                    {
                        outDate = AQLDate( inDate.getCString(), "DDMMYYYY" );
                    }
                    catch( AQLCoreError& )
                    {
                            
                        AQ_THROW("Invalid Date, unable to convert date string '" + in + "' to date. Acceptable Date String Formats: DD/MM/YYYY, YYYY/MM/DD and YYYYMMDD." );
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
                    AQ_THROW("Invalid Date, unable to convert date string '" + in + "' to date. Acceptable Date String Formats: DD/MM/YYYY, YYYY/MM/DD and YYYYMMDD." );
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
                    AQ_THROW("Invalid Date, unable to convert date string '" + in + "' to date. Acceptable Date String Formats: DD/MM/YYYY, YYYY/MM/DD and YYYYMMDD." );
                }

                if( inDate.size() == 7 )
                {
                    inDate.insert( 6, "0" );
                }
                else if( inDate.size() != 8 )
                {
                    AQ_THROW("Invalid Date, unable to convert date string '" + in + "' to date. Acceptable Date String Formats: DD/MM/YYYY, YYYY/MM/DD and YYYYMMDD." );
                }

                AQLDate ret_( inDate.getCString() );
                outDate = ret_;
            }

            // Check if Date is Valid and throw when Invalid
            if( !AQLDateScheduleHelpers::isValidDate( outDate ) )
            {
                if ( err.size() != 0 )
                {
                    throw AQLCoreInvalidData( err.getCString(), __FILE__, __LINE__ );
                }
                else
                {
                    AQ_THROW("Invalid Date, unable to convert date string '" + in + "' to date. Acceptable Date String Formats: DD/MM/YYYY, YYYY/MM/DD and YYYYMMDD." );
                }
            }

            return outDate;
        }

        // If we reach here we have an invalid date and we should throw and error
        AQ_THROW("Invalid Date, unable to convert date string '" + in + "' to date. Acceptable Date String Formats: DD/MM/YYYY, YYYY/MM/DD and YYYYMMDD." );
        return outDate;
    }

    /* @brief			Converts a string to a boost::gregorian::date or throws an error if the string format is invalid
    *  @param [in]		inDate  Date under validation
    *  @param [in]		err		error string
    *  @output			date with expected format
    */
    boost::gregorian::date validateAndConvertStringToGregorianDate( const AQLString & inDate, const AQLString& err )
    {
        AQLDate aqDate = stringToDate( inDate, err );
        boost::gregorian::date gregorianDate = toGregorianDateFromAQLDate( aqDate );
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
    bool isNumber( const AQLString& inputString )
    {
        std::string s = inputString.getCString();
		return isNumber(s);
    }

    /* @brief			Get AlgoQuantLib dataInstance object
    * @output			AlgoQuantLib dataInstance
    */
    AQLDataInstance* getDataInstance()
    {
        return etrading::InitializeETrading::instance( false ).dataInstance();

    }

    /* @brief			Get default calendar based on the curveCollection if calendar is empty
    * @param [in]		calendar		Calendar
    * @param [in]		curveCollection	curve collection set id
    * @output			Non-empty calendar string
    */
    AQLString getDefaultCalendarForEmptyString( const AQLString& calendar, const AQLString& curveCollection )
    {
        AQLString cal = calendar;
        if( calendar == AQLString( "" ) || calendar == nullptr )
        {
            AQLPriceDataCalendar calAttr = etrading::AQLCurveForwardRateHelpers::getYieldCurveForCurveID( getDataInstance(), curveCollection ).getCalendar();
            if ( !calAttr.isNull() )
            {
                cal  = calAttr.convertToString();
            }
            else
            {
                // Allow null calendar
                //throw AQLCoreInvalidData( "#Error: 'Calendar' must be specified.", __FILE__, __LINE__ );
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
    bool same(const AQLString& value1, const AQLString& value2)
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
		AQ_REQUIRE(numberOfRows != 0, "Unable to build the schedule. The results schedule is empty.");

		unsigned int numberOfColumns = bodyBlock[0].size();
		AQ_REQUIRE(numberOfColumns != 0, "Unable to build the schedule. The results schedule has no columns.");

		// Check header and body column number match 
		if (showColumnHeaders)
		{
			AQ_REQUIRE(headers.size() == numberOfColumns, "The column numbers of Display headers and body  not matched.");
		}
	}

	/* @brief			Validate endDate string and convert it to AQLDate. 
	* @param [in]		startDate				Start date
	* @param [in]		endDateStr				End Date in string format, can be a date or tenor
	* @param [in]		businessDayAdjustment	Business day adjustment (MOD_FOLLOWING, NO_CHANGE, etc)
	* @param [in]		calendar				Calendar
	* @param [in]		errorStr				Error string
	* @output			End date in AQLDate format
	*/
	AQLDate validateDateOrTenor(const AQLDate& startDate, const AQLString& endDateStr, const AQLString& businessDayAdjustment, const AQLString& calendar, const std::string& errorStr)
	{
		AQLDate endDate;
		bool isInputStrInDateFormat = true;
		try
		{
			endDate = stringToDate(endDateStr, errorStr);
		}
		catch (AQLCoreError&)
		{
			isInputStrInDateFormat = false;
		}

		// Check if Maturity End Date is a Tenor or Date and Convert to a Date, if required
		if (!isInputStrInDateFormat)
		{
			AQ_REQUIRE(AQLDateScheduleHelpers::isValidDate(startDate), "Invalid Date: Unable to convert tenor to a date");

			//endDateStr is a tenor
			endDate = AQLDateScheduleHelpers::getDate(startDate, endDateStr, businessDayAdjustment, calendar); 

		}

		AQ_REQUIRE(AQLDateScheduleHelpers::isValidDate(endDate), "Invalid Date " + endDateStr)
		return endDate;
	}


}