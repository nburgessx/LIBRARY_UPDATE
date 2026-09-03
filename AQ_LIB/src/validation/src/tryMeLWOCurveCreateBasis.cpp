#include <memory>

#include "tryMeLWOCurveCreateBasis.h"

#include "LAUpdateStaticDataManager.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "CurveValidation.h"
#include "LabelValueBlockValidation.h"
#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"
#include "InterpolationParameters.h"
#include "LACurveForwardRateHelpers.h"
#include "LACurveCalibrationHelpers.h"
#include "LAInterpolationBase.h"
#include "LASplineInterpolation.h"
#include "LAConstrainedSplineInterpolation.h"
#include "LAMonotoneConvexInterpolation.h"
#include "LALinearInterpolation.h"
#include "LAStepInterpolation.h"
#include "LACoreComponentManager.h"
#include "LACurvePricingObject.h"
#include "LAPriceDataInterpolation.h"
#include "EntityPoolUtilities.h"


#include "CurveBuildProperties.h"
#include "Environment.h"
#include "EnvironmentPool.h"
#include "ObjectUtilities.h"
#include "LWOCurve.h"
#include "EnvironmentUtilities.h"
#include "DateUtilities.h"
#include "CurveResultsContainer.h"

using etrading::CreateDataFile;
using etrading::decorateCurvename;

namespace validation_api
{
    /* @brief			validation interface for the meLWOCurveCreateBasis method
    *  @param [in]		curveCollection		The curve collection ID
    *  @param [in]		staticDataTable		Name of the curve constructed by this method
    *  @param [in]		curveIndex			Equivalent names of the curve being built
    *  @param [in]		curveConv			General curve properties such as asofdate, ccy, interp, etc
    *  @param [in]		basisConv			Basis swap conventions
    *  @param [in]		basisRates			Basis swap market rates
    *  @param [in]		fxFwdConv			FX forward market convention
    *  @param [in]		fxFwdRates			Forward FX rates
    *  @param [in]		spotFxRates			Spot FX rates
    */
    const LAString tryMeLWOCurveCreateBasis( const std::string& lwoCurveName,
                                             const LAString& curveCollectionInput,
                                             const LAString& staticDataTableInput,
                                             const LAString& curveIndexInput,
                                             const LAStringMatrix& curveConv,
                                             const LAStringMatrix& basisConv,
                                             const LAStringMatrix& basisRates,
                                             const LAStringMatrix& fxFwdConv,
                                             const LAStringMatrix& fxFwdRates,
                                             const LAStringMatrix& spotFxRates )
    {
        VALID_EXCEPTION_START
        
        // LWO Single Curves Populate Curve Results Objects that Conflict with Other Curve Types, so we must clear the Curve Results Cache
        AQ_CLEAR_CURVE_RESULTS_CACHE

        // Ensure Curve Name Data is in uppercase
        // --------------------------------------

        LAString curveCollection  = curveCollectionInput;
        curveCollection.toUpper();
        
        LAString staticDataTable  = staticDataTableInput;
        staticDataTable.toUpper();

        // Append the staticDataTable to the curveIndex Name Set, ensuring to use the ':' delimiter
        LAString curveIndex       = curveIndexInput + ":" + staticDataTable;
        curveIndex.toUpper();

        // --------------------------------------

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMeLWOCurveCalibrateBasis_inputs", curveCollection, staticDataTable ) );
            file.write( "generatorFunction", "tryMeLWOCurveCreateBasis" );
            file.write( "lwoCurveName", lwoCurveName );
            file.write( "curveCollection", curveCollection );
            file.write( "staticDataTable", staticDataTable );
            file.write( "curveIndex", curveIndex );
            file.write( "curveConv", curveConv );
            file.write( "basisConv", basisConv );
            file.write( "basisRates", basisRates );
            file.write( "fxFwdConv", fxFwdConv );
            file.write( "fxFwdRates", fxFwdRates );
            file.write( "spotFxRates", spotFxRates );
        }

        if( ( fxFwdRates.empty() && basisRates.empty() ) || basisConv.empty() || curveConv.empty() )
        {
            throw LACoreInvalidData( "Input Matrix is empty", __FILE__, __LINE__ );
        }

        if( ( fxFwdRates.empty() && 2 > basisRates[0].size() ) || 2 > basisConv[0].size() || 2 > curveConv[0].size() )
        {
            AQ_THROW("Invalid Data: Input matix data must have column size 2")
        }

