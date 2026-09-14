#include "OISSwap.h"
#include "AQLDateScheduleHelpers.h"
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
#include "AQLPriceDataSlidingRule.h"
#include <boost/algorithm/string.hpp>

namespace etrading
{
    /* @brief			Constructor
    *  @param [in]		A string matrix that defines an interest swap
    */
    OISSwap::OISSwap( const LabelValueBlock& swapLVB ) : InterestRateSwap( swapLVB ), isFwdInterp_( false ), isYieldCurveReady_( false )
    {
        compoundingMethod_ = swapLVB.getCompulsoryValueAsAQLString( IRS_KEY::COMPOUND_METHOD );
    }

    /* @brief		Method that returns swap PV
    *  @return		instrument PV
    */
    double OISSwap::pv()
    {
        AQLCurvePricingObject& yc = etrading::AQLCurveForwardRateHelpers::getYieldCurveForCurveID( etrading::InitializeETrading::instance().dataInstance(), curveSet_ );

        AQLString fixeddc = AQLCoreComponentManager::getDayCount( fixedDayCount_ );
        AQLString floatdc = AQLCoreComponentManager::getDayCount( floatDayCount_ );

        // Initialise yield curve by reference only once
        if ( !isYieldCurveReady_ )
        {
            AQ_THROW_IF( interpolation_.size() == 0, "Interpolation has not been specified for PV" );

            AQLString inter =  AQLCoreComponentManager::getInterpolation( interpolation_ );
            yc.setInterpolation( inter );

            yc.getDayCount().setDayCount( floatdc ); // floatdc

            etrading::AQLCurveForwardRateHelpers::setCalendarForCurveID( yc, "" );

            yc.getSlidingRule().convertFromString( NO_CH );

            // Is forward interpolation possible?
            if ( etrading::AQLCurveForwardRateHelpers::setUpForwardDayCount( etrading::getDataInstance(), curveSet_, forecastCurve_, yc ) )
            {
                isFwdInterp_ = true;
            }

            isYieldCurveReady_ = true;
        }

        AQ_THROW_IF( fixedAccrualDates_.size() == 0
                || fixedPaymentDates_.size() == 0
                || floatFixingDates_.size() == 0
                || floatAccrualDates_.size() == 0
                || floatPaymentDates_.size() == 0, "Swap cash flow schedule has not been completely generated" );

        AQLString floatCalendar = floatCalendar_;
        if ( floatCalendar.size() == 0 )
        {
            floatCalendar = floatAccrualCalendar_;
            AQ_THROW_IF( floatCalendar.size() == 0, "Calendar is missing on the floating leg for compounding purpose" );
        }

        AQLString floatBusinessDayAdj = floatBusinessDayAdjustment_;
        if ( floatBusinessDayAdj.size() == 0 )
        {
            floatBusinessDayAdj = floatAccrualBusinessDayAdjustment_;
            AQ_THROW_IF( floatBusinessDayAdj.size() == 0, "Roll Convention (or Business Day Adjustment) is missing on the floating leg for compounding purpose" );
        }

        AQLString slidingRule( AQLString( "NORMAL" ) );
        AQLString compoundingMethod( compoundingMethod_ );
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

