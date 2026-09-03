// FuturesDates.h
#pragma once

#include "LADate.h"
#include "LADateScheduleHelpers.h"

namespace etrading
{
    // Function to get today's date using system date
    LADate todaysDate();

    /* @brief			Function to calculate a future's start date given the futures contract ticker
    *  @param [in]		LAString        future's ticker
    *  @return			returns a date representing the future's start date
    */
    LADate futureStartDate( const LAString& futuresTicker );


    // IMM Helper Utility Methods
    // ----------------------------------------------------------------------------------------
    struct MonthYear
    {
        int month_;
        int year_;
    };
    void monthYearRollIMMForwards( MonthYear & monthAndYear, const int nMonths );
    void monthYearRollIMMBackwards( MonthYear & monthAndYear, const int nMonths );
    MonthYear monthYearCurrentIMM( const LADate& valuationDate );
    // ----------------------------------------------------------------------------------------


    /* @brief			Function to calculate the current IMM Date
    *  @param [in]		LADate          valuationDate - valuation date the reference date for the IMM Date
    *  @param [in]		boolean         includeToday - if IMM date is today include or exclude? Defaults to false
    *  @param [in]		string          calendar - LA Flag, needed for the underlying fuction - we think this is not needed - defaults to nothing "" i.e. null string
    *  @param [in]		string          businessDayAdjustment - LA flag, needed for the underlying function - we think this is not needed - defaults to NO_CHANGE i.e. unadjusted
    *  @return			returns a date representing the future's start date
    */
    LADate currentIMMDate( const LADate& valuationDate, const bool includeToday = false, const std::string& calendar = "", const std::string businessDayAdjustment = "NO_CHANGE" );
    

    /* @brief			Function to calculate the nth IMM Date
    *  @param [in]		LADate          valuationDate - valuation date the reference date for the IMM Date
    *  @param [in]		int             nthIMM - the nth IMM date to calculate
    *  @param [in]		boolean         includeToday - if IMM date is today include or exclude? Defaults to false
    *  @param [in]		string          calendar - LA Flag, needed for the underlying fuction - we think this is not needed - defaults to nothing "" i.e. null string
    *  @param [in]		string          businessDayAdjustment - LA flag, needed for the underlying function - we think this is not needed - defaults to NO_CHANGE i.e. unadjusted
    *  @return			returns a date representing the future's start date
    */
    LADate nthIMMDate( const LADate& valuationDate, const int& nthIMM, const bool includeToday = false, const std::string& calendar = "", const std::string businessDayAdjustment = "NO_CHANGE"  );


    /* @brief			Function to calculate the next IMM Date
    *  @param [in]		LADate          referenceDate - the reference date for the IMM Date
    *  @param [in]		string          calendar - LA Flag, needed for the underlying fuction - we think this is not needed - defaults to nothing "" i.e. null string
    *  @param [in]		string          businessDayAdjustment - LA flag, needed for the underlying function - we think this is not needed - defaults to NO_CHANGE i.e. unadjusted
    *  @return			returns a date representing the future's start date
    */
    LADate nextIMMDate( const LADate& referenceDate, const std::string& calendar = "", const std::string businessDayAdjustment = "NO_CHANGE"  );
    

    /* @brief			Function to calculate the previous IMM Date
    *  @param [in]		LADate          referenceDate - reference date the reference date for the IMM Date
    *  @param [in]		string          calendar - LA Flag, needed for the underlying fuction - we think this is not needed - defaults to nothing "" i.e. null string
    *  @param [in]		string          businessDayAdjustment - LA flag, needed for the underlying function - we think this is not needed - defaults to NO_CHANGE i.e. unadjusted
    *  @return			returns a date representing the future's start date
    */
    LADate previousIMMDate( const LADate& referenceDate, const std::string& calendar = "", const std::string businessDayAdjustment = "NO_CHANGE"  );
    

