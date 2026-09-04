#include <memory>
#include <boost/date_time.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/range/irange.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/numeric.hpp>

#include "tryMeLWOCurveCreateSwap.h"
#include "InterpolationParameters.h"

#include "AQLUpdateStaticDataManager.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "CurveValidation.h"
#include "ParameterValidation.h"
#include "LabelValueBlockValidation.h"
#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"
#include "AQLCurveCalibrationHelpers.h"
#include "AQLCurveForwardRateHelpers.h"
#include "AQLMonotoneConvexInterpolation.h"
#include "AQLSplineInterpolation.h"
#include "AQLConstrainedSplineInterpolation.h"
#include "AQLLinearInterpolation.h"
#include "AQLStepInterpolation.h"
#include "ObjectUtilities.h"
#include "AQOCurve.h"
#include "EnvironmentUtilities.h"
#include "Environment.h"
#include "DateUtilities.h"
#include "AQLCoreComponentManager.h"
#include "AQLCurvePricingObject.h"
#include "AQLPriceDataInterpolation.h"
#include "tryMeCurveForwardRate.h"
#include "EntityPoolUtilities.h"
#include "CurveResultsContainer.h"

using etrading::CreateDataFile;
using etrading::decorateCurvename;

namespace validation
{

    /* @brief			validation interface for meLWOCurveCreateSwap
    *  @param [in]		curveCollection		The curve collection ID
    *  @param [in]		staticDataTable		Name of the curve constructed by this method
    *  @param [in]		curveIndex			Equivalent names of the curve being built
    *  @param [in]		curveConv			General yield curve data
    *  @param [in]		moneyMarketConv			Money market conventions
    *  @param [in]		liborConv			Libor market conventions
    *  @param [in]		liborRates			Libor market data
    *  @param [in]		swapConv			Swap market conventions
    *  @param [in]		swapRates			Swap market data
    *  @param [in]		fraConv				FRA market conventions
    *  @param [in]		fra3mRates			3M FRA market data
    *  @param [in]		fra6mRates			6M FRA market data
    *  @param [in]		futureConv			Futures market conventions
    *  @param [in]		futureRates			Futures market data
    *  @param [in]		convexityAdjConv	Convexity adjustment market conventions
    *  @param [in]		convexityAdjRates	Convexity adjustment market data
    */
    const AQLString tryMeLWOCurveCreateSwap( const std::string& lwoCurveName,
                                            const AQLString& curveCollectionInput,
                                            const AQLString& staticDataTableInput,
                                            const AQLString& curveIndexInput,
                                            const AQLStringMatrix& curveConv,
                                            const AQLStringMatrix& moneyMarketConv,
                                            const AQLStringMatrix& liborConv,
                                            const AQLStringMatrix& liborRates,
                                            const AQLStringMatrix& swapConv,
                                            const AQLStringMatrix& swapRates,
                                            const AQLStringMatrix& fraConv,
                                            const AQLStringMatrix& fra3mRates,
                                            const AQLStringMatrix& fra6mRates,
                                            const AQLStringMatrix& futureConv,
                                            const AQLStringMatrix& futureRates,
                                            const AQLStringMatrix& convexityAdjConv,
                                            const AQLStringMatrix& convexityAdjRates )
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
            CreateDataFile file( decorateCurvename( "tryMeLWOCurveCalibrateSwap_inputs", curveCollection, staticDataTable ) );
            file.write( "generatorFunction", "tryMeCurveCalibrateSwap" );
            file.write( "lwoCurveName", lwoCurveName.c_str()  );
            file.write( "curveCollection", curveCollection );
            file.write( "staticDataTable", staticDataTable );
            file.write( "curveIndex", curveIndex );
            file.write( "curveConv", curveConv );
            file.write( "moneyMarketConv", moneyMarketConv );
            file.write( "liborConv", liborConv );
            file.write( "liborRates", liborRates );
            file.write( "swapConv", swapConv );
            file.write( "swapRates", swapRates );
            file.write( "fraConv", fraConv );
            file.write( "fra3mRates", fra3mRates );
            file.write( "fra6mRates", fra6mRates );
            file.write( "futureConv", futureConv );
            file.write( "futureRates", futureRates );
            file.write( "convexityAdjConv", convexityAdjConv );
            file.write( "convexityAdjRates", convexityAdjRates );
        }

        if ( !curveConv.empty() && curveConv[0].size() < 2 )
        {
            AQ_THROW("Invalid Data: Input matix data must have column size 2")
        }

        if ( !moneyMarketConv.empty() && moneyMarketConv[0].size() < 2 )
        {
            AQ_THROW("Invalid Data: Input matix data must have column size 2")
        }

        if ( !liborConv.empty() && liborConv[0].size() < 2 )
        {
            AQ_THROW("Invalid Data: Input matix data must have column size 2")
        }

        if ( !liborRates.empty() && liborRates[0].size() < 2 )
        {
            AQ_THROW("Invalid Data: Input matix data must have column size 2")
        }

        if ( !swapConv.empty() && swapConv[0].size() < 2 )
        {
            AQ_THROW("Invalid Data: Input matix data must have column size 2")
        }

        if ( !swapRates.empty() && swapRates[0].size() < 2 )
        {
            AQ_THROW("Invalid Data: Input matix data must have column size 2")
        }

        if ( !fraConv.empty() && fraConv[0].size() < 2 )
        {
            AQ_THROW("Invalid Data: Input matix data must have column size 2")
        }

        if ( !fra3mRates.empty() && fra3mRates[0].size() < 2 )
        {
            AQ_THROW("Invalid Data: Input matix data must have column size 2")
        }

        if ( !fra6mRates.empty() && fra6mRates[0].size() < 2 )
        {
            AQ_THROW("Invalid Data: Input matix data must have column size 2")
        }

        if ( !futureConv.empty() && futureConv[0].size() < 2 )
        {
            AQ_THROW("Invalid Data: Input matix data must have column size 2")
        }

        if ( !futureRates.empty() && futureRates[0].size() < 3 )
        {
            throw AQLCoreInvalidData( "Matrix column size must be greater or equal to 3", __FILE__, __LINE__ );
        }

        if ( !convexityAdjConv.empty() && convexityAdjConv[0].size() < 2 )
        {
            AQ_THROW("Invalid Data: Input matix data must have column size 2")
        }

        if ( !convexityAdjRates.empty() && convexityAdjRates[0].size() < 2 )
        {
            AQ_THROW("Invalid Data: Input matix data must have column size 2")
        }

        // Remove the Curve from the Object Pool Curve Engine if it is registered
        // ----------------------------------------------------------------------
        if(etrading::isCurveRegistered(curveCollection))
        {
            // TODO: remove the first curveCollection argument being passed through, ccy will get deleted from the argument list
            etrading::removeCurveFromEntityPool(curveCollection,curveIndex);
        }

        // Build Curve using Object Pool Curve Engine
        // ------------------------------------------
        
		// 1. Load Curve Static Data
		etrading::AQLUpdateStaticDataManager::loadStaticDataSwapCurve( etrading::getDataInstance(),
																	  curveCollection,
																	  staticDataTable,
																	  curveConv,
																	  moneyMarketConv,
																	  liborRates,
																	  liborConv,
																	  swapRates,
																	  swapConv,
																	  fra3mRates,
																	  fra6mRates,
																	  fraConv,
																	  futureRates,
																	  futureConv,
																	  convexityAdjConv,
																	  convexityAdjRates,
																	  curveIndex,
																	  AQLString( "" ) );

		// 2. Calibrate Curve
		etrading::AQLUpdateStaticDataManager::calibrateSwapCurve( etrading::getDataInstance(), curveCollection, staticDataTable, curveConv );

        //Throw exception if the curve has not been built.
        etrading::getCurveStaticDataTableName( curveCollection, curveIndex );

        // Important Note on: CurveIndexCopy
        // ---------------------------------
        // The 'curveIndexCopy' variable is used to search for a curve for discount factors and forward rates. Unfortunately the object pool sometimes searches for
        // curves by 'staticDataTable' (aka MarketDataName) and sometimes by 'curveIndex'. To mitigate this problem we ensure that 'staticDataTable' name is always
        // included in the 'curveIndex' name list.
        AQLString curveIndexCopy( etrading::getDefaultValueForEmptyString( staticDataTable, "STD" ) );

        //Throw exception if the curve has not been built.
        etrading::getCurveStaticDataTableName( curveCollection, curveIndexCopy );

        // ------------------------------------------


        AQLString interpolation = etrading::trim_to_upper( etrading::getCurveInterpolation( curveCollection, staticDataTable ).getCString() ).c_str();
        auto& env =  etrading::Environment::defaultEnv();

        // AsOfDate and Calendar to get the start of the year fractions (and whether date is a holiday -> not in AlgoQuantLib) +
        // CurveBuildProperties because it creates the AlgoQuantLib Calendar
        const std::string staticDataName = etrading::trim_to_upper( staticDataTable.getCString() );

        const std::string baseCurveName = lwoCurveName;
        const std::string curveNameForLWOCurve =  baseCurveName;

        const etrading::CurveTypeEnum curveTypeEnum = etrading::toCurveTypeEnum( staticDataName );

        LabelValueBlock curveConvLVB( curveConv );
        AQLDate effectiveDate = curveConvLVB.getCompulsoryValueAsDate( "ASOFDATE" );

        const boost::gregorian::date asOfDate( effectiveDate.yearOfEra(), effectiveDate.monthOfYear(), effectiveDate.dayOfMonth() );
        const etrading::CCY ccy = etrading::toCCYEnum( curveConvLVB.getCompulsoryValue( "CURRENCY" ) );

        LabelValueBlock swapConvLVB( swapConv );
        const std::string calendar = swapConvLVB.getCompulsoryValue( "CALENDAR" );

        const etrading::CompoundingFrequencyEnum floatRateCompoundingFreq =
            etrading::toCompoundingFrequencyEnum( swapConvLVB.getCompulsoryValue( "FREQUENCYFLOAT" ) );

        const etrading::CurveTenorEnum floatRateTenor =
            etrading::toCurveTenorEnum( etrading::toTermString( floatRateCompoundingFreq ) );

        const etrading::CompoundingMethodEnum oisCompoundingMethod =
            etrading::SIMPLE_COMPOUNDING_METHOD; // not used for STD curve

        const etrading::BusinessDayAdjustmentEnum busDayAdjustment =
            etrading::toBusinessDayAdjustmentEnum( swapConvLVB.getCompulsoryValue( "SLIDINGRULE" ) );

        const etrading::InterpolationEnum interpMethod =
            etrading::toInterpolationEnum( curveConvLVB.getCompulsoryValue( "YIELDGEN.INTERPOLATION" ) );

        etrading::CurveBuildProperties stdCurveBuildProperties(	curveTypeEnum,
                                                                curveNameForLWOCurve + "_CBP",
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
																{},
                                                                true );

        // call this paragraph of code if you wish to populate discount factors for every day
        // this method retrieves 51 years of discount factors and places it in the AQOCurve
        boost::gregorian::date endDate  = asOfDate + boost::gregorian::years( 51 );
        auto numberOfdaysBetween        = boost::gregorian::date_period( asOfDate, endDate ).length().days();

        std::vector<AQLDate> massiveDateVector;
        massiveDateVector.reserve( numberOfdaysBetween );

        std::vector<double> massiveYearFractionVector;
        massiveYearFractionVector.reserve( numberOfdaysBetween );
        AQLPriceDataDayCount dc_act365( ACT_365 ); // ACT_365

        auto spotDate = etrading::toAQLDateFromGregorianDate( asOfDate );
        auto finalDate = etrading::toAQLDateFromGregorianDate( endDate );
        auto iterDate = spotDate;

        while( iterDate <= finalDate )
        {
            const double yearFrac = dc_act365.getTerm( spotDate, iterDate );
            massiveYearFractionVector.push_back( yearFrac );
            massiveDateVector.push_back( iterDate );
            iterDate.addDays( 1 );
        }


        // Calculate Discount Factors and Forwards & Set LWO Curve Container
        // ----------------------------------------------------------------

        // Calculate the discount factors using the Object Pool Curve Engine
        auto massiveDFVector = etrading::AQLCurveForwardRateHelpers::getMultiDF( massiveYearFractionVector,
                                                                           etrading::getDataInstance(),
                                                                           curveCollection,
                                                                           AQLString( "ACT/365" ),
                                                                           interpolation.toUpper(),
                                                                           false,                     // isBasisFlag
                                                                           staticDataTable );
        
        // Calculate the forward rates using the Object Pool Curve Engine
        auto massiveFwdRatesVector
            = etrading::getCurveForwardRates( massiveDateVector, curveCollection, curveIndexCopy ); // Note we use curveIndexCopy, which is actually the staticDataTable

        // Get the Payment / Fixing Dates
        std::vector<boost::gregorian::date> massiveDateVectorBoost;
        massiveDateVectorBoost.reserve( massiveDateVector.size() );

        std::transform( massiveDateVector.cbegin(),
                        massiveDateVector.cend(),
                        std::back_inserter( massiveDateVectorBoost ),
                        []( const AQLDate & AQLDate )
        {
            return etrading::toGregorianDateFromLADate( AQLDate );
        } );


        // Set the LWO Curve; yearFractions, discountFactors, forward rates and Curve build properties (cbp)
        etrading::AQOCurve lwoCurve( curveNameForLWOCurve,
                                     massiveDateVectorBoost,
                                     massiveYearFractionVector,
                                     massiveDFVector,
                                     massiveFwdRatesVector,
                                     stdCurveBuildProperties );

        // ----------------------------------------------------------------


        etrading::moveToCache(  std::move( lwoCurve ) );
        auto& curve_store = etrading::getObjectStore<etrading::AQOCurve>( etrading::Environment::DEFAULT_ENV_NAME );
        
        if( curve_store.has( curveNameForLWOCurve ) )
        {
            AQLString ret = curveNameForLWOCurve.c_str();
            if ( CreateDataFile::recordEnabled() )
            {
                CreateDataFile file( decorateCurvename( "tryMeLWOCurveCalibrateSwap_outputs", curveCollection, staticDataTable ) );
                file.write( "output", ret );
            }
            return ret;
        }
        else
        {
            AQ_THROW( ( boost::format( "Unable to create AQOCurve named %s" ) % curveNameForLWOCurve.c_str() ).str().c_str() );
        }

        VALID_EXCEPTION_END
    }

}

