// InterestRateSwap.cpp


#include "InterestRateSwap.h"
#include "LAMultiSwapPricer.h"
#include "LACoreError.h"
#include "tryMirGetParRate.h"
#include "tryMirSwapPV.h"
#include "tryMirSwapPV01.h"

namespace google_test
{

    InterestRateSwap::InterestRateSwap( LADataInstance* dataInstance, const ReadDataFile::Load& inputFile )
    {
        // Initialize the dataInstance
        dataInstance_                       = dataInstance;

        //Schedule parameters:
        effectDt_			= LAString( inputFile[ "effectDt" ]() );
        maturity_			= LAString( inputFile[ "maturity" ]() );
        xFirstStub_			= LAString( inputFile[ "xFirstStub" ]() );
        xLastStub_			= LAString( inputFile[ "xLastStub" ]() );
        xRollDay_			= LAString( inputFile[ "xRollDay" ]() );
        tFirstStub_			= LAString( inputFile[ "tFirstStub" ]() );
        tLastStub_			= LAString( inputFile[ "tLastStub" ]() );
        tRollDay_			= LAString( inputFile[ "tRollDay" ]() );
        xFreq_				= LAString( inputFile[ "xFreq" ]() );
        tFreq_				= LAString( inputFile[ "tFreq" ]() );
        eomRoll_			= inputFile[ "eomRoll" ];
        xRollCnv_			= LAString( inputFile[ "xRollCnv" ]() );
        xCalendar_			= LAString( inputFile[ "xCalendar" ]() );
        tRollCnv_		    = LAString( inputFile[ "tRollCnv" ]() );
        tCalendar_		    = LAString( inputFile[ "tCalendar" ]() );
        xStub_			    = LAString( inputFile[ "xStub" ]() );
        tStub_				= LAString( inputFile[ "tStub" ]() );
        xPayLag_			= LAString( inputFile[ "xPayLag" ]() );
        tPayLag_			= LAString( inputFile[ "tPayLag" ]() );
        tFixLag_			= LAString( inputFile[ "tFixLag" ]() );

        // parRate Parameters: Mandatory
        curveID_                    = LAString( inputFile[ "curveID" ]() );
        xDayCount_			        = LAString( inputFile[ "xDayCount" ]() );
        tDayCount_					= LAString( inputFile[ "tDayCount" ]() );
        tFirstFix_					= inputFile[ "tFirstFix" ];
        tLastFix_					= inputFile[ "tLastFix" ];
        interpolation_				= LAString( inputFile[ "interpolation" ]() );
        forecastCurve_				= LAString( inputFile[ "forecastCurve" ]() );
        discountCurve_				= LAString( inputFile[ "discountCurve" ]() );
        interpFwds_					= inputFile[ "interpFwds" ];

        // parRate Parameters: Optional
        tSpd_						= inputFile.getOptional( "tSpd",                         0.0 );
        useFwdData_                 = inputFile.getOptional( "useFwdData",                   false );
        isOIS_                      = inputFile.getOptional( "isOIS",                        false );
        oisCompoundingType_         = inputFile.getOptional( "oisCompoundingType",           LAString() );
        calendar_                   = inputFile.getOptional( "calendar",                     LAString() );
        rollConvention_             = inputFile.getOptional( "rollConvention",               LAString() );
        slidingRule_                = inputFile.getOptional( "slidingRule",                  LAString() );

        // swapPV Parameters: Mandatory ( inlcuded in the swapPV parameters but not included in parRate function parameters )
        payRec_						= inputFile.getOptional( "payRec",						LAString( "PAYER" ) );
        notional_                   = inputFile.getOptional( "notional",                     1000000 );
        fixedRate_                  = inputFile.getOptional( "fixedRate",                    0.0 );

        // swapPV Parameters: Optional
        floatSpreadInBasisPoints_   = inputFile.getOptional( "floatSpreadInBasisPoints",     0.0 );
        compoundingMethod_          = inputFile.getOptional( "compoundingMethod",            LAString() );
        floatCalendar_              = inputFile.getOptional( "floatCalendar",                LAString() );
        floatRollConv_              = inputFile.getOptional( "floatRollConv",                LAString() );

    }

