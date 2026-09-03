#pragma once

#include "LabelValueBlock.h"
#include "BaseInstrument.h"
#include "DeltaRiskGenerator.h"

class LADate;

namespace etrading
{

    /* @brief		Interest rate swap instrument
    				Pure virtual class.
    				Must be derived from concrete classes of certain business type.
    */
    class InterestRateSwap : public BaseInstrument
    {
    public:

        InterestRateSwap( const LabelValueBlock& swapLVB );

        virtual ~InterestRateSwap() {}

        /* @brief		Method that sets market data
        *  @param [in]	market	The market data IDs used for pricing
        */
        virtual void setMarketData( const LabelValueBlock& marketDataLVB );

        /* @brief		Set interpolation for pricing
        *  @param [in]	interpolation	Yield curve interpolation
        */
        virtual void setInterpolation( const LAString& interpolation );

        /* @brief		Method that returns swap PV
        *  @return		instrument PV
        */
        virtual double pv() = 0;

    protected:

        // Common parameters
        double		notional_;
        LAString	effectiveDate_;
        LAString	maturityDate_;
        bool		isPayerSwap_;

        // Fixed leg parameters
        double		fixedRate_;
        LAString    fixedBusinessDayAdjustment_;
        LAString    fixedCalendar_;
        LAString	fixedFrequency_;
        LAString	fixedDayCount_;
        LAString	fixedAccrualBusinessDayAdjustment_;
        LAString	fixedAccrualCalendar_;
        LAString	fixedPaymentBusinessDayAdjustment_;
        LAString	fixedPaymentCalendar_;
        LAString	fixedFirstStubDate_;
        LAString	fixedLastStubDate_;
        LAString	fixedRollDayString_;
        LAString	fixedPaymentLag_;
        LAString	fixedStubType_;

        // LA fixed leg parameters
        int         fixedLegRollDay_;
        LAString    fixedLegRollConvention_;   // Options: IMM, EOM, Start, End or NULL
        bool        fixedLegIsEOMRoll_;
        bool        fixedLegIsStartRoll_;

        // Float leg parameters
        LAString    floatBusinessDayAdjustment_;
        LAString    floatCalendar_;
        LAString	floatFrequency_;
        LAString	floatDayCount_;
        LAString	floatFixingBusinessDayAdjustment_;
        LAString	floatFixingCalendar_;
        LAString	floatAccrualBusinessDayAdjustment_;
        LAString	floatAccrualCalendar_;
        LAString	floatPaymentBusinessDayAdjustment_;
        LAString	floatPaymentCalendar_;
        LAString	floatFirstStubDate_;
        LAString	floatLastStubDate_;
        LAString	floatRollDayString_;
        LAString	floatPaymentLag_;
        LAString	floatFixingLag_;
        LAString	floatStubType_;
        double		floatSpread_;
        double		floatFirstFixing_;
        double		floatLastFixing_;

        // LA float leg parameters
        int         floatLegRollDay_;
        LAString    floatLegRollConvention_;   // Options: IMM, EOM, Start, End or NULL
        bool        floatLegIsEOMRoll_;
        bool        floatLegIsStartRoll_;

        // Date schedules
        DateVector fixedAccrualDates_;
        DateVector fixedAccrualStartDates_;
        DateVector fixedAccrualEndDates_;
        DateVector fixedPaymentDates_;
        DateVector floatAccrualDates_;
        DateVector floatAccrualStartDates_;
        DateVector floatAccrualEndDates_;
        DateVector floatFixingDates_;
        DateVector floatPaymentDates_;

        // Market
        LAString curveSet_;
        LAString forecastCurve_;
        LAString discountCurve_;
        LAString interpolation_;

    };


}
