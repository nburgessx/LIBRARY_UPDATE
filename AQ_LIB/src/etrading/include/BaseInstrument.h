/*
 * @brief			Class that serves as the base class of all trade classes
 * @Created:		11 April 2016
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include "LabelValueBlock.h"
#include "CommonConstants.h"
#include <boost/shared_array.hpp>

class LADate;

namespace etrading
{
    /* @brief		Pure virtual base class for all instruments
    */
    class BaseInstrument
    {
    public:

        BaseInstrument() {}
        virtual ~BaseInstrument() {}

        BaseInstrument( const LabelValueBlock& tradeLVB )
        {
            tradeID_ = tradeLVB.getCompulsoryValueAsLAString( IRS_KEY::TRADE_ID );
        }

        /* @brief		Method that returns instrument PV
        *  @return		instrument PV
        */
        virtual double pv() = 0;

        /* @brief		Method that sets market data
        *  @return		instrument PV
        */
        virtual void setMarketData( const LabelValueBlock& market ) = 0;

        /* @brief		Set interpolation for pricing
        *  @param [in]	interpolation	Yield curve interpolation
        */
        virtual void setInterpolation( const LAString& interpolation ) = 0;

        /* @brief		Method that returns trade ID
        *  @return		Trade ID
        */
        LAString getTradeID() const
        {
            return tradeID_;
        }

    private:

        LAString	tradeID_;
    };

    typedef boost::shared_ptr<BaseInstrument> BaseInstrumentPtr;

}
