// FuturesDates.h
#pragma once

#include "AQLDate.h"
#include "LADateScheduleHelpers.h"

namespace etrading
{
    // Function to get today's date using system date
    AQLDate todaysDate();

    /* @brief			Function to calculate a future's start date given the futures contract ticker
    *  @param [in]		AQLString        future's ticker
    *  @return			returns a date representing the future's start date
    */
    AQLDate futureStartDate( const AQLString& futuresTicker );


    // IMM Helper Utility Methods
    // ----------------------------------------------------------------------------------------
    struct MonthYear
    {
        int month_;
        int year_;
    };
    void monthYearRollIMMForwards( MonthYear & monthAndYear, const int nMonths );
    void monthYearRollIMMBackwards( MonthYear & monthAndYear, const int nMonths );
    MonthYear monthYearCurrentIMM( const AQLDate& valuationDate );
    // ----------------------------------------------------------------------------------------


    /* @brief			Function to calculate the current IMM Date
    *  @param [in]		AQLDate          valuationDate - valuation date the reference date for the IMM Date
    *  @param [in]		boolean         includeToday - if IMM date is today include or exclude? Defaults to false
    *  @param [in]		string          calendar - LA Flag, needed for the underlying fuction - we think this is not needed - defaults to nothing "" i.e. null string
    *  @param [in]		string          businessDayAdjustment - LA flag, needed for the underlying function - we think this is not needed - defaults to NO_CHANGE i.e. unadjusted
    *  @return			returns a date representing the future's start date
    */
    AQLDate currentIMMDate( const AQLDate& valuationDate, const bool includeToday = false, const std::string& calendar = "", const std::string businessDayAdjustment = "NO_CHANGE" );
    

    /* @brief			Function to calculate the nth IMM Date
    *  @param [in]		AQLDate          valuationDate - valuation date the reference date for the IMM Date
    *  @param [in]		int             nthIMM - the nth IMM date to calculate
    *  @param [in]		boolean         includeToday - if IMM date is today include or exclude? Defaults to false
    *  @param [in]		string          calendar - LA Flag, needed for the underlying fuction - we think this is not needed - defaults to nothing "" i.e. null string
    *  @param [in]		string          businessDayAdjustment - LA flag, needed for the underlying function - we think this is not needed - defaults to NO_CHANGE i.e. unadjusted
    *  @return			returns a date representing the future's start date
    */
    AQLDate nthIMMDate( const AQLDate& valuationDate, const int& nthIMM, const bool includeToday = false, const std::string& calendar = "", const std::string businessDayAdjustment = "NO_CHANGE"  );


    /* @brief			Function to calculate the next IMM Date
    *  @param [in]		AQLDate          referenceDate - the reference date for the IMM Date
    *  @param [in]		string          calendar - LA Flag, needed for the underlying fuction - we think this is not needed - defaults to nothing "" i.e. null string
    *  @param [in]		string          businessDayAdjustment - LA flag, needed for the underlying function - we think this is not needed - defaults to NO_CHANGE i.e. unadjusted
    *  @return			returns a date representing the future's start date
    */
    AQLDate nextIMMDate( const AQLDate& referenceDate, const std::string& calendar = "", const std::string businessDayAdjustment = "NO_CHANGE"  );
    

    /* @brief			Function to calculate the previous IMM Date
    *  @param [in]		AQLDate          referenceDate - reference date the reference date for the IMM Date
    *  @param [in]		string          calendar - LA Flag, needed for the underlying fuction - we think this is not needed - defaults to nothing "" i.e. null string
    *  @param [in]		string          businessDayAdjustment - LA flag, needed for the underlying function - we think this is not needed - defaults to NO_CHANGE i.e. unadjusted
    *  @return			returns a date representing the future's start date
    */
    AQLDate previousIMMDate( const AQLDate& referenceDate, const std::string& calendar = "", const std::string businessDayAdjustment = "NO_CHANGE"  );
    

