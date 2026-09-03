/*
 * @brief			Class that carries multiple trades
 * @Created:		21 May 2016
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include "LabelValueBlock.h"
#include "BaseInstrument.h"
#include "DeltaRiskGenerator.h"

class LADate;

namespace etrading
{
    /* @brief		Interest rate swap instrument
    */
    class TradeCollection : public BaseInstrument
    {
    public:

        TradeCollection();

        /* @brief		Constructor
        */
        TradeCollection( const std::vector<BaseInstrumentPtr>& portfolio );

        /* @brief		Copy Constructor
        */
        TradeCollection( const TradeCollection& rhs );

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
        virtual double pv();

    private:

        // Trades
        std::vector<BaseInstrumentPtr> trades_;

        // Market
        LAString curveSet_;
        LAString forecastCurve_;
        LAString discountCurve_;
        LAString interpolation_;

        // Delta ladder generators, one for each trade
        DeltaGenerator riskGen_;

    };


}
