// FuturesDates.cpp
#include "FuturesDates.h"
#include "AQLDateHelpers.h"
#include "TypeHelpers.h"
#include "DateUtilities.h"
#include "ExceptionMacros.h"


namespace etrading
{
    // Function to get today's date using system date
    AQLDate todaysDate()
    {
        AQLDate today;
        today.setSystemDate();
        return today;
    }
     

    /* @brief			Function to calculate a future's start date given the futures contract ticker
    *  @param [in]		AQLString        future's ticker
    *  @return			returns a date representing the future's start date
    */
    AQLDate futureStartDate( const AQLString& futuresTicker )
    {
        // Futures Ticker must be in the format futures contract + year e.g. Dec-19 as Z9 or Z2019.
        // This means the futures ticker must be of size 2, 3 or 5 to be valid.
        if ( futuresTicker.size() != 2 && futuresTicker.size() != 3 && futuresTicker.size() != 5 )
        {
            throw AQLCoreInvalidData("#Error: Invalid futures ticker. The futures ticker must be input as contract + year. For example Dec-19 can be entered as Z9, Z19 or Z2019.", __FILE__, __LINE__);
        }

        AQLDate futuresStartDate = AQLDate();

        // Get the future's month from the ticker
        AQLString futuresContract = futuresTicker.subString( 0, 0 ).toUpper(); // Get the first character from the futuresTicker, note must be in uppercase
        unsigned int month  = etrading::AQLDateHelpers::changeFutureMonthFormat( futuresContract );
        
        // Get the future's year from the ticker
        unsigned int year   = 0;
        if ( futuresTicker.size() == 2 )
        {
            //
            // Case 1: Format: Z7 = Dec-2017
            //
            AQLString futuresYear = futuresTicker.subString( 1, 1 ); // Get the second character from the futuresTicker
            year = etrading::StringToNumber< unsigned int >( futuresYear.getCString() );

            AQLDate currentDate = etrading::getCurrentAqDate();
            unsigned int currentDecade = etrading::getCurrentDecade();

            // Get the contract year, if the year is in the past roll forwards 1 decade
            year += currentDecade;
            
            futuresStartDate = etrading::AQLDateHelpers::getFuturesContractStartDate( month, year );
            
            // Roll Contract Forward a decade if in the futures start date in the past and has expired
            if ( futuresStartDate < currentDate )
            {
                futuresStartDate = etrading::AQLDateHelpers::getFuturesContractStartDate( month, year + 10 );
            }

        }
        else if ( futuresTicker.size() == 3 )
        {
            //
            // Case 2: Format: Z17 = Dec-2017
            //
            AQLString futuresYear = futuresTicker.subString( 1, 2 ); // Get characters 2-3 from the futuresTicker
            year = etrading::StringToNumber< unsigned int >( futuresYear.getCString() );
            
            unsigned int currentCentury = etrading::getCurrentCentury();

            futuresStartDate = etrading::AQLDateHelpers::getFuturesContractStartDate( month, currentCentury + year );
        }
        else if ( futuresTicker.size() == 5 )
        {
            //
            // Case 3: Format: Z2017 = Dec-2017
            //
            AQLString futuresYear = futuresTicker.subString( 1, 4 ); // Get characters 2-5 from the futuresTicker
            year = etrading::StringToNumber< unsigned int >( futuresYear.getCString() );
            
            futuresStartDate = etrading::AQLDateHelpers::getFuturesContractStartDate( month, year );
        }
        
        
        return futuresStartDate;
    }


    // IMM Helper Utilities
    // ---------------------------------------------------------------------------------------
    
    void monthYearRollIMMForwards( MonthYear & monthAndYear, const int nMonths )
    {
        int month   = monthAndYear.month_;
        int year    = monthAndYear.year_;

        month += nMonths;

         // Check for adjustments into the Next Year
        unsigned int loopGuard = 0;
        while ( month > 12 && loopGuard < 1000 )
        {
            // Set the Correct Year and Month for a Forwards Year Roll
            // i.e. Add the Carry - Add a Year and Subtract 12 Months
            year++;
            month -= 12;
            
            loopGuard++; // Prevent accidental infinite loops
        }

        // Update the return value
        monthAndYear.month_ = month;
        monthAndYear.year_  = year;
    }
    
    void monthYearRollIMMBackwards( MonthYear & monthAndYear, const int nMonths )
    {
        int month   = monthAndYear.month_;
        int year    = monthAndYear.year_;

        month -= nMonths;

         // Check for adjustments into the Previous Year
        unsigned int loopGuard = 0;
        while ( month < 1 && loopGuard < 1000 )
        {
            // Set the Correct Year and Month for a Backwards Year Roll
            // i.e. Subtract the Carry - Subtract a Year and Add 12 Months
            year--;
            month += 12;

            loopGuard++;
        }

        // Update the return value
        monthAndYear.month_ = month;
        monthAndYear.year_  = year;
    }

