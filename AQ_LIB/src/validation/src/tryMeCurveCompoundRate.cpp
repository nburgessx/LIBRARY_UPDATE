#include "tryMeCurveCompoundRate.h"

#include "LACurveForwardRateHelpers.h"
#include "CreateDataFile.h"
#include "CurveValidation.h"
#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"
#include "StructuredExceptionHandler.h"
#include "LADateScheduleHelpers.h"
#include "CurveUtilities.h"
#include "LWOUtilities.h"

namespace validation_api
{

    using etrading::CreateDataFile;
    using etrading::decorateFilename;

    /* @brief			validation interface for the meCurveCompoundRate method
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
    DoubleVector tryMeCurveCompoundRate( const DateVector& startDates,
                                         const DateVector& endDates,
                                         const LAString& curveCollection,
                                         const LAString& forecastCurveIndex,
                                         const LAString& frequency,
                                         double spread,
                                         const LAString& stubType,
                                         const LAString& rollDayInput,
                                         const LAString& calendar,
                                         const LAString& businessDayAdj,
                                         const LAString& dayCount,
                                         const LAString& interpolation,
                                         const LAString& compoundType,
                                         const LAString& firstStubDate,
                                         const LAString& lastStubDate )

    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateFilename( "tryMeCurveCompoundRate_inputs", curveCollection ) );
            file.write( "generatorFunction", "tryMeCurveCompoundRate" );
            file.write( "startDates", startDates );
            file.write( "endDates", endDates );
            file.write( "curveCollection", curveCollection );
            file.write( "forecastCurveIndex", forecastCurveIndex );
            file.write( "frequency", frequency );
            file.write( "spread", spread );
            file.write( "stubType", stubType );
            file.write( "rollDayInput", rollDayInput );
            file.write( "calendar", calendar );
            file.write( "businessDayAdj", businessDayAdj );
            file.write( "dayCount", dayCount );
            file.write( "interpolation", interpolation );
            file.write( "compoundType", compoundType );
            file.write( "firstStubDate", firstStubDate );
            file.write( "lastStubDate", lastStubDate );
        }

        if( startDates.size() == 0 || endDates.size() == 0 )
        {
            throw LACoreInvalidData( "#Error: a size of vector is zero.", __FILE__, __LINE__ );
        }

        if( startDates.size() != endDates.size() )
        {
            throw LACoreInvalidData( "#Error: The input startDates and endDates must be of the same size.", __FILE__, __LINE__ );
        }

        LADate* firstOddDt  = NULL;
        LADate tmpfirstOddDt;
        if ( firstStubDate.size() != 0 )
        {
            tmpfirstOddDt = etrading::stringToDate( firstStubDate, "#Error: Invalid 'firstStubDate'." );
            firstOddDt = &tmpfirstOddDt;
        }
        LADate* lastOddDt  = NULL;
        LADate tmplastOddDt;
        if ( lastStubDate.size() != 0 )
        {
            tmplastOddDt = etrading::stringToDate( lastStubDate, "#Error: Invalid 'lastStubDate'." );
            lastOddDt = &tmplastOddDt;
        }

        LAString cal = etrading::getDefaultCalendarForEmptyString( calendar, curveCollection );

        LAString rollD( etrading::getDefaultValueForEmptyString( rollDayInput, "ENDDATE" ) );
        LAString businessDayAdj( etrading::getDefaultValueForEmptyString( businessDayAdj, "MOD_FOLLOWING" ) );
        LAString dayC( etrading::getDefaultValueForEmptyString( dayCount, "ACT/360" ) );
        LAString interp( etrading::getDefaultValueForEmptyString( interpolation, "SPLINE" ) );
        LAString cType( etrading::getDefaultValueForEmptyString( compoundType, "FLAT" ) );

        DoubleVector ret;
        LADataInstance* dataInstance = etrading::getDataInstance();

        //Throw exception if the curve has not been built.
        etrading::getCurveStaticDataTableName( curveCollection, forecastCurveIndex );

        for( size_t i = 0; i < startDates.size(); i++ )
        {

            LADate startDt = startDates[i];
            LADate endDt = endDates[i];

            bool isStartRoll = etrading::isStartRollAndPopulateStubDatesFromStubType( etrading::toStubTypeEnum(stubType.getCString()), firstOddDt, lastOddDt, startDt, endDt, frequency );

            double result = etrading::LACurveForwardRateHelpers::compound( dataInstance,
                                                                      curveCollection,
                                                                      forecastCurveIndex,
                                                                      startDt,
                                                                      endDt,
                                                                      spread,
                                                                      frequency,
                                                                      isStartRoll,
                                                                      rollD,
                                                                      cal,
                                                                      businessDayAdj,
                                                                      dayC,
                                                                      interp,
                                                                      cType,
                                                                      firstOddDt,
                                                                      lastOddDt,
                                                                      true );
            ret.push_back( result );
        }


        if ( CreateDataFile::recordEnabled() )
        {

            CreateDataFile file( decorateFilename( "tryMeCurveCompoundRate_outputs", curveCollection ) );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }


    /* @brief			validation interface for the meCompoundRate method
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
    double tryMeCurveCompoundRate( const LADate& startDate,
                                   const LADate& endDate,
                                   const LAString& curveCollection,
                                   const LAString& forecastCurveIndex,
                                   const LAString& frequency,
                                   double spread,
                                   const LAString& stubType,
                                   const LAString& rollDayInput,
                                   const LAString& calendar,
                                   const LAString& businessDayAdj,
                                   const LAString& dayCount,
                                   const LAString& interpolation,
                                   const LAString& compoundType,
                                   const LAString& firstStubDate,
                                   const LAString& lastStubDate )
    {
        VALID_EXCEPTION_START

        DateVector startDateVector;
        startDateVector.push_back( startDate );
        DateVector endDateVector;
        endDateVector.push_back( endDate );

        DoubleVector ret = tryMeCurveCompoundRate( startDateVector,
                           endDateVector,
                           curveCollection,
                           forecastCurveIndex,
                           frequency,
                           spread,
                           stubType,
                           rollDayInput,
                           calendar,
                           businessDayAdj,
                           dayCount,
                           interpolation,
                           compoundType,
                           firstStubDate,
                           lastStubDate );

        return ret[0];

        VALID_EXCEPTION_END
    }


    /* @brief			validation interface for the meCurveCompoundRateWithFixingTable method
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
    *  @param [in]		fixingTableName		The cached fixing table name
    *  @param [in]		annualized  		True to return annualized rate. Default to true
    *  @return			interest rates calculated based on the startDates, endDates, and compound type
    */
    DoubleVector tryMeCurveCompoundRateWithFixingTable( const DateVector& startDates,
                                         const DateVector& endDates,
                                         const LAString& curveCollection,
                                         const LAString& forecastCurveIndex,
                                         const LAString& frequency,
                                         double spread,
                                         const LAString& stubType,
                                         const LAString& rollDayInput,
                                         const LAString& calendar,
                                         const LAString& businessDayAdj,
                                         const LAString& dayCount,
                                         const LAString& interpolation,
                                         const LAString& compoundType,
                                         const LAString& firstStubDate,
                                         const LAString& lastStubDate,
                                         const std::string& fixingTableName,
                                         bool annualized)

    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateFilename( "tryMeCurveCompoundRateWithFixingTable_inputs", curveCollection ) );
            file.write( "generatorFunction", "tryMeCurveCompoundRateWithFixingTable" );
            file.write( "startDates", startDates );
            file.write( "endDates", endDates );
            file.write( "curveCollection", curveCollection );
            file.write( "forecastCurveIndex", forecastCurveIndex );
            file.write( "frequency", frequency );
            file.write( "spread", spread );
            file.write( "stubType", stubType );
            file.write( "rollDayInput", rollDayInput );
            file.write( "calendar", calendar );
            file.write( "businessDayAdj", businessDayAdj );
            file.write( "dayCount", dayCount );
            file.write( "interpolation", interpolation );
            file.write( "compoundType", compoundType );
            file.write( "firstStubDate", firstStubDate );
            file.write( "lastStubDate", lastStubDate );
            file.write( "fixingTableName", fixingTableName );
            file.write( "annualized", annualized );
        }

        //Throw exception if the curve has not been built.
        etrading::getCurveStaticDataTableName( curveCollection, forecastCurveIndex );

		auto fixingTable = (fixingTableName.size() != 0) ? etrading::getFixingTable(fixingTableName) : etrading::FixingTablePtr();
        
		DoubleVector ret = etrading::calculateCompoundRateWithFixingTable(startDates,
										endDates,
										curveCollection,
										forecastCurveIndex,
										frequency,
										spread,
										etrading::toStubTypeEnum(stubType.getCString()),
										rollDayInput,
										calendar,
										etrading::toBusinessDayAdjustmentEnum(businessDayAdj.getCString()),
										etrading::toDayCountEnum(dayCount.getCString()),
										interpolation,
										compoundType,
										firstStubDate,
										lastStubDate,
										etrading::FALSE_BOOL,
										fixingTable,
                                         annualized);

        if ( CreateDataFile::recordEnabled() )
        {

            CreateDataFile file( decorateFilename( "tryMeCurveCompoundRateWithFixingTable_outputs", curveCollection ) );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }


    /* @brief			validation interface for the meCurveCompoundRateWithFixingTable method
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
    *  @param [in]		fixingTableName		The cached fixing table name
    *  @param [in]		annualized  		True to return annualized rate. Default to true
    *  @return			compounding rate calculated based on the startDate, endDate, and compound type
    */
    double tryMeCurveCompoundRateWithFixingTable( const LADate& startDate,
                                   const LADate& endDate,
                                   const LAString& curveCollection,
                                   const LAString& forecastCurveIndex,
                                   const LAString& frequency,
                                   double spread,
                                   const LAString& stubType,
                                   const LAString& rollDayInput,
                                   const LAString& calendar,
                                   const LAString& businessDayAdj,
                                   const LAString& dayCount,
                                   const LAString& interpolation,
                                   const LAString& compoundType,
                                   const LAString& firstStubDate,
                                   const LAString& lastStubDate, 
                                   const std::string& fixingTableName,
                                   bool annualized)
    {
        VALID_EXCEPTION_START

        DateVector startDateVector;
        startDateVector.push_back( startDate );
        DateVector endDateVector;
        endDateVector.push_back( endDate );

        DoubleVector ret = tryMeCurveCompoundRateWithFixingTable( startDateVector,
                           endDateVector,
                           curveCollection,
                           forecastCurveIndex,
                           frequency,
                           spread,
                           stubType,
                           rollDayInput,
                           calendar,
                           businessDayAdj,
                           dayCount,
                           interpolation,
                           compoundType,
                           firstStubDate,
                           lastStubDate,
                           fixingTableName);

        return ret[0];

        VALID_EXCEPTION_END
    }


}


