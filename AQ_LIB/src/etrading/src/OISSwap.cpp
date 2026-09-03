/*
 * @brief			Class that defines an overnight index swap instrument
 * @Created:		21 May 2016
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "OISSwap.h"
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
#include "LAPriceDataSlidingRule.h"
#include <boost/algorithm/string.hpp>

namespace etrading
{
    /* @brief			Constructor
    *  @param [in]		A string matrix that defines an interest swap
    */
    OISSwap::OISSwap( const LabelValueBlock& swapLVB ) : InterestRateSwap( swapLVB ), isFwdInterp_( false ), isYieldCurveReady_( false )
    {
        compoundingMethod_ = swapLVB.getCompulsoryValueAsLAString( IRS_KEY::COMPOUND_METHOD );
    }

    /* @brief		Method that returns swap PV
    *  @return		instrument PV
    */
    double OISSwap::pv()
    {
        LACurvePricingObject& yc = etrading::LACurveForwardRateHelpers::getYieldCurveForCurveID( etrading::InitializeMLibETrading::instance().dataInstance(), curveSet_ );

        LAString fixeddc = LACoreComponentManager::getDayCount( fixedDayCount_ );
        LAString floatdc = LACoreComponentManager::getDayCount( floatDayCount_ );

        // Initialise yield curve by reference only once
        if ( !isYieldCurveReady_ )
        {
            if ( interpolation_.size() == 0 )
            {
                throw LACoreInvalidData( "#Error: Interpolation has not been specified for PV", __FILE__, __LINE__ );
            }

            LAString inter =  LACoreComponentManager::getInterpolation( interpolation_ );
            yc.setInterpolation( inter );

            yc.getDayCount().setDayCount( floatdc ); // floatdc

            etrading::LACurveForwardRateHelpers::setCalendarForCurveID( yc, "" );

            yc.getSlidingRule().convertFromString( NO_CH );

            // Is forward interpolation possible?
            if ( etrading::LACurveForwardRateHelpers::setUpForwardDayCount( etrading::getDataInstance(), curveSet_, forecastCurve_, yc ) )
            {
                isFwdInterp_ = true;
            }

            isYieldCurveReady_ = true;
        }

        if ( fixedAccrualDates_.size() == 0
                || fixedPaymentDates_.size() == 0
                || floatFixingDates_.size() == 0
                || floatAccrualDates_.size() == 0
                || floatPaymentDates_.size() == 0 )
        {
            throw LACoreInvalidData( "#Error: Swap cash flow schedule has not been completely generated", __FILE__, __LINE__ );
        }

        LAString floatCalendar = floatCalendar_;
        if ( floatCalendar.size() == 0 )
        {
            floatCalendar = floatAccrualCalendar_;
            if ( floatCalendar.size() == 0 )
            {
                throw LACoreInvalidData( "#Error: Calendar is missing on the floating leg for compounding purpose", __FILE__, __LINE__ );
            }
        }

        LAString floatBusinessDayAdj = floatBusinessDayAdjustment_;
        if ( floatBusinessDayAdj.size() == 0 )
        {
            floatBusinessDayAdj = floatAccrualBusinessDayAdjustment_;
            if ( floatBusinessDayAdj.size() == 0 )
            {
                throw LACoreInvalidData( "#Error: Roll Convention (or Business Day Adjustment) is missing on the floating leg for compounding purpose", __FILE__, __LINE__ );
            }
        }

        LAString slidingRule( LAString( "NORMAL" ) );
        LAString compoundingMethod( compoundingMethod_ );
        bool eomRoll = false;
        if ( boost::iequals( floatRollDayString_.getCString(), "EOM" ) )
        {
            eomRoll = true;
        }
        etrading::validateOISParameters( slidingRule, compoundingMethod, eomRoll );

        double PV = yc.getSwapPV( isPayerSwap_,
                                  notional_,
                                  fixedRate_,
                                  floatSpread_,
                                  fixedAccrualDates_,
                                  fixedPaymentDates_,
                                  floatFixingDates_,
                                  floatAccrualDates_,
                                  floatPaymentDates_,
                                  useFloatFixing( floatFirstFixing_ ),
                                  floatFirstFixing_,
                                  useFloatFixing( floatLastFixing_ ),
                                  floatLastFixing_,
                                  fixeddc,
                                  floatdc,
                                  forecastCurve_,
                                  discountCurve_,
                                  false,			// isFWDInter. This input is obsolete to OIS pricing
                                  false,			// useFwdData. This input is obsolete to OIS pricing
                                  true,				// isOIS
                                  compoundingMethod,
                                  floatCalendar,
                                  interpolation_,
                                  floatBusinessDayAdj,
                                  slidingRule );		// slidingRule

        return PV;
    }



}