    MonthYear monthYearCurrentIMM( const AQLDate& valuationDate )
    {
        MonthYear monthyear; 
        monthyear.month_   = valuationDate.monthOfYear();
        monthyear.year_    = valuationDate.yearOfEra();
        
        // IMM Months are March (H), June (), September and December (Z) i.e. Month Number is a Factor of 3
        int monthModulusThree = monthyear.month_ % 3;
        
        if ( monthModulusThree != 0)
        {
            // Adjust result to last previous IMM Month i.e. Subtract the modulus
            monthYearRollIMMBackwards( monthyear, monthModulusThree );
        }
        
        return monthyear;
    }
    // ---------------------------------------------------------------------------------------


    /* @brief			Function to calculate the current IMM Date
    *  @param [in]		AQLDate          valuationDate - valuation date the reference date for the IMM Date
    *  @param [in]		boolean         includeToday - if IMM date is today include or exclude? Defaults to false
    *  @param [in]		string          calendar - LA Flag, needed for the underlying fuction - we think this is not needed - defaults to nothing "" i.e. null string
    *  @param [in]		string          businessDayAdjustment - LA flag, needed for the underlying function - we think this is not needed - defaults to NO_CHANGE i.e. unadjusted
    *  @return			returns a date representing the future's start date
    */
    AQLDate currentIMMDate( const AQLDate& valuationDate, const bool includeToday, const std::string& calendar, const std::string businessDayAdjustment )
    {
        MonthYear currentIMM = monthYearCurrentIMM( valuationDate );
		AQLString calendarStr( calendar.c_str() );
		AQLString businessDayAdjustmentStr( businessDayAdjustment.c_str() );
        AQLDate IMMDate = AQLDateScheduleHelpers::getIMMDate1( currentIMM.year_, currentIMM.month_, calendarStr , businessDayAdjustmentStr );
        
        // Roll Backwards when the current IMM is in the future and manage the IMM that rolls on the valuation date
        if ( (IMMDate > valuationDate) || ( IMMDate == valuationDate && !includeToday) )
        {
            // Use Previous IMM, since the current IMM is in the future
            monthYearRollIMMBackwards( currentIMM, 3 );
            IMMDate = AQLDateScheduleHelpers::getIMMDate1( currentIMM.year_, currentIMM.month_, calendarStr , businessDayAdjustmentStr );
        }
        
        return IMMDate;
    }
    

    /* @brief			Function to calculate the Nth IMM Date from the Valuation Date
    *  @param [in]		AQLDate          valuationDate - valuation date the reference date for the IMM Date
    *  @param [in]		int             nthIMM - the nth IMM date to calculate
    *  @param [in]		boolean         includeToday - if IMM date is today include or exclude? Defaults to false
    *  @param [in]		string          calendar - LA Flag, needed for the underlying fuction - we think this is not needed - defaults to nothing "" i.e. null string
    *  @param [in]		string          businessDayAdjustment - LA flag, needed for the underlying function - we think this is not needed - defaults to NO_CHANGE i.e. unadjusted
    *  @return			returns a date representing the future's start date
    */
    AQLDate nthIMMDate( const AQLDate& valuationDate, const int& nthIMM, const bool includeToday, const std::string& calendar, const std::string businessDayAdjustment )
    {
        AQ_REQUIRE( nthIMM >= 0 && nthIMM <= 1000, "Invalid IMM Futures Contract - nthIMM must be between 0 and 1,000")

        // Imply the Nth IMM Month and Year from the Current IMM Month Year
        const AQLDate currentIMMReferenceDate = currentIMMDate( valuationDate, includeToday, calendar, businessDayAdjustment );
        
        // Return the result for nthIMM = 0 i.e. CurrentIMMDate
        if ( nthIMM == 0 )
        {
            return currentIMMReferenceDate;
        }

        MonthYear monthYearNthIMM = monthYearCurrentIMM( currentIMMReferenceDate );
        
        if ( nthIMM > 0 )
        {
            // IMM: Roll Forwards in 3M Increments
            int rollMonths = nthIMM * 3;
            monthYearRollIMMForwards( monthYearNthIMM, rollMonths );
        }
        else if ( nthIMM < 0 )
        {
            // IMM: Roll Forwards in 3M Decrements
            // *** NOTE *** The roll backwards function wants positive rollMonths, so we make the sign of nthIMM positive here
            int rollMonths = nthIMM * 3 * -1;
            monthYearRollIMMBackwards( monthYearNthIMM, rollMonths );
        }

        // Calculate the Nth IMM Date from the Current IMM Date
		AQLString calendarStr( calendar.c_str() );
		AQLString businessDayAdjustmentStr( businessDayAdjustment.c_str() );
        AQLDate nthIMMDate = AQLDateScheduleHelpers::getIMMDate1( monthYearNthIMM.year_, monthYearNthIMM.month_, calendarStr , businessDayAdjustmentStr );
        return nthIMMDate;
    }


