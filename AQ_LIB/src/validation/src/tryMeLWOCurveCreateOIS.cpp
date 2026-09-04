#include <algorithm>
#include <memory>
#include <boost/format.hpp>
#include <boost/range/irange.hpp>
#include <boost/assign.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "tryMeLWOCurveCreateOIS.h"
#include "tryMeCurveDiscountFactor.h"

#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "ParameterValidation.h"
#include "CurveValidation.h"
#include "KeyValueLookupTable.h"
#include "DateUtilities.h"
#include "AQOCurve.h"
#include "ContainerUtilities.h"
#include "CurveBuildProperties.h"
#include "ObjectUtilities.h"
#include "Environment.h"
#include "EnvironmentUtilities.h"
#include "InterpolationParameters.h"
#include "AQLCurvePricingObject.h"
#include "AQLCurveForwardRateHelpers.h"
#include "AQLUpdateStaticDataManager.h"
#include "AQLCurveCalibrationHelpers.h"
#include "AQLPriceDataInterpolation.h"
#include "AQLInterpolationBase.h"
#include "AQLSplineInterpolation.h"
#include "AQLConstrainedSplineInterpolation.h"
#include "AQLMonotoneConvexInterpolation.h"
#include "AQLLinearInterpolation.h"
#include "AQLStepInterpolation.h"
#include "EnvironmentPool.h"
#include "tryMeLWOGrid.h"
#include "AQLCoreComponentManager.h"
#include "AQLCalibrateModelIR.h"
#include "EntityPoolUtilities.h"
#include "CurveResultsContainer.h"

using etrading::CreateDataFile;
using etrading::decorateCurvename;

namespace validation
{

    /* @brief			validation interface for the meLWOCurveCreateOIS method
    *  @param [in]		curveCollection		The curve collection ID
    *  @param [in]		staticDataTable		Name of the curve constructed by this method
    *  @param [in]		curveIndex			Equivalent names of the curve being built
    *  @param [in]		curveConv			General curve properties such as asofdate, ccy, interp, etc
    *  @param [in]		oisConv				The OIS curve configuration info
    *  @param [in]		oisRates			Constituent OIS instrument rates
    *  @param [in]		oisHistoricalRates	Historical OIS fixings
    *  @param [in]		liborOisBasisConv	Libor-OIS swap conventions
    *  @param [in]		liborOisBasisRates	Libor-OIS basis spreads
    *  @param [in]		swapConv			Libor swap conventions
    *  @param [in]		swapRates			Libor swap market rates
    */
    const AQLString tryMeLWOCurveCreateOIS( const std::string& lwoCurveName,
                                           const AQLString& curveCollectionInput,
                                           const AQLString& staticDataTableInput,
                                           const AQLString& curveIndexInput,
                                           const AQLStringMatrix& curveConv,
                                           const AQLStringMatrix& oisConv,
                                           const AQLStringMatrix& oisRates,
                                           const AQLStringMatrix& oisHistoricalRates,
                                           const AQLStringMatrix& liborOisBasisConv,
                                           const AQLStringMatrix& liborOisBasisRates,
                                           const AQLStringMatrix& swapConv,
                                           const AQLStringMatrix& swapRates )
    {
        VALID_EXCEPTION_START
        
        // LWO Single Curves Populate Curve Results Objects that Conflict with Other Curve Types, so we must clear the Curve Results Cache
        AQ_CLEAR_CURVE_RESULTS_CACHE

        // Ensure Curve Name Data is in uppercase
        // --------------------------------------

        AQLString curveCollection  = curveCollectionInput;
        curveCollection.toUpper();
        
        AQLString staticDataTable  = staticDataTableInput;
        staticDataTable.toUpper();

        // Append the staticDataTable to the curveIndex Name Set, ensuring to use the ':' delimiter
        AQLString curveIndex       = curveIndexInput + ":" + staticDataTable;
        curveIndex.toUpper();

        // --------------------------------------

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMeLWOCurveCalibrateOIS_inputs", curveCollection, staticDataTable ) );
            file.write( "generatorFunction", "tryMeCurveCalibrateOIS" );
            file.write( "lwoCurveName", lwoCurveName.c_str() );
            file.write( "curveCollection", curveCollection );
            file.write( "staticDataTable", staticDataTable );
            file.write( "curveIndex", curveIndex );
            file.write( "curveConv", curveConv );
            file.write( "oisConv", oisConv );
            file.write( "oisRates", oisRates );
            file.write( "oisHistoricalRates", oisHistoricalRates );
            file.write( "liborOisBasisConv", liborOisBasisConv );
            file.write( "liborOisBasisRates", liborOisBasisRates );
            file.write( "swapConv", swapConv );
            file.write( "swapRates", swapRates );
        }

        if ( !curveConv.empty() && curveConv[0].size() < 2 )
        {
            AQ_THROW("Invalid Data: Input matix data must have column size 2")
        }

