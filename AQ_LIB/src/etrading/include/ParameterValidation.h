#pragma once

#include "LACoreTemplateType.h"
#include "LADataInstance.h"
#include <boost/lexical_cast.hpp>
#include "LabelValueBlock.h"
#include "FixingTable.h"
#include "CoreEnumerations.h"
#include "ScheduleValidation.h"
#include <boost/date_time/gregorian/gregorian.hpp>

using etrading::LabelValueBlock;

// Macro to push back to a vector when the condition is true
#define AQ_PUSH_BACK_IF(vectorToChange, value, predicate)	\
if (predicate)											\
{														\
	vectorToChange.push_back(value);					\
}														\


#define AQ_PUSH_BACK_DATE_IF(vectorToChange, date, predicate, convertDatesToExcelFormat)	\
if (predicate)													\
{																\
	if (convertDatesToExcelFormat)								\
		vectorToChange.push_back( fromLADateToDouble( date) );	\
    else														\
		vectorToChange.push_back( std::string( date.stringWithFormat().c_str() ) );	\
}																\


namespace etrading
{

    /* @brief				return default value for empty string
    * @param [in]			val		Input value
    * @param [in]			defVal  Default value
    * @output				non-empty value
    */
    LAString getDefaultValueForEmptyString( const LAString& val, const LAString& defVal );

    /* @brief				Validate if a string is empty and throw if yes
    * @param [in]			str		String under validation
    * @param [in]			err		If err is not empty, throw it
    */
    void validateStringEmptiness( const LAString& str, const LAString& err );

    /* @brief			Converts a string to a Date or throws an error if the string format is invalid
    *  @param [in]		inDate  Date under validation
    *  @param [in]		err		error string
    *  @output			date with expected format
    */
    LADate stringToDate( const LAString& inDate, const LAString& err = LAString("") );

    /* @brief			Converts a string to a boost::gregorian::date or throws an error if the string format is invalid
    *  @param [in]		inDate  Date under validation
    *  @param [in]		err		error string
    *  @output			date with expected format
    */
    boost::gregorian::date validateAndConvertStringToGregorianDate( const LAString & inDate, const LAString& err = LAString("") );
    
    // this function returns a boolean indicating whether the input std::string can be validly converted to a type T
    template<typename T> bool isValid( const std::string& num )
    {
        bool flag = true;
        try
        {
            T tmp = boost::lexical_cast<T>( num );
        }
        catch ( boost::bad_lexical_cast& )
        {
            flag = false;
        }
        return flag;
    }

	/* @brief			Check if a string is a number
	*  @param [in]		inputString        The input string
	*  @output			boolean; True if the string is a number and False otherwise
	*/
	bool isNumber(const std::string& s);

    /* @brief			Check if a string is a number
    *  @param [in]		inputString        The input string
    *  @output			boolean; True if the string is a number and False otherwise
    */
    bool isNumber( const LAString& inputString );


    /* @brief			Get MLib dataInstance object
    * @output			MLib dataInstance
    */
    LADataInstance* getDataInstance();

    /* @brief			Get default calendar based on the curveCollection if calendar is empty
    * @param [in]		calendar		Calendar
    * @param [in]		curveCollection	curve collection set id
    * @output			Non-empty calendar string
    */
    LAString getDefaultCalendarForEmptyString( const LAString& calendar, const LAString& curveCollection );

    std::string addIndexToSchemaName(const std::string& schemaName, const int& i);

    /* @brief			Check if two string values are the same
    * @param [in]		value1	first value
    * @param [in]		value2	second value
    * @output			True if two values are the same
    */
    bool same(const LAString& value1, const LAString& value2);

    /* @brief			Check if two string values are the same
    * @param [in]		value1	first value
    * @param [in]		value2	second value
    * @output			True if two values are the same
    */
    bool same(const std::string& value1, const  std::string& value2);

	/* @brief Check if an header should be shown or not
	*
	*  @param [in] toSearch			The item to search
	*  @param [in] searchSet		The search set specified by the user, this set takes higher priority than the allowColumns
	*  @param [in] allowColumns		The pre-defined allowed columns
	*  @param [in] predicate		Default to true, when searchSet is empty, only show the headers in allowColumns with predicate as true
	*  @Return     True if the item is found
	*/
	bool includeCashflowColumn(const CashflowHeaderEnum& toSearch, const std::unordered_set<CashflowHeaderEnum,EnumClassHash>& searchSet, const std::unordered_set<CashflowHeaderEnum,EnumClassHash>& allowColumns, const bool& predicate=true);

	/* @brief Check if an item exists in the set
    *  @param [in] toSearch The item to search
    *  @Return     True if the item is found
    */
    bool includeFXPriceColumn(const FXPriceEnum& toSearch, const std::unordered_set<FXPriceEnum, EnumClassHash>& searchSet);

	/* @brief Check if the scheduleType is fixedleg or fixedbond
	*  @param [in]	legScheduleType		scheduleType
	*  @Return		True if it's fixed
	*/
	bool isFixedLeg(const ScheduleTypeEnum& legScheduleType);

	/* @brief Check if the scheduleType is float or floatBond
	*  @param [in]	legScheduleType		scheduleType
	*  @Return		True if it's float
	*/
	bool isFloatLeg(const ScheduleTypeEnum& legScheduleType);

	/* @brief Check if the scheduleType is inflation
	*  @param [in]	legScheduleType		scheduleType
	*  @Return		True if it's an inflation leg
	*/
	bool isInflationLeg( const ScheduleTypeEnum& legScheduleType );

	/* @brief Validate the display cashflow headers and body size
	*  @param [in]	showColumnHeaders	True to display headers
	*  @param [in]	headers				The display cashflow Headers
	*  @param [in]	bodyBlock			The display cashflow body
	*  @Return		True if it's float
	*/
	void validateDisplayCashflowHeaderAndBody(const bool& showColumnHeaders, const AnyTypeVector& headers, const AnyTypeMatrix& bodyBlock);

	/* @brief			Validate endDate string and convert it to LADate.
	* @param [in]		startDate				Start date
	* @param [in]		endDateStr				End Date in string format, can be a date or tenor
	* @param [in]		businessDayAdjustment	Business day adjustment (MOD_FOLLOWING, NO_CHANGE, etc)
	* @param [in]		calendar				Calendar
	* @param [in]		errorStr				Error string
	* @output			End date in LADate format
	*/
	LADate validateDateOrTenor(const LADate& startDate, const LAString& endDateStr, const LAString& businessDayAdjustment, const LAString& calendar, const std::string& errorStr);


}