    /* @brief			Function to calculate the Next IMM Date relative to the Reference Date
    *  @param [in]		AQLDate          referenceDate - the reference date for the IMM Date
    *  @param [in]		string          calendar - LA Flag, needed for the underlying fuction - we think this is not needed - defaults to nothing "" i.e. null string
    *  @param [in]		string          businessDayAdjustment - LA flag, needed for the underlying function - we think this is not needed - defaults to NO_CHANGE i.e. unadjusted
    *  @return			returns a date representing the future's start date
    */
    AQLDate nextIMMDate( const AQLDate& referenceDate, const std::string& calendar, const std::string businessDayAdjustment )
    {
        MonthYear monthYearIMM = monthYearCurrentIMM( referenceDate );
        monthYearRollIMMForwards( monthYearIMM, 3 );
		AQLString calendarStr( calendar.c_str() );
		AQLString businessDayAdjustmentStr( businessDayAdjustment.c_str() );
        AQLDate nextIMMDate = AQLDateScheduleHelpers::getIMMDate1( monthYearIMM.year_, monthYearIMM.month_, calendarStr , businessDayAdjustmentStr );
        return nextIMMDate;
    }


    /* @brief			Function to calculate the previous IMM Date relative to the Reference Date
    *  @param [in]		AQLDate          referenceDate - reference date the reference date for the IMM Date
    *  @param [in]		string          calendar - LA Flag, needed for the underlying fuction - we think this is not needed - defaults to nothing "" i.e. null string
    *  @param [in]		string          businessDayAdjustment - LA flag, needed for the underlying function - we think this is not needed - defaults to NO_CHANGE i.e. unadjusted
    *  @return			returns a date representing the future's start date
    */
    AQLDate previousIMMDate( const AQLDate& referenceDate, const std::string& calendar, const std::string businessDayAdjustment )
    {
        MonthYear monthYearIMM = monthYearCurrentIMM( referenceDate );
        monthYearRollIMMBackwards( monthYearIMM, 3 );
		AQLString calendarStr( calendar.c_str() );
		AQLString businessDayAdjustmentStr( businessDayAdjustment.c_str() );
        AQLDate previousIMMDate = AQLDateScheduleHelpers::getIMMDate1( monthYearIMM.year_, monthYearIMM.month_, calendarStr , businessDayAdjustmentStr );
        return previousIMMDate;
    }


    /* @brief			Function to provide the current IMM Futures Ticker
    *  @param [in]		AQLDate          valuationDate - valuation date the reference date for the IMM Date
    *  @param [in]		boolean         includeToday - if IMM date is today include or exclude? Defaults to false
    *  @param [in]		string          calendar - LA Flag, needed for the underlying fuction - we think this is not needed - defaults to nothing "" i.e. null string
    *  @param [in]		string          businessDayAdjustment - LA flag, needed for the underlying function - we think this is not needed - defaults to NO_CHANGE i.e. unadjusted
    *  @param [in]		bool            showYearWithTwoDigits - Show contacts with 2 year digits i.e. Z19 instead of the traditional Z9 format
    *  @return			returns a date representing the future's start date
    */
    std::string currentIMMFuturesTicker( const AQLDate& valuationDate, const bool includeToday, const std::string& calendar, const std::string businessDayAdjustment, const bool showYearWithTwoDigits )
    {
        const AQLDate futuresStartDate = currentIMMDate( valuationDate, includeToday, calendar, businessDayAdjustment );
        std::string futuresTicker = convertDateToFuturesTicker( futuresStartDate, showYearWithTwoDigits );
        return futuresTicker;
    }
    

