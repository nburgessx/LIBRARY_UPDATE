#include "tryAqCurvesForwardRate.h"
#include "tryAqDates.h"
#include "AQLCurveForwardRateHelpers.h"
#include "CreateDataFile.h"
#include "RecordMacros.h"
#include "StructuredExceptionHandler.h"
#include "ParameterValidation.h"
#include "CurveValidation.h"
#include "EnvironmentUtilities.h"
#include "ContainerUtilities.h"
#include "CurveValidation.h"
#include "CurveUtilities.h"
#include "AQObjUtilities.h"			// includes getCurveCollectionFromHandle()

using etrading::CreateDataFile;
using etrading::decorateCurvename;

namespace validation
{

    /* @brief			validation interface for aqCurvesForwardRatesFromYearFraction
    *  @param [in]		fromDates	    A vector of from dates
    *  @param [in]		yearFraction	YearFraction
    *  @param [in]		curveCollection	ID of the yield curve
    *  @param [in]		curveIndex		Index of the curve set. Default to STD
    *  @param [in]		dayCount		Day count convention. Default to ACT/365
    *  @param [in]		fwdInter		True or false, if not specified use the curve one
	*  @param [in]		businessDayAdjust	The business day adjustment, Default to MOD_FOLLOWING
	*  @return			The forward rates based on fromDates and yearFraction
    */
    DoubleVector tryAqCurvesForwardRatesFromYearFraction( const DateVector& fromDates,
														double yearFraction,
														const AQLString& dayCount,
														const AQLString& curveCollection,
														const AQLString& curveIndex,
														const std::string& fwdInter,
														const std::string& businessDayAdjust)
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryAqCurvesForwardRatesFromYearFraction_inputs", curveCollection, curveIndex ) );
            file.write( "generatorFunction",    "tryAqCurvesForwardRatesFromYearFraction" );
            file.write( "fromDates",			fromDates );
            file.write( "yearFraction",		    yearFraction );
            file.write( "dayCount",			    dayCount );
            file.write( "curveCollection",	    curveCollection );
            file.write( "curveIndex",		    curveIndex );
            file.write( "fwdInter",				fwdInter );
			file.write( "businessDayAdjust",	businessDayAdjust);
        }

		// Get the forward rates
        DoubleVector result = etrading::getCurveForwardRatesFromYearFraction( fromDates, yearFraction, dayCount, curveCollection, curveIndex, etrading::toBooleanEnum(fwdInter) );

        // Record the outputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryAqCurvesForwardRatesFromYearFraction_outputs", curveCollection, curveIndex ) );
            file.write( "output", result );
        }

        return result;

        VALID_EXCEPTION_END
    };


    /* @brief			validation interface for tryAqCurvesForwardRatesFromForwardDates
    *  @param [in]		fromDates	    A vector of from dates
    *  @param [in]		toDates			A vector of to dates
    *  @param [in]		curveCollection	ID of the yield curve
    *  @param [in]		curveIndex		Index of the curve set. Default to STD
    *  @param [in]		fwdInter		True or false, if not specified use the curve one
	*  @param [in]		businessDayAdjust	The business day adjustment, Default to MOD_FOLLOWING
	*  @return			The forward rates based on fromDates and toDates
    */
    DoubleVector tryAqCurvesForwardRatesFromForwardDates( const DateVector& fromDates,
                                                         const DateVector& toDates,
                                                         const AQLString& curveCollection,
                                                         const AQLString& curveIndex,
														 const std::string& fwdInter,
														 const std::string& businessDayAdjust)
    {

        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryAqCurvesForwardRatesFromForwardDates_inputs", curveCollection, curveIndex ) );
            file.write( "generatorFunction",    "tryAqCurvesForwardRatesFromForwardRates" );
            file.write( "fromDates",			fromDates );
            file.write( "toDates",			    toDates );
            file.write( "curveCollection",	    curveCollection );
            file.write( "curveIndex",		    curveIndex );
			file.write(	"fwdInter",				fwdInter);
			file.write(	"businessDayAdjust",	businessDayAdjust);
		}

		// Get the forward rates
        DoubleVector results = etrading::getCurveForwardRatesFromForwardDates( fromDates, toDates, curveCollection, curveIndex, etrading::toBooleanEnum(fwdInter) );

        // Record the outputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryAqCurvesForwardRatesFromForwardDates_outputs", curveCollection, curveIndex ) );
            file.write( "output", results );
        }

        return results;

        VALID_EXCEPTION_END
    };


	/* @brief			validation interface for tryAqCurvesForwardRatesFromForwardDates
    *  @param [in]		fromDates	    A vector of from dates
    *  @param [in]		toDates			A vector of to dates
    *  @param [in]		curveCollection	ID of the yield curve
    *  @param [in]		curveIndex		Index of the curve set. Default to STD
    *  @param [in]		fwdInter		True or false, if not specified use the curve one
	*  @param [in]		businessDayAdjust	The business day adjustment, Default to MOD_FOLLOWING
	*  @return			The forward rates based on fromDates and toDates
    */
    DoubleVector tryAqCurvesForwardRatesFromForwardDatesFromObject( const DateVector& fromDates,
																   const DateVector& toDates,
																   const AQLString& curveCollectionOrHandle,
																   const AQLString& curveIndex,
																   const std::string& fwdInter,
																   const std::string& businessDayAdjust)
    {

        VALID_EXCEPTION_START
		
		// To Allow Support for Curve Objects or Curve Collections
		AQLString curveCollectionFromHandle = etrading::getCurveCollectionFromHandle( curveCollectionOrHandle.c_str() );

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryAqCurvesForwardRatesFromForwardDates_inputs", curveCollectionFromHandle, curveIndex ) );
            file.write( "generatorFunction",    "tryAqCurvesForwardRatesFromForwardRates" );
            file.write( "fromDates",			fromDates );
            file.write( "toDates",			    toDates );
            file.write( "curveCollection",	    curveCollectionOrHandle );
            file.write( "curveIndex",		    curveIndex );
			file.write(	"fwdInter",				fwdInter);
			file.write(	"businessDayAdjust",	businessDayAdjust);
		}

		// Get the forward rates
        DoubleVector results = etrading::getCurveForwardRatesFromForwardDates( fromDates, toDates, curveCollectionFromHandle, curveIndex, etrading::toBooleanEnum(fwdInter) );

        // Record the outputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryAqCurvesForwardRatesFromForwardDates_outputs", curveCollectionFromHandle, curveIndex ) );
            file.write( "output", results );
        }

        return results;

        VALID_EXCEPTION_END
    };

    /* @brief			validation interface for aqCurvesForwardRates, which uses the curve frequency
    *  @param [in]		fixingDates	            A vector of fixing Dates
    *  @param [in]		curveCollection			ID of the yield curve
    *  @param [in]		curveIndex		        Index of the curve set. Default to STD
    *  @param [in]		fwdInter				True or false, if not specified use the curve one
	*  @param [in]		businessDayAdjust	The business day adjustment, Default to MOD_FOLLOWING
	*  @return			The forward rates based on the fixing dates
    */
    DoubleVector tryAqCurvesForwardRates( const DateVector& fixingDates,
                                         const AQLString& curveCollectionOrHandle,
                                         const AQLString& curveIndex,
										 const std::string& fwdInter,
										 const std::string& businessDayAdjust)
    {

        VALID_EXCEPTION_START
		
		// To Allow Support for Curve Objects or Curve Collections
		AQLString curveCollectionFromHandle = etrading::getCurveCollectionFromHandle( curveCollectionOrHandle.c_str() );

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryAqCurvesForwardRates_inputs", curveCollectionFromHandle, curveIndex ) );
            file.write( "generatorFunction", "tryAqCurvesForwardRates" );
            file.write( "fixingDates",		fixingDates );
            file.write( "curveCollection",	curveCollectionFromHandle );
            file.write( "curveIndex",		curveIndex );
            file.write( "fwdInter",			fwdInter );
			file.write(	"businessDayAdjust",businessDayAdjust);
		}

		// Calculate the forward rates
        DoubleArray result = etrading::getCurveForwardRates( fixingDates, curveCollectionFromHandle, curveIndex, etrading::toBusinessDayAdjustmentEnum(businessDayAdjust), "", etrading::toBooleanEnum(fwdInter));

        // Record outputs for testing
		if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryAqCurvesForwardRates_outputs", curveCollectionFromHandle, curveIndex ) );
            file.write( "output", result );
        }

        return result;

        VALID_EXCEPTION_END
    };

	/* @brief			Validation interface for aqObjCurvesForwardRatesTable, which generates a date schedule and
	*					uses the date schedule to calculate a matrix of forward rates for a set of curveIndices.
	*  @param [out]		fixingDates			The fixing dates on which the forwardRates are calculated. Generated from schedule parameters
	*  @param [out]		forwardRates		A matrix containing the calculated forward rates for each curve, for each fixing date in the generated schedule
	*  @param [in]		curveCollection		The curveCollection to use when accessing the curveIndices
	*  @param [in]		curveIndices		The curveIndices to use when calculating forward rates
	*  @param [in]		startDate			An optional string specifying the first date in the schedule. If empty, the curveCollection asOf date is used.
	*  @param [in]		maturity			A tenor describing the length of the date schedule, for example 10Y
	*  @param [in]		businessDayAdjust	The businessday adjustment, for example MODFOLLOWING
	*  @param [in]		calendar			The holiday calendar to use when rolling out the date schedule
	*  @param [in]		rollConvention		The roll convention to use when generating the date schedule, for example IMM
	*  @param [in]		frequency			The frquency of points in the date schedule, for example MONTHLY		
	*  @param [in]		fwdInterps			A list of fwdInter Flags for the curveIndices. Default to empty
	*/
	void tryAqObjCurvesForwardRatesTable( DateVector& fixingDates,
										 DoubleMatrix& forwardRates,
										 const AQLString& curveCollection,
										 const AQLStringVector& curveIndices,
										 const AQLString& startDate,
										 const AQLString& maturity,
										 const AQLString& businessDayAdjust,
										 const AQLString& calendar,             // We allow the calendar to be empty
										 const AQLString& rollConvention,
										 const AQLString& frequency,
										 const AQLStringVector& fwdInterps)
	{
		VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
		RECORD_INPUTS( curveCollection, curveIndices, startDate, maturity, businessDayAdjust, calendar, rollConvention, frequency, fwdInterps);

		if ( curveCollection.size() == 0 )
		{
			throw AQLCoreInvalidData("#Error: No curve collection have been provided.",__FILE__,__LINE__);
		}
		if (curveIndices.empty() )
		{
			throw AQLCoreInvalidData("#Error: No curve indices have been provided.",__FILE__,__LINE__);
		}
		if ( maturity.size() == 0 )
		{
			throw AQLCoreInvalidData("#Error: No maturity tenor has been provided.",__FILE__,__LINE__);
		}
		if ( businessDayAdjust.size() == 0 )
		{
			throw AQLCoreInvalidData("#Error: No business day adjustment has been provided.",__FILE__,__LINE__);
		}
		if ( rollConvention.size() == 0 )
		{
			throw AQLCoreInvalidData("#Error: No roll convention has been provided.",__FILE__,__LINE__);
		}
		if ( frequency.size() == 0 )
		{
			throw AQLCoreInvalidData("#Error: No frequency has been provided.",__FILE__,__LINE__);
		}

		// Calculate the forward rates
        etrading::getForwardRatesForCurveIndices( fixingDates, forwardRates, curveCollection, curveIndices, startDate, maturity, etrading::toBusinessDayAdjustmentEnum(businessDayAdjust.getCString()), calendar, rollConvention, frequency, fwdInterps);

        VALID_EXCEPTION_END
	}
}

