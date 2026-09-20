#include "tryAqCurveCompoundRate.h"

#include "RecordMacros.h"
#include "AQLCurveForwardRateHelpers.h"
#include "CreateDataFile.h"
#include "CurveValidation.h"
#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"
#include "StructuredExceptionHandler.h"
#include "AQLDateSchedule.h"
#include "CurveUtilities.h"
#include "AQObjUtilities.h"

namespace validation
{

    using etrading::CreateDataFile;
    using etrading::decorateFilename;

    /* @brief			validation interface for the aqCurveCompoundRate method
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
    DoubleVector tryAqCurveCompoundRate( const DateVector& startDates,
                                         const DateVector& endDates,
                                         const AQLString& curveCollection,
                                         const AQLString& forecastCurveIndex,
                                         const AQLString& frequency,
                                         double spread,
                                         const AQLString& stubType,
                                         const AQLString& rollDayInput,
                                         const AQLString& calendar,
                                         const AQLString& businessDayAdj,
                                         const AQLString& dayCount,
                                         const AQLString& interpolation,
                                         const AQLString& compoundType,
                                         const AQLString& firstStubDate,
                                         const AQLString& lastStubDate )

    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        AQ_RECORD_DECORATED_INPUTS( curveCollection, "", startDates, endDates, curveCollection, forecastCurveIndex, frequency, spread, stubType, rollDayInput, calendar, businessDayAdj, dayCount, interpolation, compoundType, firstStubDate, lastStubDate );

        AQ_THROW_IF( startDates.size() == 0 || endDates.size() == 0, "a size of vector is zero." );

        AQ_THROW_IF( startDates.size() != endDates.size(), "The input startDates and endDates must be of the same size." );

        AQLDate* firstOddDt  = NULL;
        AQLDate tmpfirstOddDt;
        if ( firstStubDate.size() != 0 )
        {
            tmpfirstOddDt = etrading::stringToDate( firstStubDate, "#Error: Invalid 'firstStubDate'." );
            firstOddDt = &tmpfirstOddDt;
        }
        AQLDate* lastOddDt  = NULL;
        AQLDate tmplastOddDt;
        if ( lastStubDate.size() != 0 )
        {
            tmplastOddDt = etrading::stringToDate( lastStubDate, "#Error: Invalid 'lastStubDate'." );
            lastOddDt = &tmplastOddDt;
        }

        AQLString cal = etrading::getDefaultCalendarForEmptyString( calendar, curveCollection );

        AQLString rollD( etrading::getDefaultValueForEmptyString( rollDayInput, "ENDDATE" ) );
        AQLString businessDayAdj( etrading::getDefaultValueForEmptyString( businessDayAdj, "MOD_FOLLOWING" ) );
        AQLString dayC( etrading::getDefaultValueForEmptyString( dayCount, "ACT/360" ) );
        AQLString interp( etrading::getDefaultValueForEmptyString( interpolation, "SPLINE" ) );
        AQLString cType( etrading::getDefaultValueForEmptyString( compoundType, "FLAT" ) );

        DoubleVector ret;
        AQLDataInstance* dataInstance = etrading::getDataInstance();

        //Throw exception if the curve has not been built.
        etrading::getCurveStaticDataTableName( curveCollection, forecastCurveIndex );

        for( size_t i = 0; i < startDates.size(); i++ )
        {

            AQLDate startDt = startDates[i];
            AQLDate endDt = endDates[i];

            bool isStartRoll = etrading::isStartRollAndPopulateStubDatesFromStubType( etrading::toStubTypeEnum(stubType.getCString()), firstOddDt, lastOddDt, startDt, endDt, frequency );

            double result = etrading::AQLCurveForwardRateHelpers::compound( dataInstance,
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


        AQ_RECORD_DECORATED_OUTPUTS( curveCollection, "", ret );

        return ret;

        VALID_EXCEPTION_END
    }


    /* @brief			validation interface for the aqRatesCompoundRate method
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
    double tryAqCurveCompoundRate( const AQLDate& startDate,
                                   const AQLDate& endDate,
                                   const AQLString& curveCollection,
                                   const AQLString& forecastCurveIndex,
                                   const AQLString& frequency,
                                   double spread,
                                   const AQLString& stubType,
                                   const AQLString& rollDayInput,
                                   const AQLString& calendar,
                                   const AQLString& businessDayAdj,
                                   const AQLString& dayCount,
                                   const AQLString& interpolation,
                                   const AQLString& compoundType,
                                   const AQLString& firstStubDate,
                                   const AQLString& lastStubDate )
    {
        VALID_EXCEPTION_START

        DateVector startDateVector;
        startDateVector.push_back( startDate );
        DateVector endDateVector;
        endDateVector.push_back( endDate );

        DoubleVector ret = tryAqCurveCompoundRate( startDateVector,
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


    /* @brief			validation interface for the aqCurveCompoundRateWithFixingTable method
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
    DoubleVector tryAqCurveCompoundRateWithFixingTable( const DateVector& startDates,
                                         const DateVector& endDates,
                                         const AQLString& curveCollection,
                                         const AQLString& forecastCurveIndex,
                                         const AQLString& frequency,
                                         double spread,
                                         const AQLString& stubType,
                                         const AQLString& rollDayInput,
                                         const AQLString& calendar,
                                         const AQLString& businessDayAdj,
                                         const AQLString& dayCount,
                                         const AQLString& interpolation,
                                         const AQLString& compoundType,
                                         const AQLString& firstStubDate,
                                         const AQLString& lastStubDate,
                                         const std::string& fixingTableName,
                                         bool annualized)

    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        AQ_RECORD_DECORATED_INPUTS( curveCollection, "", startDates, endDates, curveCollection, forecastCurveIndex, frequency, spread, stubType, rollDayInput, calendar, businessDayAdj, dayCount, interpolation, compoundType, firstStubDate, lastStubDate, fixingTableName, annualized );

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

        AQ_RECORD_DECORATED_OUTPUTS( curveCollection, "", ret );

        return ret;

        VALID_EXCEPTION_END
    }


    /* @brief			validation interface for the aqCurveCompoundRateWithFixingTable method
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
    double tryAqCurveCompoundRateWithFixingTable( const AQLDate& startDate,
                                   const AQLDate& endDate,
                                   const AQLString& curveCollection,
                                   const AQLString& forecastCurveIndex,
                                   const AQLString& frequency,
                                   double spread,
                                   const AQLString& stubType,
                                   const AQLString& rollDayInput,
                                   const AQLString& calendar,
                                   const AQLString& businessDayAdj,
                                   const AQLString& dayCount,
                                   const AQLString& interpolation,
                                   const AQLString& compoundType,
                                   const AQLString& firstStubDate,
                                   const AQLString& lastStubDate, 
                                   const std::string& fixingTableName,
                                   bool annualized)
    {
        VALID_EXCEPTION_START

        DateVector startDateVector;
        startDateVector.push_back( startDate );
        DateVector endDateVector;
        endDateVector.push_back( endDate );

        DoubleVector ret = tryAqCurveCompoundRateWithFixingTable( startDateVector,
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