    /* @brief			Function to provide the current IMM Futures Ticker
    *  @param [in]		AQLDate          valuationDate - valuation date the reference date for the IMM Date
    *  @param [in]		boolean         includeToday - if IMM date is today include or exclude? Defaults to false
    *  @param [in]		string          calendar - LA Flag, needed for the underlying fuction - we think this is not needed - defaults to nothing "" i.e. null string
    *  @param [in]		string          businessDayAdjustment - LA flag, needed for the underlying function - we think this is not needed - defaults to NO_CHANGE i.e. unadjusted
    *  @param [in]		bool            showYearWithTwoDigits - Show contacts with 2 year digits i.e. Z19 instead of the traditional Z9 format
    *  @return			returns a date representing the future's start date
    */
    std::string currentIMMFuturesTicker( const AQLDate& valuationDate, const bool includeToday = false, const std::string& calendar = "", const std::string businessDayAdjustment = "NO_CHANGE", const bool showYearWithTwoDigits = false );
    

    /* @brief			Function to provide the nth IMM Futures Ticker
    *  @param [in]		AQLDate          valuationDate - valuation date the reference date for the IMM Date
    *  @param [in]		int             nthIMM - the nth IMM date to calculate
    *  @param [in]		boolean         includeToday - if IMM date is today include or exclude? Defaults to false
    *  @param [in]		string          calendar - LA Flag, needed for the underlying fuction - we think this is not needed - defaults to nothing "" i.e. null string
    *  @param [in]		string          businessDayAdjustment - LA flag, needed for the underlying function - we think this is not needed - defaults to NO_CHANGE i.e. unadjusted
    *  @param [in]		bool            showYearWithTwoDigits - Show contacts with 2 year digits i.e. Z19 instead of the traditional Z9 format
    *  @return			returns a date representing the future's start date
    */
    std::string nthIMMFuturesTicker( const AQLDate& valuationDate, const int& nthIMM, const bool includeToday = false, const std::string& calendar = "", const std::string businessDayAdjustment = "NO_CHANGE", const bool showYearWithTwoDigits = false );


    /* @brief			Function to provide the next IMM Futures Ticker
    *  @param [in]		AQLDate          referenceDate - the reference date for the IMM Date
    *  @param [in]		string          calendar - LA Flag, needed for the underlying fuction - we think this is not needed - defaults to nothing "" i.e. null string
    *  @param [in]		string          businessDayAdjustment - LA flag, needed for the underlying function - we think this is not needed - defaults to NO_CHANGE i.e. unadjusted
    *  @param [in]		bool            showYearWithTwoDigits - Show contacts with 2 year digits i.e. Z19 instead of the traditional Z9 format
    *  @return			returns a date representing the future's start date
    */
    std::string nextIMMFuturesTicker( const AQLDate& referenceDate, const std::string& calendar = "", const std::string businessDayAdjustment = "NO_CHANGE", const bool showYearWithTwoDigits = false  );
    

    /* @brief			Function to provide the previous IMM Futures Ticker
    *  @param [in]		AQLDate          referenceDate - reference date the reference date for the IMM Date
    *  @param [in]		string          calendar - LA Flag, needed for the underlying fuction - we think this is not needed - defaults to nothing "" i.e. null string
    *  @param [in]		string          businessDayAdjustment - LA flag, needed for the underlying function - we think this is not needed - defaults to NO_CHANGE i.e. unadjusted
    *  @param [in]		bool            showYearWithTwoDigits - Show contacts with 2 year digits i.e. Z19 instead of the traditional Z9 format
    *  @return			returns a date representing the future's start date
    */
    std::string previousIMMFuturesTicker( const AQLDate& referenceDate, const std::string& calendar = "", const std::string businessDayAdjustment = "NO_CHANGE", const bool showYearWithTwoDigits = false  );


    /* @brief			Function to provide the previous IMM Futures Ticker
    *  @param [in]		AQLDate          futuresStartDate - The futures start date
    *  @param [in]		bool            showYearWithTwoDigits - Show contacts with 2 year digits i.e. Z19 instead of the traditional Z9 format
    *  @return			returns the corresponding futures ticker
    */
    std::string convertDateToFuturesTicker( const AQLDate& futuresStartDate, const bool showYearWithTwoDigits = false );

}