    const double InterestRateSwap::parRate()
    {

        const double parRate = validation_api::tryMirGetParRate4(
                                   dataInstance_,
                                   effectDt_,
                                   maturity_,
                                   curveID_,
                                   xFreq_,
                                   xDayCount_,
                                   xRollCnv_,
                                   xCalendar_,
                                   xFirstStub_,
                                   xLastStub_,
                                   xRollDay_,
                                   xPayLag_,
                                   xStub_,
                                   tFreq_,
                                   tDayCount_,
                                   tRollCnv_,
                                   tCalendar_,
                                   tFirstStub_,
                                   tLastStub_,
                                   tRollDay_,
                                   tFixLag_,
                                   tFirstFix_,
                                   tLastFix_,
                                   tPayLag_,
                                   tStub_,
                                   interpolation_,
                                   forecastCurve_,
                                   discountCurve_,
                                   interpFwds_,
                                   eomRoll_,
                                   tSpd_,
                                   useFwdData_ );



        return parRate;
    }

    const double InterestRateSwap::pv()
    {


        const double pv = validation_api::tryMirSwapPV(
                              dataInstance_,
                              notional_,
                              payRec_,
                              effectDt_,
                              maturity_,
                              curveID_,
                              fixedRate_,
                              xFreq_,
                              xDayCount_,
                              xRollCnv_,
                              xCalendar_,
                              xFirstStub_,
                              xLastStub_,
                              xRollDay_,
                              xPayLag_,
                              xStub_,
                              tFreq_,
                              tDayCount_,
                              tRollCnv_,
                              tCalendar_,
                              tFirstStub_,
                              tLastStub_,
                              tRollDay_,
                              tFixLag_,
                              tFirstFix_,
                              tLastFix_,
                              tPayLag_,
                              tStub_,
                              interpolation_,
                              forecastCurve_,
                              discountCurve_,
                              interpFwds_,
                              eomRoll_,
                              tSpd_ );
        return pv;
    }

    const double InterestRateSwap::pv( const double& parRate )
    {

        const double parPV = validation_api::tryMirSwapPV(
                                 dataInstance_,
                                 notional_,
                                 payRec_,
                                 effectDt_,
                                 maturity_,
                                 curveID_,
                                 parRate,   // fixedRate = parRate
                                 xFreq_,
                                 xDayCount_,
                                 xRollCnv_,
                                 xCalendar_,
                                 xFirstStub_,
                                 xLastStub_,
                                 xRollDay_,
                                 xPayLag_,
                                 xStub_,
                                 tFreq_,
                                 tDayCount_,
                                 tRollCnv_,
                                 tCalendar_,
                                 tFirstStub_,
                                 tLastStub_,
                                 tRollDay_,
                                 tFixLag_,
                                 tFirstFix_,
                                 tLastFix_,
                                 tPayLag_,
                                 tStub_,
                                 interpolation_,
                                 forecastCurve_,
                                 discountCurve_,
                                 interpFwds_,
                                 eomRoll_,
                                 tSpd_ );

        return parPV;
    }

	 const double InterestRateSwap::pv01()
    {
		const double pv01 = validation_api::tryMirSwapPV01( dataInstance_,
															notional_,
															payRec_,
															effectDt_,
															maturity_,
															curveID_,
															xFreq_,
															xDayCount_,
															xRollCnv_,
															xCalendar_,
															xFirstStub_,
															xLastStub_,
															xRollDay_,
															xPayLag_,
															xStub_,
															tSpd_,
															tFreq_,
															tDayCount_,
															tRollCnv_,
															tCalendar_,
															tFirstStub_,
															tLastStub_,
															tRollDay_,
															tFixLag_,
															tFirstFix_,
															tLastFix_,
															tPayLag_,
															tStub_,
															interpolation_,
															forecastCurve_,
															discountCurve_,
															interpFwds_,
															eomRoll_ );

		return pv01;
    }
}
