#define _HAS_STD_BYTE 0

/*
 * @brief			validation interface for mirSwapSchedule method
 * @Created:		04 April 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */
#include "tryMirSwapSchedule.h"
#include "SwapSchedule.h"
#include "CreateDataFile.h"
#include "CurveValidation.h"
#include "LabelValueBlockValidation.h"
#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"
#include "StructuredExceptionHandler.h"
#include <sstream>

namespace validation
{
    using etrading::CreateDataFile;
    using etrading::decorateFilename;

    /* @brief			validation interface for the mirSwapSchedule method
    *  @param [in]		accrualStartDate				Accrual start date
    *  @param [in]		accrualEndDateOrTenor			Accrual end date or tenor
    *  @param [in]		accrualDaycount					Accrual day count convention
    *  @param[in]		accrualFrequency				Accrual frequencey
    *  @param[in]		accrualSlidingRule				Accrual Roll Convention or Sliding Rule
    *  @param[in]		accrualCalendar					Accrual calendar
    *  @param[in]		paymentFrequency				Payment frequencey
    *  @param[in]		paymentSlidingRule				Payment Roll Convention or Sliding Rule
    *  @param[in]		paymentCalendar					Payment calendar
    *  @param[in]		paymentLag						Payment Lag
    *  @param[in]		fixingSlidingRule				Fixing Roll Convention or Sliding Rule
    *  @param[in]		fixingCalendar					Fixing calendar
    *  @param[in]		fixingLag						Fixing Lag
    *  @param[in]		fixingAdvanceOrArrears	        Fixing in advance or arrears, default to fixing in advance
    *  @param[in]		firstStub						First Odd Date, i.e. end date of the front stub period
    *  @param[in]		lastStub						Last Odd Date, i.e. start date of the end stub period
    *  @param[in]		paymentRollDay					Roll Day of Month (e.g. 9th of month )
    *  @param[in]		showColumnHeaders				True to show column headers.
    *  @return			a matrix of floading leg/fixing leg schedules
    */
    AQLStringMatrix tryMirSwapSchedule( const AQLString& accrualStartDate,
                                     const AQLString& accrualEndDateOrTenor,
                                     const AQLString& accrualDaycount,
                                     const AQLString& accrualFrequency,
                                     const AQLString& accrualSlidingRule,
                                     const AQLString& accrualCalendar,
                                     const AQLString& paymentFrequency,
                                     const AQLString& paymentSlidingRule,
                                     const AQLString& paymentCalendar,
                                     const AQLString& paymentLag,
                                     const AQLString& fixingSlidingRule,
                                     const AQLString& fixingCalendar,
                                     const AQLString& fixingLag,
                                     const AQLString& fixingAdvanceOrArrears,
                                     const AQLString& firstStub,
                                     const AQLString& lastStub,
                                     const AQLString& paymentRollDay,
                                     bool showColumnHeaders )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMirSwapSchedule_inputs" );
            file.write( "generatorFunction", "tryMirSwapSchedule" );
            file.write( "accrualStartDate", accrualStartDate );
            file.write( "accrualEndDateOrTenor", accrualEndDateOrTenor );
            file.write( "accrualDaycount", accrualDaycount );
            file.write( "accrualFrequency", accrualFrequency );
            file.write( "accrualSlidingRule", accrualSlidingRule );
            file.write( "accrualCalendar", accrualCalendar );
            file.write( "paymentFrequency", paymentFrequency );
            file.write( "paymentSlidingRule", paymentSlidingRule );
            file.write( "paymentCalendar", paymentCalendar );
            file.write( "paymentLag", paymentLag );
            file.write( "fixingSlidingRule", fixingSlidingRule );
            file.write( "fixingCalendar", fixingCalendar );
            file.write( "fixingLag", fixingLag );
            file.write( "fixingAdvanceOrArrears", fixingAdvanceOrArrears );
            file.write( "firstStub", firstStub );
            file.write( "lastStub", lastStub );
            file.write( "paymentRollDay", paymentRollDay );
        }

        etrading::validateStringEmptiness( accrualStartDate,		"#Error: The'accrualStartDate' is missing." );
        etrading::validateStringEmptiness( accrualEndDateOrTenor,	"#Error: The'accrualEndDateOrTenor' is missing." );
        etrading::validateStringEmptiness( accrualFrequency,		"#Error: The'accrualFrequency' is missing." );

        AQLDate accrualStartDt = etrading::stringToDate( accrualStartDate, "#Error: Invalid 'AccrualStartDate'." );

        AQLDate firstSt;
        if ( firstStub.size() > 0 )
        {
            firstSt = etrading::stringToDate( firstStub, "#Error: Invalid 'FirstStubDate'." );
        }

        AQLDate lastSt;
        if ( lastStub.size() > 0 )
        {
            lastSt = etrading::stringToDate( lastStub, "#Error: Invalid 'LastStubDate'." );
        }

        AQLDate accrualEndDt  = etrading::validateMaturityDate( accrualStartDt, accrualEndDateOrTenor );

        AQLString payLag( etrading::getDefaultValueForEmptyString( paymentLag, "0D" ) );
        AQLString fixLag( etrading::getDefaultValueForEmptyString( fixingLag, "0D" ) );


        // Validate the 'fixingAdvanceOrArrears'.
        bool isfixingInAdvance = true;
        if ( fixingAdvanceOrArrears.size() > 0 )
        {
            AQLString fixingAdOrAr( fixingAdvanceOrArrears );
            fixingAdOrAr.toLower();

            if ( fixingAdOrAr != "adv"
                    && fixingAdOrAr != "advance"
                    && fixingAdOrAr != "arr"
                    && fixingAdOrAr != "arrears" )
            {
                throw AQLCoreInvalidData( "#Error: Invalid Input, the 'fixingAdvanceOrArrears' parameter must be set to 'advance', 'arrears', 'adv', 'arr' or left blank.", __FILE__, __LINE__ );
            }
            else if ( fixingAdvanceOrArrears == "arrears" || fixingAdvanceOrArrears == "arr" )
            {
                isfixingInAdvance = false;
            }
        }

        bool isStartRoll = true;

        etrading::SwapSchedule swapSchedule( accrualStartDt,
                                             accrualEndDt,
                                             accrualDaycount,
                                             accrualFrequency,
                                             accrualSlidingRule,
                                             accrualCalendar,
                                             paymentFrequency,
                                             paymentSlidingRule,
                                             paymentCalendar,
                                             payLag,
                                             fixLag,
                                             fixingSlidingRule,
                                             fixingCalendar,
                                             firstSt,
                                             lastSt,
                                             paymentRollDay,
                                             isStartRoll,
                                             isfixingInAdvance );

        DoubleMatrix result = swapSchedule.getSchedule();

        unsigned int numberOfRows = result.size();
        if ( numberOfRows == 0 )
        {
            throw AQLCoreInvalidData( "#Error: Unable to build the schedule. The results schedule is empty.", __FILE__, __LINE__ );
        }

        unsigned int numberOfColumns = result[0].size();
        if ( numberOfColumns == 0 )
        {
            throw AQLCoreInvalidData( "#Error: Unable to build the schedule. The results schedule has no columns.", __FILE__, __LINE__ );
        }

        size_t COLUMN_SIZE = 5;
        if ( numberOfColumns != COLUMN_SIZE )
        {
            throw AQLCoreInvalidData( "#Error: Unable to build the schedule. The results schedule has the wrong number of columns. Must contain 5 columns, representing Fixing Date, Accrual Start Date, Accrual End Date, Accrual Period and Payment Date respectively.", __FILE__, __LINE__ );
        }

        AQLStringMatrix ret = AQLStringMatrix( 0 );
        if( showColumnHeaders )
        {
            // Insert Column Headings, if requested.
            AQLStringVector headers;
            headers.push_back( "Fixing Date" );
            headers.push_back( "Accrual Start" );
            headers.push_back( "Accrual End" );
            headers.push_back( "Accrual Period" );
            headers.push_back( "Payment Date" );

            ret.push_back( headers );
        }

        // Parse Results to Excel Dates
        for( unsigned int i = 0; i < numberOfRows; ++i )
        {
            if ( ret[i].size() < numberOfColumns )
            {
                throw AQLCoreInvalidData( "#Error: results schedule has inconstistent column sizes.", __FILE__, __LINE__ );
            }

            AQLStringVector row( 0 );
            for( unsigned int j = 0; j < numberOfColumns; ++j )
            {
                std::stringstream s;
                s << result[i][j];
                row.push_back( s.str().c_str() );
            }
            ret.push_back( row );
        }

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryMirSwapSchedule_outputs" );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }
}
