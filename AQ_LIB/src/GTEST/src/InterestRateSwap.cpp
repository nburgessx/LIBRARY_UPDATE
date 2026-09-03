// InterestRateSwap.cpp


#include "InterestRateSwap.h"
#include "AQLMultiSwapPricer.h"
#include "AQLCoreError.h"
#include "tryMirGetParRate.h"
#include "tryMirSwapPV.h"
#include "tryMirSwapPV01.h"

namespace google_test
{

    InterestRateSwap::InterestRateSwap( AQLDataInstance* dataInstance, const ReadDataFile::Load& inputFile )
    {
        // Initialize the dataInstance
        dataInstance_                       = dataInstance;

        //Schedule parameters:
        effectDt_			= AQLString( inputFile[ "effectDt" ]() );
        maturity_			= AQLString( inputFile[ "maturity" ]() );
        xFirstStub_			= AQLString( inputFile[ "xFirstStub" ]() );
        xLastStub_			= AQLString( inputFile[ "xLastStub" ]() );
        xRollDay_			= AQLString( inputFile[ "xRollDay" ]() );
        tFirstStub_			= AQLString( inputFile[ "tFirstStub" ]() );
        tLastStub_			= AQLString( inputFile[ "tLastStub" ]() );
        tRollDay_			= AQLString( inputFile[ "tRollDay" ]() );
        xFreq_				= AQLString( inputFile[ "xFreq" ]() );
        tFreq_				= AQLString( inputFile[ "tFreq" ]() );
        eomRoll_			= inputFile[ "eomRoll" ];
        xRollCnv_			= AQLString( inputFile[ "xRollCnv" ]() );
        xCalendar_			= AQLString( inputFile[ "xCalendar" ]() );
        tRollCnv_		    = AQLString( inputFile[ "tRollCnv" ]() );
        tCalendar_		    = AQLString( inputFile[ "tCalendar" ]() );
        xStub_			    = AQLString( inputFile[ "xStub" ]() );
        tStub_				= AQLString( inputFile[ "tStub" ]() );
        xPayLag_			= AQLString( inputFile[ "xPayLag" ]() );
        tPayLag_			= AQLString( inputFile[ "tPayLag" ]() );
        tFixLag_			= AQLString( inputFile[ "tFixLag" ]() );

        // parRate Parameters: Mandatory
        curveID_                    = AQLString( inputFile[ "curveID" ]() );
        xDayCount_			        = AQLString( inputFile[ "xDayCount" ]() );
        tDayCount_					= AQLString( inputFile[ "tDayCount" ]() );
        tFirstFix_					= inputFile[ "tFirstFix" ];
        tLastFix_					= inputFile[ "tLastFix" ];
        interpolation_				= AQLString( inputFile[ "interpolation" ]() );
        forecastCurve_				= AQLString( inputFile[ "forecastCurve" ]() );
        discountCurve_				= AQLString( inputFile[ "discountCurve" ]() );
        interpFwds_					= inputFile[ "interpFwds" ];

        // parRate Parameters: Optional
        tSpd_						= inputFile.getOptional( "tSpd",                         0.0 );
        useFwdData_                 = inputFile.getOptional( "useFwdData",                   false );
        isOIS_                      = inputFile.getOptional( "isOIS",                        false );
        oisCompoundingType_         = inputFile.getOptional( "oisCompoundingType",           AQLString() );
        calendar_                   = inputFile.getOptional( "calendar",                     AQLString() );
        rollConvention_             = inputFile.getOptional( "rollConvention",               AQLString() );
        slidingRule_                = inputFile.getOptional( "slidingRule",                  AQLString() );

        // swapPV Parameters: Mandatory ( inlcuded in the swapPV parameters but not included in parRate function parameters )
        payRec_						= inputFile.getOptional( "payRec",						AQLString( "PAYER" ) );
        notional_                   = inputFile.getOptional( "notional",                     1000000 );
        fixedRate_                  = inputFile.getOptional( "fixedRate",                    0.0 );

        // swapPV Parameters: Optional
        floatSpreadInBasisPoints_   = inputFile.getOptional( "floatSpreadInBasisPoints",     0.0 );
        compoundingMethod_          = inputFile.getOptional( "compoundingMethod",            AQLString() );
        floatCalendar_              = inputFile.getOptional( "floatCalendar",                AQLString() );
        floatRollConv_              = inputFile.getOptional( "floatRollConv",                AQLString() );

    }

    const double InterestRateSwap::parRate()
    {

        const double parRate = validation::tryMirGetParRate4(
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


        const double pv = validation::tryMirSwapPV(
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

        const double parPV = validation::tryMirSwapPV(
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
		const double pv01 = validation::tryMirSwapPV01( dataInstance_,
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
