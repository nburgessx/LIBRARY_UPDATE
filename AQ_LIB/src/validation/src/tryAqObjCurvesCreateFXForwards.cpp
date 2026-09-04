#include <boost/date_time.hpp>

#include "tryAqObjCurvesCreateFXForwards.h"

#include "AQLUpdateStaticDataManager.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "CurveValidation.h"
#include "LabelValueBlockValidation.h"
#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"
#include "CurveBuildProperties.h"
#include "AQObjCurve.h"
#include "EnvironmentPool.h"
#include "ObjectUtilities.h"
#include "EnvironmentUtilities.h"
#include "Environment.h"
#include "DateUtilities.h"
#include "AQLCoreComponentManager.h"
#include "AQLCurvePricingObject.h"
#include "AQLCurveForwardRateHelpers.h"
#include "AQLPriceDataInterpolation.h"
#include "EntityPoolUtilities.h"
#include "CurveResultsContainer.h"


using namespace etrading;

namespace validation
{

    /* @brief			validation method for aqObjCurvesCreateFXForwards
    *  @param [in]		curveCollection		The curve collection ID
    *  @param [in]		staticDataTable		Name of the curve constructed by this method
    *  @param [in]		curveIndex			Equivalent names of the curve being built
    *  @param [in]		curveConv			General curve properties such as asofdate, ccy, interp, etc
    *  @param [in]		fxFwdConv			FX forward conventions
    */
    AQLString tryAqObjCurvesCreateFXForwards( const std::string& aqObjCurveName,
                                            const AQLString& curveCollectionInput,
                                            const AQLString& staticDataTableInput,
                                            const AQLString& curveIndexInput,
                                            const AQLStringMatrix& curveConv,
                                            const AQLStringMatrix& fxFwdConv )
    {
        VALID_EXCEPTION_START
        
        // AQObj Single Curves Populate Curve Results Objects that Conflict with Other Curve Types, so we must clear the Curve Results Cache
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
            CreateDataFile file( decorateCurvename( "tryAqObjCurvesCalibrateFXForwards_inputs", curveCollection, staticDataTable ) );
            file.write( "generatorFunction", "tryAqCurvesCalibrateFXForwards" );
            file.write( "aqObjCurveName", aqObjCurveName.c_str() );
            file.write( "curveCollection", curveCollection );
            file.write( "staticDataTable", staticDataTable );
            file.write( "curveIndex", curveIndex );
            file.write( "curveConv", curveConv );
            file.write( "fxFwdConv", fxFwdConv );
        }

        if( fxFwdConv.empty() )
        {
            throw AQLCoreInvalidData( "Input Matrix is empty", __FILE__, __LINE__ );
        }

        if( fxFwdConv[0].size() < 2 )
        {
            AQ_THROW("Invalid Data: Input matix data must have column size 2")
        }

        // Remove the Curve from the Object Pool Curve Engine if it is registered
        // ----------------------------------------------------------------------
        if(etrading::isCurveRegistered(curveCollection))
        {
            etrading::removeCurveFromEntityPool(curveCollection,curveIndex);
        }

        // Build Curve using Object Pool Curve Engine
        // ------------------------------------------

		// 1. Load Static Data
        AQLUpdateStaticDataManager::loadStaticDataFwdFXConstantCurve( etrading::getDataInstance(), curveCollection, staticDataTable, fxFwdConv, curveConv, curveIndex );

		// 2. Calibrate Curve
		AQLUpdateStaticDataManager::calibrateFwdFXConstantCurve( etrading::getDataInstance(), curveCollection, staticDataTable, curveConv );

        // Important Note on: CurveIndexCopy
        // ---------------------------------
        // The 'curveIndexCopy' variable is used to search for a curve for discount factors and forward rates. Unfortunately the object pool sometimes searches for
        // curves by 'staticDataTable' (aka MarketDataName) and sometimes by 'curveIndex'. To mitigate this problem we ensure that 'staticDataTable' name is always
        // included in the 'curveIndex' name list.
        AQLString curveIndexCopy( etrading::getDefaultValueForEmptyString( staticDataTable, "FWDFXCONST" ) );