    /* @brief			Function to provide the current IMM Futures Ticker
    *  @param [in]		LADate          valuationDate - valuation date the reference date for the IMM Date
    *  @param [in]		boolean         includeToday - if IMM date is today include or exclude? Defaults to false
    *  @param [in]		string          calendar - LA Flag, needed for the underlying fuction - we think this is not needed - defaults to nothing "" i.e. null string
    *  @param [in]		string          businessDayAdjustment - LA flag, needed for the underlying function - we think this is not needed - defaults to NO_CHANGE i.e. unadjusted
    *  @param [in]		bool            showYearWithTwoDigits - Show contacts with 2 year digits i.e. Z19 instead of the traditional Z9 format
    *  @return			returns a date representing the future's start date
    */
    std::string currentIMMFuturesTicker( const LADate& valuationDate, const bool includeToday = false, const std::string& calendar = "", const std::string businessDayAdjustment = "NO_CHANGE", const bool showYearWithTwoDigits = false );
    

    /* @brief			Function to provide the nth IMM Futures Ticker
    *  @param [in]		LADate          valuationDate - valuation date the reference date for the IMM Date
    *  @param [in]		int             nthIMM - the nth IMM date to calculate
    *  @param [in]		boolean         includeToday - if IMM date is today include or exclude? Defaults to false
    *  @param [in]		string          calendar - LA Flag, needed for the underlying fuction - we think this is not needed - defaults to nothing "" i.e. null string
    *  @param [in]		string          businessDayAdjustment - LA flag, needed for the underlying function - we think this is not needed - defaults to NO_CHANGE i.e. unadjusted
    *  @param [in]		bool            showYearWithTwoDigits - Show contacts with 2 year digits i.e. Z19 instead of the traditional Z9 format
    *  @return			returns a date representing the future's start date
    */
    std::string nthIMMFuturesTicker( const LADate& valuationDate, const int& nthIMM, const bool includeToday = false, const std::string& calendar = "", const std::string businessDayAdjustment = "NO_CHANGE", const bool showYearWithTwoDigits = false );


    /* @brief			Function to provide the next IMM Futures Ticker
    *  @param [in]		LADate          referenceDate - the reference date for the IMM Date
    *  @param [in]		string          calendar - LA Flag, needed for the underlying fuction - we think this is not needed - defaults to nothing "" i.e. null string
    *  @param [in]		string          businessDayAdjustment - LA flag, needed for the underlying function - we think this is not needed - defaults to NO_CHANGE i.e. unadjusted
    *  @param [in]		bool            showYearWithTwoDigits - Show contacts with 2 year digits i.e. Z19 instead of the traditional Z9 format
    *  @return			returns a date representing the future's start date
    */
    std::string nextIMMFuturesTicker( const LADate& referenceDate, const std::string& calendar = "", const std::string businessDayAdjustment = "NO_CHANGE", const bool showYearWithTwoDigits = false  );
    

    /* @brief			Function to provide the previous IMM Futures Ticker
    *  @param [in]		LADate          referenceDate - reference date the reference date for the IMM Date
    *  @param [in]		string          calendar - LA Flag, needed for the underlying fuction - we think this is not needed - defaults to nothing "" i.e. null string
    *  @param [in]		string          businessDayAdjustment - LA flag, needed for the underlying function - we think this is not needed - defaults to NO_CHANGE i.e. unadjusted
    *  @param [in]		bool            showYearWithTwoDigits - Show contacts with 2 year digits i.e. Z19 instead of the traditional Z9 format
    *  @return			returns a date representing the future's start date
    */
    std::string previousIMMFuturesTicker( const LADate& referenceDate, const std::string& calendar = "", const std::string businessDayAdjustment = "NO_CHANGE", const bool showYearWithTwoDigits = false  );


    /* @brief			Function to provide the previous IMM Futures Ticker
    *  @param [in]		LADate          futuresStartDate - The futures start date
    *  @param [in]		bool            showYearWithTwoDigits - Show contacts with 2 year digits i.e. Z19 instead of the traditional Z9 format
    *  @return			returns the corresponding futures ticker
    */
    std::string convertDateToFuturesTicker( const LADate& futuresStartDate, const bool showYearWithTwoDigits = false );

}