        if ( !oisConv.empty() && oisConv[0].size() < 2 )
        {
            AQ_THROW("Invalid Data: Input matix data must have column size 2")
        }

        if ( !oisRates.empty() && oisRates[0].size() < 2 )
        {
            AQ_THROW("Invalid Data: Input matix data must have column size 2")
        }

        // Remove the Curve from the Object Pool Curve Engine if it is registered
        // ----------------------------------------------------------------------
        if(etrading::isCurveRegistered(curveCollection))
        {
            etrading::removeCurveFromEntityPool(curveCollection,curveIndex);
        }

        // Important Note on: CurveIndexCopy
        // ---------------------------------
        // The 'curveIndexCopy' variable is used to search for a curve for discount factors and forward rates. Unfortunately the object pool sometimes searches for
        // curves by 'staticDataTable' (aka MarketDataName) and sometimes by 'curveIndex'. To mitigate this problem we ensure that 'staticDataTable' name is always
        // included in the 'curveIndex' name list.
        AQLString curveIndexCopy( etrading::getDefaultValueForEmptyString( staticDataTable, "OIS" ) );


        // Build Curve using Object Pool Curve Engine
        // ------------------------------------------
		
		// 1. Curve Static Data
		etrading::AQLUpdateStaticDataManager::
		loadStaticDataOISCurve( etrading::getDataInstance(), curveCollection, staticDataTable, curveConv, oisRates, oisConv,
								curveIndex, oisHistoricalRates, liborOisBasisRates,  liborOisBasisConv, swapRates, swapConv );
	    
		// 2. Calibrate Curve
		etrading::AQLUpdateStaticDataManager::calibrateOISCurve( etrading::getDataInstance(), curveCollection, staticDataTable, curveConv );

        //Throw exception if the curve has not been built.
        etrading::getCurveStaticDataTableName( curveCollection, curveIndexCopy );

        // ------------------------------------------


        AQLString interpolation = etrading::trim_to_upper( etrading::getCurveInterpolation( curveCollection, staticDataTable ).getCString() ).c_str();
        auto& env = etrading::Environment::defaultEnv();

        // AsOfDate and Calendar to get the start of the year fractions (and whether date is a holiday -> not in AlgoQuantLib) +
        // CurveBuildProperties because it creates the AlgoQuantLib Calendar
        LabelValueBlock curveConvLVB( curveConv );
        AQLDate  effectiveDate = curveConvLVB.getCompulsoryValueAsDate( "ASOFDATE" );

        const boost::gregorian::date asOfDate( effectiveDate.yearOfEra(), effectiveDate.monthOfYear(), effectiveDate.dayOfMonth() );

        LabelValueBlock oisConvLVB( oisConv );
        const std::string calendar = etrading::trim_to_upper( oisConvLVB.getCompulsoryValue( "CALENDAR" ).c_str() );

        const std::string staticDataName = etrading::trim_to_upper( staticDataTable.getCString() );

        const std::string baseNameToUse = lwoCurveName;
        const std::string lwoCurveName =  baseNameToUse;

        const etrading::CurveTypeEnum curveTypeEnum = etrading::toCurveTypeEnum( staticDataName );
        const etrading::CCY ccy = etrading::toCCYEnum( etrading::trim_to_upper( curveConvLVB.getCompulsoryValue( "CURRENCY" ).c_str() ) );

        const etrading::CompoundingFrequencyEnum floatRateCompoundingFreq =
            etrading::toCompoundingFrequencyEnum( etrading::trim_to_upper( oisConvLVB.getCompulsoryValue( "FREQUENCY" ).c_str() ) );

        const etrading::CurveTenorEnum floatRateTenor = etrading::CURVE_TENOR_1D;

        const etrading::CompoundingMethodEnum oisCompoundingMethod =
            etrading::toCompoundingMethodEnum( etrading::trim_to_upper( oisConvLVB.getCompulsoryValue( "SHORTTERMCONVENTION" ).c_str() ) );

        const etrading::BusinessDayAdjustmentEnum busDayAdjustment =
            etrading::toBusinessDayAdjustmentEnum( etrading::trim_to_upper( oisConvLVB.getCompulsoryValue( "SLIDINGRULE" ).c_str() ) );

        const etrading::InterpolationEnum interpMethod =
            etrading::toInterpolationEnum( etrading::trim_to_upper( curveConvLVB.getCompulsoryValue( "YIELDGEN.INTERPOLATION" ).c_str() ) );
        
        // Curve Build Properties
        etrading::CurveBuildProperties oisCurveBuildProperties(	curveTypeEnum,
                                                                lwoCurveName + "_CBP",
                                                                std::string( curveCollection.getCString() ),
                                                                staticDataName,
                                                                ccy,
                                                                asOfDate,
                                                                interpMethod,
                                                                floatRateCompoundingFreq,
                                                                floatRateTenor,
                                                                oisCompoundingMethod,
                                                                busDayAdjustment,
                                                                calendar,
																{},    // interpolationParameters
                                                                true );    // onlyAllowLookup ... TRUE when we have daily discount factors stored, FALSE otherwise. The later relies on interpolation.