    /* @brief			Function to provide the nth IMM Futures Ticker
    *  @param [in]		AQLDate          valuationDate - valuation date the reference date for the IMM Date
    *  @param [in]		int             nthIMM - the nth IMM date to calculate
    *  @param [in]		boolean         includeToday - if IMM date is today include or exclude? Defaults to false
    *  @param [in]		string          calendar - LA Flag, needed for the underlying fuction - we think this is not needed - defaults to nothing "" i.e. null string
    *  @param [in]		string          businessDayAdjustment - LA flag, needed for the underlying function - we think this is not needed - defaults to NO_CHANGE i.e. unadjusted
    *  @param [in]		bool            showYearWithTwoDigits - Show contacts with 2 year digits i.e. Z19 instead of the traditional Z9 format
    *  @return			returns a date representing the future's start date
    */
    std::string nthIMMFuturesTicker( const AQLDate& valuationDate, const int& nthIMM, const bool includeToday, const std::string& calendar, const std::string businessDayAdjustment, const bool showYearWithTwoDigits  )
    {
        AQ_REQUIRE( nthIMM >= 0 && nthIMM <= 1000, "Invalid IMM Futures Contract - nthIMM must be between 0 and 1,000")
        const AQLDate futuresStartDate = nthIMMDate( valuationDate, nthIMM, includeToday, calendar, businessDayAdjustment );
        std::string futuresTicker = convertDateToFuturesTicker( futuresStartDate, showYearWithTwoDigits );
        return futuresTicker;
    }


    /* @brief			Function to provide the next IMM Futures Ticker
    *  @param [in]		AQLDate          referenceDate - the reference date for the IMM Date
    *  @param [in]		string          calendar - LA Flag, needed for the underlying fuction - we think this is not needed - defaults to nothing "" i.e. null string
    *  @param [in]		string          businessDayAdjustment - LA flag, needed for the underlying function - we think this is not needed - defaults to NO_CHANGE i.e. unadjusted
    *  @param [in]		bool            showYearWithTwoDigits - Show contacts with 2 year digits i.e. Z19 instead of the traditional Z9 format
    *  @return			returns a date representing the future's start date
    */
    std::string nextIMMFuturesTicker( const AQLDate& referenceDate, const std::string& calendar, const std::string businessDayAdjustment, const bool showYearWithTwoDigits )
    {
        const AQLDate futuresStartDate = nextIMMDate( referenceDate, calendar, businessDayAdjustment );
        std::string futuresTicker = convertDateToFuturesTicker( futuresStartDate, showYearWithTwoDigits );
        return futuresTicker;
    }
    

    /* @brief			Function to provide the previous IMM Futures Ticker
    *  @param [in]		AQLDate          referenceDate - reference date the reference date for the IMM Date
    *  @param [in]		string          calendar - LA Flag, needed for the underlying fuction - we think this is not needed - defaults to nothing "" i.e. null string
    *  @param [in]		string          businessDayAdjustment - LA flag, needed for the underlying function - we think this is not needed - defaults to NO_CHANGE i.e. unadjusted
    *  @param [in]		bool            showYearWithTwoDigits - Show contacts with 2 year digits i.e. Z19 instead of the traditional Z9 format
    *  @return			returns a date representing the future's start date
    */
    std::string previousIMMFuturesTicker( const AQLDate& referenceDate, const std::string& calendar, const std::string businessDayAdjustment, const bool showYearWithTwoDigits )
    {
        const AQLDate futuresStartDate = previousIMMDate( referenceDate, calendar, businessDayAdjustment );
        std::string futuresTicker = convertDateToFuturesTicker( futuresStartDate, showYearWithTwoDigits );
        return futuresTicker;
    }

    /* @brief			Function to provide the previous IMM Futures Ticker
    *  @param [in]		AQLDate          futuresStartDate - The futures start date
    *  @param [in]		bool            showYearWithTwoDigits - Show contacts with 2 year digits i.e. Z19 instead of the traditional Z9 format
    *  @return			returns the corresponding futures ticker
    */
    std::string convertDateToFuturesTicker( const AQLDate& futuresStartDate, const bool showYearWithTwoDigits  )
    {
        // Futures Month Codes
        std::vector<std::string> monthCode = { "F", "G", "H", "J", "K", "M", "N", "Q", "U", "V", "X", "Z" };

        // Use yearModulus of 100 for 2 digit year format and 10 for single digit year format
        const int yearModulus = showYearWithTwoDigits ? 100 : 10;

        const int month     = futuresStartDate.monthOfYear();
        const int year      = futuresStartDate.yearOfEra() % yearModulus;

        AQ_REQUIRE( month > 0 && month <= 12, "Invalid Futures Start Date: The futures month must be a number in the range 1-12" )
        const std::string monthString = monthCode[ month -1 ]; // note: base zero
        const std::string yearString = std::to_string( static_cast<long long>( year ) );

        const std::string futuresTicker = monthString + yearString;
        return futuresTicker;
    }
}