        if ( !fxFwdRates.empty() && fxFwdRates[0].size() < 2 )
        {
            AQ_THROW("Invalid Data: Input matix data must have column size 2")
        }

        if ( !fxFwdConv.empty() && fxFwdConv[0].size() < 2 )
        {
            AQ_THROW("Invalid Data: Input matix data must have column size 2")
        }

        if ( !spotFxRates.empty() && spotFxRates[0].size() < 3 )
        {
            AQ_THROW("Invalid Data: Input matix data must have column size 3")
        }

        LAStringMatrix moneyConv = LAStringMatrix( 0 );


        // Remove the Curve from the Object Pool Curve Engine if it is registered
        // ----------------------------------------------------------------------
        if(etrading::isCurveRegistered(curveCollection))
        {
            etrading::removeCurveFromEntityPool(curveCollection,curveIndex);
        }

        // Build Curve using Object Pool Curve Engine
        // ------------------------------------------

		// 1. Load Static Data
        etrading::LAUpdateStaticDataManager::loadStaticDataBasisCurve( etrading::getDataInstance(),
																	   curveCollection,
																	   staticDataTable,
																	   basisRates,
																	   basisConv,
																	   fxFwdRates,
																	   fxFwdConv,
																	   spotFxRates,
																	   curveConv,
																	   moneyConv,
																	   curveIndex );
		// 2. Calibrate Curve
		etrading::LAUpdateStaticDataManager::calibrateBasisCurve( etrading::getDataInstance(),
																  curveCollection,
																  staticDataTable,
																  basisConv );

        // Important Note on: CurveIndexCopy
        // ---------------------------------
        // The 'curveIndexCopy' variable is used to search for a curve for discount factors and forward rates. Unfortunately the object pool sometimes searches for
        // curves by 'staticDataTable' (aka MarketDataName) and sometimes by 'curveIndex'. To mitigate this problem we ensure that 'staticDataTable' name is always
        // included in the 'curveIndex' name list.
        LAString curveIndexCopy( etrading::getDefaultValueForEmptyString( staticDataTable, "3M6MBASIS" ) );
        
        //Throw exception if the curve has not been built.
        etrading::getCurveStaticDataTableName( curveCollection, curveIndexCopy );

        // ------------------------------------------

        
        LAString interpolation = etrading::trim_to_upper( etrading::getCurveInterpolation( curveCollection, staticDataTable ).getCString() ).c_str();
        auto& env = etrading::Environment::defaultEnv();

        // AsOfDate and Calendar to get the start of the year fractions (and whether date is a holiday -> not in MLIBQ) +
        // CurveBuildProperties because it creates the MLIBQ Calendar
        LabelValueBlock curveConvLVB( curveConv );
        LADate  effectiveDate = curveConvLVB.getCompulsoryValueAsDate( "ASOFDATE" );
        const etrading::CCY ccy = etrading::toCCYEnum( curveConvLVB.getCompulsoryValue( "CURRENCY" ) );

        const boost::gregorian::date asOfDate( effectiveDate.yearOfEra(), effectiveDate.monthOfYear(), effectiveDate.dayOfMonth() );
        LabelValueBlock basisConvLVB( basisConv );

        const std::string staticDataName = etrading::trim_to_upper( staticDataTable.getCString() );

        const std::string baseOfCurveName = lwoCurveName;
        const std::string	newCurveName = baseOfCurveName;


        const etrading::CurveTypeEnum curveTypeEnum = etrading::toCurveTypeEnum( staticDataName );

        const bool isLeg2 =  ( etrading::trim_to_upper( basisConvLVB.getCompulsoryValue( "TARGET" ).c_str() ) == "LEG2FORECAST" );

        const std::string leg1calendar = basisConvLVB.getCompulsoryValue( "Leg1index.fixing.calendar" ); // TODO: save all the extra stuff...!!!
        const std::string leg2calendar = basisConvLVB.getCompulsoryValue( "Leg2index.fixing.calendar" );

        const etrading::CompoundingFrequencyEnum floatRateCompoundingFreq =
            etrading::toCompoundingFrequencyEnum( basisConvLVB.getCompulsoryValue( isLeg2 ? "LEG2INDEX.FREQUENCY" : "LEG1INDEX.FREQUENCY" ) );