        // -----------------------------------------------------------------------------------------

        /*
        OPTION A: Store discount factors and forward rates on the yield curve node points only
        */
        
        // curveCollection
          //auto interpolationData    = etrading::AQLCurveForwardRateHelpers::getXY( etrading::getDataInstance(), curveCollection );
          //auto yearFractions        = std::get<0>( interpolationData );
          //auto discountFactors      = std::get<1>( interpolationData );
        
          //// Set the LWO Curve
          //etrading::AQOCurve lwoCurve( lwoCurveName, yearFractions, discountFactors, oisCurveBuildProperties );

        /*
        OPTION B: Store daily discount factors and forward rates for 51 years
        */

        boost::gregorian::date endDate  = asOfDate + boost::gregorian::years( 51 );
        auto numberOfDays               = boost::gregorian::date_period( asOfDate, endDate ).length().days();
        
        std::vector<AQLDate> dates;
        dates.reserve( numberOfDays );
        
        // this is the a way of generating the points using an add from the day count outwards...
        std::vector<double> yearFractions;

        yearFractions.reserve( numberOfDays );
        AQLPriceDataDayCount dc_act365( ACT_365 ); // ACT_365

        auto spotDate   = etrading::toAQLDateFromGregorianDate( asOfDate );
        auto finalDate  = etrading::toAQLDateFromGregorianDate( endDate );
        auto iterDate   = spotDate;

        while( iterDate < finalDate )
        {
            const double yearFrac = dc_act365.getTerm( spotDate, iterDate );
            yearFractions.push_back( yearFrac );
            dates.push_back( iterDate );
            iterDate.addDays( 1 );
        }

        // Calculate Discount Factors and Forwards & Set LWO Curve Container
        // -----------------------------------------------------------------

        auto discountFactors = etrading::AQLCurveForwardRateHelpers::getMultiDF( yearFractions,
                                                                           etrading::getDataInstance(),
                                                                           curveCollection,
                                                                           AQLString( "ACT/365" ),         // dayCount
                                                                           interpolation.toUpper(),
                                                                           false,                         // isBasis
                                                                           staticDataTable );

        // Get the Payment / Fixing Dates
        std::vector<boost::gregorian::date> datesInBoostFormat;
        datesInBoostFormat.reserve( dates.size() );

        // Populate datesInBoostFormat by transfoming 'dates' in AQLDate format
        std::transform( dates.cbegin(),
                        dates.cend(),
                        std::back_inserter( datesInBoostFormat ),
                        []( const AQLDate & AQLDate )
        {
            return etrading::toGregorianDateFromLADate( AQLDate );
        } );

        // Calculate the forward rates using the Object Pool Curve Engine
        auto forwardRates = etrading::getCurveForwardRates( dates, curveCollection, curveIndexCopy ); // Note we use curveIndexCopy, which is actually the staticDataTable

        // Set the LWO Curve; yearFractions, discountFactors, forward rates and Curve build properties (cbp) 
        etrading::AQOCurve lwoCurve( lwoCurveName,
                                     datesInBoostFormat,
                                     yearFractions,
                                     discountFactors,
                                     forwardRates,
                                     oisCurveBuildProperties );

        // End of Option B
        // -----------------------------------------------------------------------------------------
        
        // create a MarketDataCollectionObject
        // etrading::MarketDataCollection mdcOIS( std::string("MDC_") + lwoCurveName , floatRateTenor);

        // ----------------------------------------------------------------
        /*  EXECUTING CODE */
        etrading::moveToCache<etrading::AQOCurve>( std::move( lwoCurve ) );
        auto ptrToCurve = env.accessObject<etrading::AQOCurve>( lwoCurveName );

        if( ptrToCurve != nullptr )
        {
            // auto test = 	ptrToCurve->calculateDiscountFactor(1.546);
            if ( CreateDataFile::recordEnabled() )
            {
                CreateDataFile file( decorateCurvename( "tryMeLWOCurveCalibrateOIS_outputs", curveCollection, staticDataTable ) );
                file.write( "output", lwoCurveName );
            }
            return lwoCurveName.c_str();
        }
        else
        {
            std::string errString =  ( boost::format( "Unable to create AQOCurve named %s" ) % lwoCurveName.c_str() ).str();
            if ( CreateDataFile::recordEnabled() )
            {
                CreateDataFile file( decorateCurvename( "tryMeLWOCurveCalibrateOIS_outputs", curveCollection, staticDataTable ) );
                file.write( "output", errString.c_str() );
            }
            AQ_THROW( errString );
        }

        VALID_EXCEPTION_END
    }

}

