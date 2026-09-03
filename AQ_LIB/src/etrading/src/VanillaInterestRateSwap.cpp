/*
 * @brief			Class that defines a vanilla interest rate swap instrument
 * @Created:		21 May 2016
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#include "VanillaInterestRateSwap.h"
#include "LADateScheduleHelpers.h"
#include "LACurveForwardRateHelpers.h"
#include "CommonConstants.h"
#include "LACoreComponentManager.h"
#include "CurveValidation.h"
#include "LabelValueBlockValidation.h"
#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"
#include "InitializeMLibETrading.h"
#include "ConstantDeclarations.h"
#include "LACurvePricingObject.h"
#include "CurveInstrumentPricing.h"
#include "LAPriceDataSlidingRule.h"

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
			LACurvePricingObject& yc = etrading::LACurveForwardRateHelpers::getYieldCurveForCurveID( etrading::InitializeMLibETrading::instance().dataInstance(), curveSet_ );

			LAString curveType = getCurveType( curveSet_, forecastCurve_ );
			if (etrading::LACurveForwardRateHelpers::setUpForwardDayCount( etrading::getDataInstance(), curveSet_, forecastCurve_, yc )
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

