#include "tryAqCurveObjectDiscountFactor.h"

#include <sstream>

#include "CreateDataFile.h"
#include "CurveValidation.h"
#include "CurveStreaming.h"
#include "Environment.h"
#include "AQLCurveForwardRateHelpers.h"
#include "AQLDateSchedule.h"
#include "ParameterValidation.h"
#include "RecordMacros.h"
#include "StructuredExceptionHandler.h"
#include "tryAqCurveDiscountFactor.h" // needed for the utility functions
#include "tryAqDate.h"

#include <string>
#include <limits>
#include <boost/date_time.hpp>

using etrading::CreateDataFile;
using etrading::decorateCurvename;

namespace validation
{
    /* @brief			validation method for aqCurveDiscountFactorsFromYearFractions
    *  @param [in]		aqObjCurveName	Curve Name
    *  @param [in]		yearFractions	An array of yearFractions by year count
    *  @param [in]		dayCount	    The daycount fraction used to generate the yearFraction
    *  @return			An array of discount factor
    */
    DoubleVector tryAqCurveObjectDiscountFactorsFromYearFractions( const std::string& aqObjCurveName,
            const DoubleVector& yearFractions,
            const AQLString& dayCount )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( std::string( std::string( "tryAqCurveObjectDiscountFactorsFromYearFractions_inputs_" ) + aqObjCurveName ).c_str()  );
            file.write( "generatorFunction", "tryAqCurveObjectDiscountFactorsFromYearFractions" );
            file.write( "aqObjCurveName",	    aqObjCurveName );
            file.write( "yearFractions",		yearFractions );
            file.write( "dayCount",          dayCount );
        }

        // Input validations
        AQ_THROW_IF( yearFractions.size() == 0, "Size of input 'yearFractions' is zero." );

        DoubleVector ret( 0, std::numeric_limits<double>::quiet_NaN() );

        auto& env = etrading::Environment::defaultEnv();
        auto aqObjCurve = env.accessObject<etrading::AQObjCurve>( aqObjCurveName ); // 			etrading::getAQObjCurve(aqObjCurveName);

        // Convert Year Fractions to ACT/365 ones
        AQLDate curveAsOfDate = etrading::toAQLDateFromGregorianDate( aqObjCurve->getCurveBuildStaticDataObject()->asOfDate_ );
        etrading::convertToYearFractionsACT365( curveAsOfDate, const_cast< DoubleVector& >( yearFractions ), dayCount );

        if( aqObjCurve )
        {
            ret = aqObjCurve->calculateDiscountFactor( yearFractions );
        }
        else
        {
            std::ostringstream curveMsg;
            curveMsg << "Curve " << aqObjCurveName << " does not exist.";
            AQ_THROW( curveMsg.str() );
        }

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( std::string( std::string( "tryAqCurveObjectDiscountFactorsFromYearFractions_outputs_" ) + aqObjCurveName ).c_str() );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }


    /* @brief			validation method for aqCurveDiscountFactorsFromTenors
    *  @param [in]		aqObjCurveName	Curve Name
    *  @param [in]		tenors			An array of tenor strings
    *  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
    *  @param [in]		calendar		Calendar
    *  @return			An array of discount factor
    */
    DoubleVector tryAqCurveObjectDiscountFactorsFromTenors( const std::string& aqObjCurveName,
            const std::vector<std::string>& tenors,
            const std::string& businessDayAdj,
            const std::string& calendar )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( std::string( std::string( "tryAqCurveObjectDiscountFactorsFromTenors_inputs_" ) + aqObjCurveName ).c_str()  );
            file.write( "generatorFunction", "tryAqCurveObjectDiscountFactorsFromTenors" );
            file.write( "aqObjCurveName",	    aqObjCurveName );
            file.write( "tenors",			tenors );
            file.write( "businessDayAdj",	businessDayAdj );
            file.write( "calendar",			calendar );

        }

        // Input validations
        AQ_THROW_IF( tenors.size() == 0, "Size of input 'tenors' is zero." );

        etrading::BusinessDayAdjustmentEnum busDayAdjust = etrading::toBusinessDayAdjustmentEnum( businessDayAdj );

        DoubleVector ret( 0, std::numeric_limits<double>::quiet_NaN() );
        auto& env = etrading::Environment::defaultEnv();
        auto aqObjCurve = env.accessObject<etrading::AQObjCurve>( aqObjCurveName ); // 			etrading::getAQObjCurve(aqObjCurveName);
        if( aqObjCurve )
        {
            ret = aqObjCurve->calculateDiscountFactor( tenors, busDayAdjust, calendar );
        }
        else
        {
            std::ostringstream curveMsg;
            curveMsg << "Curve " << aqObjCurveName << " does not exist.";
            AQ_THROW( curveMsg.str() );
        }

        AQ_THROW_IF( ret.size() == 0, "No DFs have been returned." );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( std::string( std::string( "tryAqCurveObjectDiscountFactorsFromTenors_outputs_" ) + aqObjCurveName ).c_str()  );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }


    DoubleVector tryAqCurveObjectDiscountFactorsForwardStarting( const std::string& aqObjCurveName,
            const std::vector<boost::gregorian::date>& fromDates,
            const std::vector<boost::gregorian::date>& toDates )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( ( std::string( "tryAqCurveObjectDiscountFactorsForwardStarting_inputs_" ) + aqObjCurveName ).c_str() );
            file.write( "generatorFunction", "tryAqCurveObjectDiscountFactorsForwardStarting" );
            file.write( "aqObjCurveName",	    aqObjCurveName );
            file.write( "fromDates",			fromDates );
            file.write( "toDates",			toDates );
        }

        size_t M = fromDates.size();
        size_t N = toDates.size();
        AQ_THROW_IF( M != N && M != 1, "'fromDates' either takes 1 date, or an array of dates in equal size of 'toDates'" );
        AQ_THROW_IF( N == 0 || M == 0, "'fromDates' and 'toDates' must not be empty" );

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
        auto aqObjCurve = env.accessObject<etrading::AQObjCurve>( aqObjCurveName ); // 			etrading::getAQObjCurve(aqObjCurveName);

        if( aqObjCurve )
        {
            ret = aqObjCurve->calculateDiscountFactor( fromDateVec, toDates );
        }
        else
        {
            std::ostringstream curveMsg;
            curveMsg << "Curve " << aqObjCurveName << " does not exist.";
            AQ_THROW( curveMsg.str() );
        }

        AQ_THROW_IF( ret.size() == 0, "No DFs have been returned." );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( ( std::string( "tryAqCurveObjectDiscountFactorsForwardStarting_outputs_" ) + aqObjCurveName ).c_str() );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }


    /* @brief			validation method for aqCurveDiscountFactorsForwardStartingFromYearFractions
    *  @param [in]		aqObjCurveName		Curve Name
    *  @param [in]		fromDates			An array of from-dates in YYYYMMDD formate
    *  @param [in]		yearFractions		An array of year fraction yearFractions
    *  @param [in]		dayCount	        The daycount fraction used to generate the yearFraction
    *  @return			A array of discount factors
    */
    DoubleVector tryAqCurveObjectDiscountFactorsForwardStartingFromYearFractions( const std::string& aqObjCurveName,
            const std::vector<boost::gregorian::date>& fromDates,
            const DoubleVector& yearFractions,
            const AQLString& dayCount )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( ( std::string( "tryAqCurveObjectDiscountFactorsForwardStartingFromYearFractions_inputs_" ) + aqObjCurveName ).c_str() );
            file.write( "generatorFunction", "tryAqCurveObjectDiscountFactorsForwardStartingFromYearFractions" );
            file.write( "aqObjCurveName",		aqObjCurveName );
            file.write( "fromDates",			fromDates );
            file.write( "yearFractions",		yearFractions );
            file.write( "dayCount",          dayCount );
        }

        size_t M = fromDates.size();
        size_t N = yearFractions.size();
        AQ_THROW_IF( M != N, "'fromDates' and 'yearFractions' must be of equal size'" );

        AQ_THROW_IF( N == 0 || M == 0, "'fromDates' and 'yearFractions' must not be empty" );

        etrading::BusinessDayAdjustmentEnum busDayAdjust = etrading::toBusinessDayAdjustmentEnum( "NO_CHANGE" );
        DoubleVector ret( 0, std::numeric_limits<double>::quiet_NaN() );

        auto& env = etrading::Environment::defaultEnv();
        auto aqObjCurve = env.accessObject<etrading::AQObjCurve>( aqObjCurveName ); // 			etrading::getAQObjCurve(aqObjCurveName);


        // Convert Year Fractions to ACT/365 ones
        AQLDate curveAsOfDate = etrading::toAQLDateFromGregorianDate( aqObjCurve->getCurveBuildStaticDataObject()->asOfDate_ );
        etrading::convertToYearFractionsACT365( curveAsOfDate, const_cast< DoubleVector& >( yearFractions ), dayCount );

        if( aqObjCurve )
        {
            std::string calendar( "" );
            ret = aqObjCurve->calculateDiscountFactor( fromDates, yearFractions, busDayAdjust, calendar );
        }
        else
        {
            std::ostringstream curveMsg;
            curveMsg << "Curve " << aqObjCurveName << " does not exist.";
            AQ_THROW( curveMsg.str() );
        }

        AQ_THROW_IF( ret.size() == 0, "No DFs have been returned." );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( ( std::string( "tryAqCurveDiscountFactorsForwardStartingFromYearFractions_outputs_" ) + aqObjCurveName ).c_str() );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }


    /* @brief			validation method for aqCurveDiscountFactorsForwardStartingFromTenor
    *  @param [in]		aqObjCurveName		Curve Name
    *  @param [in]		fromDates		An array of from-dates in YYYYMMDD formate
    *  @param [in]		tenor			A tenor string
    *  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
    *  @param [in]		calendar		Calendar
    *  @return			A array of discount factors
    */
    DoubleVector tryAqCurveObjectDiscountFactorsForwardStartingFromTenors( const std::string& aqObjCurveName,
            const std::vector<boost::gregorian::date>& fromDates,
            const std::vector<std::string>& tenors,
            const std::string& businessDayAdj,
            const std::string& calendar )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( ( std::string( "tryAqCurveObjectDiscountFactorsForwardStartingFromTenors_inputs_" ) + aqObjCurveName ).c_str() );
            file.write( "generatorFunction", "tryAqCurveObjectDiscountFactorsForwardStartingFromTenors" );
            file.write( "aqObjCurveName",		aqObjCurveName );
            file.write( "fromDates",			fromDates );
            file.write( "tenor",				tenors );
            file.write( "businessDayAdj",	businessDayAdj );
            file.write( "calendar",			calendar );
        }

        AQ_THROW_IF( fromDates.size() == 0, "'fromDates' must not be empty" );

        if( fromDates.size() != tenors.size() )
        {
            std::ostringstream errStream;
            errStream << "Number of fromDates (" << fromDates.size() << ") is not equal to the number of tenors (" << tenors.size() << ")";
            AQ_THROW( errStream.str() );
        }

        etrading::BusinessDayAdjustmentEnum busDayAdjust = etrading::toBusinessDayAdjustmentEnum( businessDayAdj );
        DoubleVector ret( 0, std::numeric_limits<double>::quiet_NaN() );

        auto& env = etrading::Environment::defaultEnv();
        auto aqObjCurve = env.accessObject<etrading::AQObjCurve>( aqObjCurveName ); // 			etrading::getAQObjCurve(aqObjCurveName);
        if( aqObjCurve )
        {
            ret = aqObjCurve->calculateDiscountFactor( fromDates, tenors, busDayAdjust, calendar );
        }
        else
        {
            std::ostringstream curveMsg;
            curveMsg << "Curve " << aqObjCurveName << " does not exist.";
            AQ_THROW( curveMsg.str() );
        }

        AQ_THROW_IF( ret.size() == 0, "No DFs have been returned." );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( ( std::string( "tryAqCurveObjectDiscountFactorsForwardStartingFromTenors_outputs_" ) + aqObjCurveName ).c_str() );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }


    /* @brief			validation method for aqCurveObjectDiscountFactors
    *  @param [in]		paymentDates	A single or an array of to-date in YYYYMMDD formate
    *  @param [in]		aqObjCurveName	AQObj Curve Name
    *  @return			A array of discount factors
    */
    DoubleVector tryAqCurveObjectDiscountFactors( const std::string& aqObjCurveName,
            const std::vector<boost::gregorian::date>&  paymentDates )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( ( std::string( "tryAqCurveObjectDiscountFactors_inputs_" ) + aqObjCurveName ).c_str() );
            file.write( "generatorFunction",     "tryAqCurveObjectDiscountFactors" );
            file.write( "aqObjCurveName",		    aqObjCurveName );
            file.write( "paymentDates",	        paymentDates );
        }

        DoubleVector ret( 0, std::numeric_limits< double >::quiet_NaN() );

        auto& env       = etrading::Environment::defaultEnv();
        auto aqObjCurve   = env.accessObject< etrading::AQObjCurve >( aqObjCurveName );

        if( aqObjCurve )
        {
            ret = aqObjCurve->calculateDiscountFactor( paymentDates );
        }
        else
        {
            std::ostringstream curveMsg;
            curveMsg << "Curve " << aqObjCurveName << " does not exist.";
            AQ_THROW( curveMsg.str() );
        }

        AQ_THROW_IF( ret.size() == 0, "No DFs have been returned." );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( ( std::string( "tryAqCurveObjectDiscountFactors_outputs_" ) + aqObjCurveName ).c_str() );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    }

    /* @brief			Validation interface for aqCurveObjectDiscountFactorsTable, which generates a date schedule and
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
	void tryAqCurveObjectDiscountFactorsTable( DateVector& paymentDates,
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
		AQ_RECORD_INPUTS( curveCollection, curveIndices, startDate, maturity, businessDayAdjust, calendar, rollConvention, frequency );

		AQ_THROW_IF( curveCollection.size() == 0, "No curve collection have been provided." );
		AQ_THROW_IF( curveIndices.empty(), "No curve indices have been provided." );
		AQ_THROW_IF( maturity.size() == 0, "No maturity tenor has been provided." );
		AQ_THROW_IF( businessDayAdjust.size() == 0, "No business day adjustment has been provided." );
		AQ_THROW_IF( rollConvention.size() == 0, "No roll convention has been provided." );
		AQ_THROW_IF( frequency.size() == 0, "No frequency has been provided." );

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
	DoubleVector tryAqCurveObjectDiscountFactorsWithSpread( const DateVector& paymentDates,
														 const std::string& curveCollection,
														 const std::string& curveIndex,
														 const double spread,
														 const std::string& fixingTableName )
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		AQ_RECORD_INPUTS( paymentDates, curveCollection, curveIndex, spread, fixingTableName );

		AQ_REQUIRE( paymentDates.size() > 0, "No payment dates have been provided." );
		AQ_REQUIRE( curveCollection.size() > 0, "No curve collection have been provided." );
		AQ_REQUIRE( curveIndex.size() > 0, "No curveIndex has been provided." );

		const DoubleVector discountFactors = etrading::getDiscountFactorsForCurveIndexWithSpread( paymentDates, curveCollection, curveIndex, spread, fixingTableName );
		
		AQ_REQUIRE( discountFactors.size() > 0, "No discount factors have been returned." );
		AQ_REQUIRE( discountFactors.size() == paymentDates.size(), "Incorrect number of discount factors returned.");
        
		// Record Outputs and Return the result
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( discountFactors );
			
		VALID_EXCEPTION_END
	}


}