        //Throw exception if the curve has not been built.
        etrading::getCurveStaticDataTableName( curveCollection, curveIndex );

        // ------------------------------------------


        LabelValueBlock curveConvLVB( curveConv );
        AQLDate  effectiveDate = curveConvLVB.getCompulsoryValueAsDate( "ASOFDATE" );

        const boost::gregorian::date asOfDate( effectiveDate.yearOfEra(), effectiveDate.monthOfYear(), effectiveDate.dayOfMonth() );
        const etrading::CCY ccy = etrading::toCCYEnum( curveConvLVB.getCompulsoryValue( "CURRENCY" ) );

        const std::string baseCurveName = aqObjCurveName;
        const std::string aqObjCurveName = baseCurveName;

        CurveBuildProperties cbp( aqObjCurveName + "_CBP", ccy, asOfDate );

        boost::gregorian::date endDate = asOfDate + boost::gregorian::years( 51 );
        auto numberOfdaysBetween = boost::gregorian::date_period( asOfDate, endDate ).length().days();
        
        std::vector<double> massiveYearFractionVector
            = etrading::stepVector( 0.0, 51.0 / static_cast<double>( numberOfdaysBetween ), numberOfdaysBetween );

        AQLString interpolation = etrading::trim_to_upper( etrading::getCurveInterpolation( curveCollection, staticDataTable ).getCString() ).c_str();


        // Calculate Discount Factors and Forwards & Set AQObj Curve Container
        // ----------------------------------------------------------------

        // Get the Discount Factors from the Object Pool Curve Engine
        auto massiveDFVector = etrading::AQLCurveForwardRateHelpers::getMultiDF( massiveYearFractionVector, etrading::getDataInstance(), curveCollection, AQLString( "ACT/365" ), interpolation.toUpper(), false, staticDataTable );

        // Set the AQObj Curve; yearFractions, discountFactors and Curve build properties (cbp)
        etrading::AQObjCurve aqObjCurve( aqObjCurveName, massiveYearFractionVector, massiveDFVector, cbp );

        // Get the Fixing Dates
        const auto& aqObjFixingDates = aqObjCurve.getDates();
        auto fixingDatesAsMlibDates = etrading::toAQLDatesFromGregorianDates( aqObjFixingDates );

        // Get the Forward Rates from the Object Pool Curve Engine
        auto massiveFwdRatesVector
            = etrading::getCurveForwardRates( fixingDatesAsMlibDates, curveCollection, curveIndexCopy ); // Note we use curveIndexCopy, which is actually the staticDataTable

        // Set the AQObj Curve; dates, discountFactors and forwardRates ... done twice to resolve a date consistency issue
        aqObjCurve.setData( aqObjCurve.
            getDates(), aqObjCurve.getDiscountFactors(), massiveFwdRatesVector );

        // ----------------------------------------------------------------


        etrading::moveToCache( std::move( aqObjCurve ) );
        auto& curve_store = etrading::getObjectStore<etrading::AQObjCurve>( etrading::Environment::DEFAULT_ENV_NAME );

        if( curve_store.has( aqObjCurveName ) )
        {
            AQLString ret = aqObjCurveName.c_str();
            if ( CreateDataFile::recordEnabled() )
            {
                CreateDataFile file( decorateCurvename( "tryAqObjCurvesCalibrateFXForwards_outputs", curveCollection, staticDataTable ) );
                file.write( "output", ret );
            }
            return ret;
        }
        else
        {
            AQ_THROW( ( boost::format( "Unable to create AQObjCurve named %s" ) % aqObjCurveName.c_str() ).str().c_str() );
        }

        VALID_EXCEPTION_END
    };

}