        // ACCESSARY, really... stuck because in use in many XL sheets1
        const etrading::CurveTenorEnum floatRateTenor =
            etrading::toCurveTenorEnum( basisConvLVB.getCompulsoryValue( isLeg2 ? "LEG2INDEX.ACCESSARY" : "LEG1INDEX.ACCESSARY" ) ) ;
        const etrading::CompoundingMethodEnum oisCompoundingMethod = etrading::SIMPLE_COMPOUNDING_METHOD; // hard coded to simple at the moment...

        const etrading::BusinessDayAdjustmentEnum busDayAdjustment =
            etrading::toBusinessDayAdjustmentEnum( basisConvLVB.getCompulsoryValue( isLeg2 ? "LEG2CASHLET.SLIDINGRULE" : "LEG1CASHLET.SLIDINGRULE" ) );

        const etrading::InterpolationEnum interpMethod = etrading::toInterpolationEnum( curveConvLVB.getCompulsoryValue( "BASIS.INTERPOLATION" ) );

        etrading::CurveBuildProperties stdCurveBuildProperties(	curveTypeEnum,
                newCurveName + "_CBP", std::string( curveCollection.getCString() ), staticDataName,
                ccy, asOfDate, interpMethod, floatRateCompoundingFreq, floatRateTenor,
			    oisCompoundingMethod, busDayAdjustment, isLeg2 ?  leg2calendar : leg1calendar, {}, true );

        // call this paragraph of code if you wish to populate discount factors for every day
        // this method retrieves 51 years of discount factors and places it in the LWOCurve
        boost::gregorian::date endDate  = asOfDate + boost::gregorian::years( 51 );
        auto numberOfdaysBetween        = boost::gregorian::date_period( asOfDate, endDate ).length().days();
        

        // Calculate Discount Factors and Forwards & Set LWO Curve Container
        // ----------------------------------------------------------------
        
        // Calculate the year fractions
        std::vector<double> massiveYearFractionVector =
            etrading::stepVector( 0.0, 51.0 / static_cast<double>( numberOfdaysBetween ), numberOfdaysBetween );
        
        // Calculate the discount factors using the Object Pool Curve Engine
        auto massiveDFVector = etrading::LACurveForwardRateHelpers::getMultiDF( massiveYearFractionVector,
                                                                           etrading::getDataInstance(),
                                                                           curveCollection,
                                                                           LAString( "ACT/365" ),
                                                                           interpolation.toUpper(),
                                                                           false,
                                                                           staticDataTable );

        // Set the LWO Curve; yearFractions, discountFactors and Curve build properties (cbp)
        etrading::LWOCurve lwoCurve( newCurveName, massiveYearFractionVector, massiveDFVector, stdCurveBuildProperties );
        
        // Get the Fixing Dates
        const auto& lwoFixingDates = lwoCurve.getDates();
        auto fixingDatesAsMlibDates = etrading::toLADatesFromGregorianDates( lwoFixingDates );
        
        // Get the Forward Rates from the Object Pool Curve Engine
        auto massiveFwdRatesVector
            = etrading::getCurveForwardRates( fixingDatesAsMlibDates, curveCollection, curveIndexCopy ); // Note we use curveIndexCopy, which is actually the staticDataTable

        // Set the LWO Curve; dates, discountFactors and forwardRates ... done twice to resolve a date consistency issue
        lwoCurve.setData( lwoCurve.getDates(), lwoCurve.getDiscountFactors(), massiveFwdRatesVector );

        // ----------------------------------------------------------------


        etrading::moveToCache( std::move( lwoCurve ) );

        auto& default_env = etrading::getObjectStore<etrading::LWOCurve>( etrading::Environment::DEFAULT_ENV_NAME );
        if( default_env.has( newCurveName ) )
        {
            LAString ret = newCurveName.c_str();
            if ( CreateDataFile::recordEnabled() )
            {
                CreateDataFile file( decorateCurvename( "tryMeLWOCurveCalibrateSwap_outputs", curveCollection, staticDataTable ) );
                file.write( "output", ret );
            }
            return ret;
        }
        else
        {
            AQ_THROW( ( boost::format( "Unable to create LWOCurve named %s" ) % newCurveName.c_str() ).str().c_str() );
        }

        /*
        	********** We display a message indicating the success of the curve build.
        	********** The long term and more appropriate plan is to display an object handle rather than simply a message
        */
        VALID_EXCEPTION_END


    }


}
