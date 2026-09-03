#pragma once

#include "LabelValueBlock.h"
#include "BaseInstrument.h"
#include "DeltaRiskGenerator.h"

class AQLDate;

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
        virtual void setInterpolation( const AQLString& interpolation );

        /* @brief		Method that returns swap PV
        *  @return		instrument PV
        */
        virtual double pv() = 0;

    protected:

        // Common parameters
        double		notional_;
        AQLString	effectiveDate_;
        AQLString	maturityDate_;
        bool		isPayerSwap_;

        // Fixed leg parameters
        double		fixedRate_;
        AQLString    fixedBusinessDayAdjustment_;
        AQLString    fixedCalendar_;
        AQLString	fixedFrequency_;
        AQLString	fixedDayCount_;
        AQLString	fixedAccrualBusinessDayAdjustment_;
        AQLString	fixedAccrualCalendar_;
        AQLString	fixedPaymentBusinessDayAdjustment_;
        AQLString	fixedPaymentCalendar_;
        AQLString	fixedFirstStubDate_;
        AQLString	fixedLastStubDate_;
        AQLString	fixedRollDayString_;
        AQLString	fixedPaymentLag_;
        AQLString	fixedStubType_;

        // LA fixed leg parameters
        int         fixedLegRollDay_;
        AQLString    fixedLegRollConvention_;   // Options: IMM, EOM, Start, End or NULL
        bool        fixedLegIsEOMRoll_;
        bool        fixedLegIsStartRoll_;

        // Float leg parameters
        AQLString    floatBusinessDayAdjustment_;
        AQLString    floatCalendar_;
        AQLString	floatFrequency_;
        AQLString	floatDayCount_;
        AQLString	floatFixingBusinessDayAdjustment_;
        AQLString	floatFixingCalendar_;
        AQLString	floatAccrualBusinessDayAdjustment_;
        AQLString	floatAccrualCalendar_;
        AQLString	floatPaymentBusinessDayAdjustment_;
        AQLString	floatPaymentCalendar_;
        AQLString	floatFirstStubDate_;
        AQLString	floatLastStubDate_;
        AQLString	floatRollDayString_;
        AQLString	floatPaymentLag_;
        AQLString	floatFixingLag_;
        AQLString	floatStubType_;
        double		floatSpread_;
        double		floatFirstFixing_;
        double		floatLastFixing_;

        // LA float leg parameters
        int         floatLegRollDay_;
        AQLString    floatLegRollConvention_;   // Options: IMM, EOM, Start, End or NULL
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
        AQLString curveSet_;
        AQLString forecastCurve_;
        AQLString discountCurve_;
        AQLString interpolation_;

    };


}
