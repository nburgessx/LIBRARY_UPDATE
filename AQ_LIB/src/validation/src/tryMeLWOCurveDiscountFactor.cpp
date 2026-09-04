#include "tryMeLWOCurveDiscountFactor.h"

#include "CreateDataFile.h"
#include "CurveValidation.h"
#include "CurveStreaming.h"
#include "Environment.h"
#include "AQLCurveForwardRateHelpers.h"
#include "AQLDateScheduleHelpers.h"
#include "ParameterValidation.h"
#include "RecordMacros.h"
#include "StructuredExceptionHandler.h"
#include "tryAqCurvesDiscountFactor.h" // needed for the utility functions
#include "tryAqDates.h"

#include <string>
#include <limits>
#include <boost/date_time.hpp>

using etrading::CreateDataFile;
using etrading::decorateCurvename;

namespace validation
{
    /* @brief			validation method for aqCurvesDiscountFactorsFromYearFractions
    *  @param [in]		lwoCurveName	Curve Name
    *  @param [in]		yearFractions	An array of yearFractions by year count
    *  @param [in]		dayCount	    The daycount fraction used to generate the yearFraction
    *  @return			An array of discount factor
    */
    DoubleVector tryMeLWOCurveDiscountFactorsFromYearFractions( const std::string& lwoCurveName,
            const DoubleVector& yearFractions,
            const AQLString& dayCount )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( std::string( std::string( "tryMeLWOCurveDiscountFactorsFromYearFractions_inputs_" ) + lwoCurveName ).c_str()  );
            file.write( "generatorFunction", "tryMeLWOCurveDiscountFactorsFromYearFractions" );
            file.write( "lwoCurveName",	    lwoCurveName );
            file.write( "yearFractions",		yearFractions );
            file.write( "dayCount",          dayCount );
        }

        // Input validations
        if( yearFractions.size() == 0 )
        {
            throw AQLCoreInvalidData( "#Error: Size of input 'yearFractions' is zero.", __FILE__, __LINE__ );
        }

        DoubleVector ret( 0, std::numeric_limits<double>::quiet_NaN() );

        auto& env = etrading::Environment::defaultEnv();
        auto lwoCurve = env.accessObject<etrading::AQOCurve>( lwoCurveName ); // 			etrading::getLWOCurve(lwoCurveName);

        // Convert Year Fractions to ACT/365 ones
        AQLDate curveAsOfDate = etrading::toAQLDateFromGregorianDate( lwoCurve->getCurveBuildStaticDataObject()->asOfDate_ );
        etrading::convertToYearFractionsACT365( curveAsOfDate, const_cast< DoubleVector& >( yearFractions ), dayCount );

        if( lwoCurve )
        {
            ret = lwoCurve->calculateDiscountFactor( yearFractions );
        }
        else
        {
            AQ_THROW( ( boost::format( "Curve %s does not exist." ) % lwoCurveName.c_str() ).str().c_str() );
        }

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( std::string( std::string( "tryMeLWOCurveDiscountFactorsFromYearFractions_outputs_" ) + lwoCurveName ).c_str() );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }


    /* @brief			validation method for aqCurvesDiscountFactorsFromTenors
    *  @param [in]		lwoCurveName	Curve Name
    *  @param [in]		tenors			An array of tenor strings
    *  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
    *  @param [in]		calendar		Calendar
    *  @return			An array of discount factor
    */
    DoubleVector tryMeLWOCurveDiscountFactorsFromTenors( const std::string& lwoCurveName,
            const std::vector<std::string>& tenors,
            const std::string& businessDayAdj,
            const std::string& calendar )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( std::string( std::string( "tryMeLWOCurveDiscountFactorsFromTenors_inputs_" ) + lwoCurveName ).c_str()  );
            file.write( "generatorFunction", "tryMeLWOCurveDiscountFactorsFromTenors" );
            file.write( "lwoCurveName",	    lwoCurveName );
            file.write( "tenors",			tenors );
            file.write( "businessDayAdj",	businessDayAdj );
            file.write( "calendar",			calendar );

        }

        // Input validations
        if( tenors.size() == 0 )
        {
            throw AQLCoreInvalidData( "#Error: Size of input 'tenors' is zero.", __FILE__, __LINE__ );
        }

        etrading::BusinessDayAdjustmentEnum busDayAdjust = etrading::toBusinessDayAdjustmentEnum( businessDayAdj );

        DoubleVector ret( 0, std::numeric_limits<double>::quiet_NaN() );
        auto& env = etrading::Environment::defaultEnv();
        auto lwoCurve = env.accessObject<etrading::AQOCurve>( lwoCurveName ); // 			etrading::getLWOCurve(lwoCurveName);
        if( lwoCurve )
        {
            ret = lwoCurve->calculateDiscountFactor( tenors, busDayAdjust, calendar );
        }
        else
        {
            AQ_THROW( ( boost::format( "Curve %s does not exist." ) % lwoCurveName.c_str() ).str().c_str() );
        }

        if ( ret.size() == 0 )
        {
            throw AQLCoreInvalidData( "#Error: No DFs have been returned.", __FILE__, __LINE__ );
        }

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( std::string( std::string( "tryMeLWOCurveDiscountFactorsFromTenors_outputs_" ) + lwoCurveName ).c_str()  );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }


    DoubleVector tryMeLWOCurveDiscountFactorsForwardStarting( const std::string& lwoCurveName,
            const std::vector<boost::gregorian::date>& fromDates,
            const std::vector<boost::gregorian::date>& toDates )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( ( std::string( "tryMeLWOCurveDiscountFactorsForwardStarting_inputs_" ) + lwoCurveName ).c_str() );
            file.write( "generatorFunction", "tryMeLWOCurveDiscountFactorsForwardStarting" );
            file.write( "lwoCurveName",	    lwoCurveName );
            file.write( "fromDates",			fromDates );
            file.write( "toDates",			toDates );
        }

        size_t M = fromDates.size();
        size_t N = toDates.size();
        if( M != N && M != 1 )
        {
            throw AQLCoreInvalidData( "#Error: 'fromDates' either takes 1 date, or an array of dates in equal size of 'toDates'", __FILE__, __LINE__ );
        }
        if( N == 0 || M == 0 )
        {
            throw AQLCoreInvalidData( "#Error: 'fromDates' and 'toDates' must not be empty", __FILE__, __LINE__ );
        }

        // Create pairs of fromDate and toDate even when there is only one fromDate
        std::vector<boost::gregorian::date> fromDateVec;
        fromDateVec.push_back( fromDates[0] );
        if( M == 1 )
        {
            for( size_t i = 1; i < N; ++i )
            {
                fromDateVec.push_back( fromDates[0] );
            }
        }
        else
        {
            for( size_t i = 1; i < M; ++i )
            {
                fromDateVec.push_back( fromDates[i] );
            }
        }

        DoubleVector ret( 0, std::numeric_limits<double>::quiet_NaN() );

        auto& env = etrading::Environment::defaultEnv();
        auto lwoCurve = env.accessObject<etrading::AQOCurve>( lwoCurveName ); // 			etrading::getLWOCurve(lwoCurveName);

        if( lwoCurve )
        {
            ret = lwoCurve->calculateDiscountFactor( fromDateVec, toDates );
        }
        else
        {
            AQ_THROW( ( boost::format( "Curve %s does not exist." ) % lwoCurveName.c_str() ).str().c_str() );
        }

        if ( ret.size() == 0 )
        {
            throw AQLCoreInvalidData( "#Error: No DFs have been returned.", __FILE__, __LINE__ );
        }

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( ( std::string( "tryMeLWOCurveDiscountFactorsForwardStarting_outputs_" ) + lwoCurveName ).c_str() );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }


    /* @brief			validation method for aqCurvesDiscountFactorsForwardStartingFromYearFractions
    *  @param [in]		lwoCurveName		Curve Name
    *  @param [in]		fromDates			An array of from-dates in YYYYMMDD formate
    *  @param [in]		yearFractions		An array of year fraction yearFractions
    *  @param [in]		dayCount	        The daycount fraction used to generate the yearFraction
    *  @return			A array of discount factors
    */
    DoubleVector tryMeLWOCurveDiscountFactorsForwardStartingFromYearFractions( const std::string& lwoCurveName,
            const std::vector<boost::gregorian::date>& fromDates,
            const DoubleVector& yearFractions,
            const AQLString& dayCount )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( ( std::string( "tryMeLWOCurveDiscountFactorsForwardStartingFromYearFractions_inputs_" ) + lwoCurveName ).c_str() );
            file.write( "generatorFunction", "tryMeLWOCurveDiscountFactorsForwardStartingFromYearFractions" );
            file.write( "lwoCurveName",		lwoCurveName );
            file.write( "fromDates",			fromDates );
            file.write( "yearFractions",		yearFractions );
            file.write( "dayCount",          dayCount );
        }

        size_t M = fromDates.size();
        size_t N = yearFractions.size();
        if( M != N )
        {
            throw AQLCoreInvalidData( "#Error: 'fromDates' and 'yearFractions' must be of equal size'", __FILE__, __LINE__ );
        }

        if( N == 0 || M == 0 )
        {
            throw AQLCoreInvalidData( "#Error: 'fromDates' and 'yearFractions' must not be empty", __FILE__, __LINE__ );
        }

        etrading::BusinessDayAdjustmentEnum busDayAdjust = etrading::toBusinessDayAdjustmentEnum( "NO_CHANGE" );
        DoubleVector ret( 0, std::numeric_limits<double>::quiet_NaN() );

        auto& env = etrading::Environment::defaultEnv();
        auto lwoCurve = env.accessObject<etrading::AQOCurve>( lwoCurveName ); // 			etrading::getLWOCurve(lwoCurveName);


        // Convert Year Fractions to ACT/365 ones
        AQLDate curveAsOfDate = etrading::toAQLDateFromGregorianDate( lwoCurve->getCurveBuildStaticDataObject()->asOfDate_ );
        etrading::convertToYearFractionsACT365( curveAsOfDate, const_cast< DoubleVector& >( yearFractions ), dayCount );

        if( lwoCurve )
        {
            std::string calendar( "" );
            ret = lwoCurve->calculateDiscountFactor( fromDates, yearFractions, busDayAdjust, calendar );
        }
        else
        {
            AQ_THROW( ( boost::format( "Curve %s does not exist." ) % lwoCurveName.c_str() ).str().c_str() );
        }

        if ( ret.size() == 0 )
        {
            throw AQLCoreInvalidData( "#Error: No DFs have been returned.", __FILE__, __LINE__ );
        }

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( ( std::string( "tryMeCurveDiscountFactorsForwardStartingFromYearFractions_outputs_" ) + lwoCurveName ).c_str() );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }


    /* @brief			validation method for aqCurvesDiscountFactorsForwardStartingFromTenor
    *  @param [in]		lwoCurveName		Curve Name
    *  @param [in]		fromDates		An array of from-dates in YYYYMMDD formate
    *  @param [in]		tenor			A tenor string
    *  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
    *  @param [in]		calendar		Calendar
    *  @return			A array of discount factors
    */
    DoubleVector tryMeLWOCurveDiscountFactorsForwardStartingFromTenors( const std::string& lwoCurveName,
            const std::vector<boost::gregorian::date>& fromDates,
            const std::vector<std::string>& tenors,
            const std::string& businessDayAdj,
            const std::string& calendar )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( ( std::string( "tryMeLWOCurveDiscountFactorsForwardStartingFromTenor_inputs_" ) + lwoCurveName ).c_str() );
            file.write( "generatorFunction", "tryMeLWOCurveDiscountFactorsForwardStartingFromTenor" );
            file.write( "lwoCurveName",		lwoCurveName );
            file.write( "fromDates",			fromDates );
            file.write( "tenor",				tenors );
            file.write( "businessDayAdj",	businessDayAdj );
            file.write( "calendar",			calendar );
        }

        if( fromDates.size() == 0 )
        {
            throw AQLCoreInvalidData( "#Error: 'fromDates' must not be empty", __FILE__, __LINE__ );
        }

        if( fromDates.size() != tenors.size() )
        {
            std::string errString = ( boost::format( "Number of fromDates (%i) is not equal to the number of tenors (%i)" ) % fromDates.size() % tenors.size() ).str();
            throw AQLCoreInvalidData( errString.c_str(), __FILE__, __LINE__ );
        }

        etrading::BusinessDayAdjustmentEnum busDayAdjust = etrading::toBusinessDayAdjustmentEnum( businessDayAdj );
        DoubleVector ret( 0, std::numeric_limits<double>::quiet_NaN() );

        auto& env = etrading::Environment::defaultEnv();
        auto lwoCurve = env.accessObject<etrading::AQOCurve>( lwoCurveName ); // 			etrading::getLWOCurve(lwoCurveName);
        if( lwoCurve )
        {
            ret = lwoCurve->calculateDiscountFactor( fromDates, tenors, busDayAdjust, calendar );
        }
        else
        {
            AQ_THROW( ( boost::format( "Curve %s does not exist." ) % lwoCurveName.c_str() ).str().c_str() );
        }

        if ( ret.size() == 0 )
        {
            throw AQLCoreInvalidData( "#Error: No DFs have been returned.", __FILE__, __LINE__ );
        }

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( ( std::string( "tryMeLWOCurveDiscountFactorsForwardStartingFromTenor_outputs_" ) + lwoCurveName ).c_str() );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }


    /* @brief			validation method for meLWOCurveDiscountFactors
    *  @param [in]		paymentDates	A single or an array of to-date in YYYYMMDD formate
    *  @param [in]		lwoCurveName	LWO Curve Name
    *  @return			A array of discount factors
    */
    DoubleVector tryMeLWOCurveDiscountFactors( const std::string& lwoCurveName,
            const std::vector<boost::gregorian::date>&  paymentDates )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( ( std::string( "tryMeLWOCurveDiscountFactors_inputs_" ) + lwoCurveName ).c_str() );
            file.write( "generatorFunction",     "tryMeLWOCurveDiscountFactors" );
            file.write( "lwoCurveName",		    lwoCurveName );
            file.write( "paymentDates",	        paymentDates );
        }

        DoubleVector ret( 0, std::numeric_limits< double >::quiet_NaN() );

        auto& env       = etrading::Environment::defaultEnv();
        auto lwoCurve   = env.accessObject< etrading::AQOCurve >( lwoCurveName );

        if( lwoCurve )
        {
            ret = lwoCurve->calculateDiscountFactor( paymentDates );
        }
        else
        {
            AQ_THROW( ( boost::format( "Curve %s does not exist." ) % lwoCurveName.c_str() ).str().c_str() );
        }

        if ( ret.size() == 0 )
        {
            throw AQLCoreInvalidData( "#Error: No DFs have been returned.", __FILE__, __LINE__ );
        }

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( ( std::string( "tryMeLWOCurveDiscountFactors_outputs_" ) + lwoCurveName ).c_str() );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }

    /* @brief			Validation interface for meLWOCurveDiscountFactorsTable, which generates a date schedule and
	*					uses the date schedule to calculate a matrix of discount factors for a set of curveIndices.
	*  @param [out]		paymentDates		The payment dates on which the discountFactors are calculated. Generated from schedule parameters
	*  @param [out]		discountFactors		A matrix containing the calculated discount factors for each curve, for each fixing date in the generated schedule
	*  @param [in]		curveCollection		The curveCollection to use when accessing the curveIndices
	*  @param [in]		curveIndices		The curveIndices to use when calculating discount factors
	*  @param [in]		startDate			An optional string specifying the first date in the schedule. If empty, the curveCollection asOf date is used.
	*  @param [in]		maturity			A tenor describing the length of the date schedule, for example 10Y
	*  @param [in]		businessDayAdjust	The businessday adjustment, for example MODFOLLOWING
	*  @param [in]		calendar			The holiday calendar to use when rolling out the date schedule
	*  @param [in]		rollConvention		The roll convention to use when generating the date schedule, for example IMM
	*  @param [in]		frequency			The frquency of points in the date schedule, for example MONTHLY		
	*/
	void tryMeLWOCurveDiscountFactorsTable( DateVector& paymentDates,
											DoubleMatrix& discountFactors,
											const AQLString& curveCollection,
											const AQLStringVector& curveIndices,
											const AQLString& startDate,
											const AQLString& maturity,
											const AQLString& businessDayAdjust,
											const AQLString& calendar,
											const AQLString& rollConvention,
											const AQLString& frequency )
	{
		VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
		RECORD_INPUTS( curveCollection, curveIndices, startDate, maturity, businessDayAdjust, calendar, rollConvention, frequency );

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

		// Calculate the discount factors
        etrading::getDiscountFactorsForCurveIndices( paymentDates, discountFactors, curveCollection, curveIndices, startDate, maturity, etrading::toBusinessDayAdjustmentEnum(businessDayAdjust.getCString()), calendar, rollConvention, frequency );

        VALID_EXCEPTION_END
	}
    
	/* @brief			Validation method which calculates a set of discount factors for the specified dates.
	*					If a spread is provided, calculates the discount factors using the zero discount-margin
	*					approach described by O'Kane in "Credit Spreads Explained".
	*
    *  @param [in]		paymentDates	A vector of one or more dates
    *  @param [in]		curveCollection	CurveCollection name
    *  @param [in]		curveIndex		Index of the curve.
	*  @param [in]		spread			Spread to be added to the curve zero rate when calculating the discount factor
	*  @param [in]		fixingTableName An optional fixing table name. This field may be blank if no fixings are required.
    *  @return			A vector of discount factors
    */
	DoubleVector tryMeLWOCurveDiscountFactorsWithSpread( const DateVector& paymentDates,
														 const std::string& curveCollection,
														 const std::string& curveIndex,
														 const double spread,
														 const std::string& fixingTableName )
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS( paymentDates, curveCollection, curveIndex, spread, fixingTableName );

		AQ_REQUIRE( paymentDates.size() > 0, "No payment dates have been provided." );
		AQ_REQUIRE( curveCollection.size() > 0, "No curve collection have been provided." );
		AQ_REQUIRE( curveIndex.size() > 0, "No curveIndex has been provided." );

		const DoubleVector discountFactors = etrading::getDiscountFactorsForCurveIndexWithSpread( paymentDates, curveCollection, curveIndex, spread, fixingTableName );
		
		AQ_REQUIRE( discountFactors.size() > 0, "No discount factors have been returned." );
		AQ_REQUIRE( discountFactors.size() == paymentDates.size(), "Incorrect number of discount factors returned.");
        
		// Record Outputs and Return the result
        RECORD_OUTPUTS_AND_RETURN_RESULT( discountFactors );
			
		VALID_EXCEPTION_END
	}


}
