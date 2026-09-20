#include "VanillaInterestRateSwap.h"
#include "AQLDateSchedule.h"
#include "AQLCurveForwardRateHelpers.h"
#include "CommonConstants.h"
#include "AQLCoreComponentManager.h"
#include "CurveValidation.h"
#include "LabelValueBlockValidation.h"
#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"
#include "InitializeETrading.h"
#include "ConstantDeclarations.h"
#include "AQLCurvePricingObject.h"
#include "CurveInstrumentPricing.h"
#include "AQLPriceDataSlidingRule.h"

namespace etrading
{
    /* @brief			Constructor
    *  @param [in]		A string matrix that defines an interest swap
    */
    VanillaInterestRateSwap::VanillaInterestRateSwap( const LabelValueBlock& swapLVB ) : InterestRateSwap( swapLVB ), isFwdInterp_( false ), isYieldCurveReady_( false )
    {}

    /* @brief		Method that returns swap PV
    *  @return		instrument PV
    */
    double VanillaInterestRateSwap::pv()
    {
		if ( !isYieldCurveReady_ )
		{
			// Is forward interpolation possible?
			AQLCurvePricingObject& yc = etrading::AQLCurveForwardRateHelpers::getYieldCurveForCurveID( etrading::InitializeETrading::instance().dataInstance(), curveSet_ );

			AQLString curveType = getCurveType( curveSet_, forecastCurve_ );
			if (etrading::AQLCurveForwardRateHelpers::setUpForwardDayCount( etrading::getDataInstance(), curveSet_, forecastCurve_, yc )
					&& curveType.getCString() != MARKET_KEY::CURVE_TYPE_BASIS
					&& curveType.getCString() != MARKET_KEY::CURVE_TYPE_TENORBASIS
					&& curveType.getCString() != MARKET_KEY::CURVE_TYPE_XCCYBASIS )
			{
				isFwdInterp_ = true;
			}
		}

		double PV = CurveInstrumentPricing::getSwapPV( isPayerSwap_,
                     notional_,
                     fixedAccrualDates_,
                    fixedPaymentDates_,
                    floatFixingDates_,
                    floatAccrualDates_,
                    floatPaymentDates_,
                     etrading::getDataInstance(),
                     curveSet_,
                     fixedRate_,
                     fixedDayCount_,
                     floatSpread_,
                     floatDayCount_,
                     interpolation_,
                     forecastCurve_,
                     discountCurve_,
                     isFwdInterp_,
                     useFloatFixing( floatFirstFixing_ ),
                     floatFirstFixing_,
                     useFloatFixing( floatLastFixing_ ),
                     floatLastFixing_,
                     isFwdInterp_ );

        return PV;
    }



}

