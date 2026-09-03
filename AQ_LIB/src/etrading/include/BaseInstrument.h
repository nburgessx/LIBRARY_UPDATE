#pragma once

#include "LabelValueBlock.h"
#include "CommonConstants.h"
#include <boost/shared_array.hpp>

class AQLDate;

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
        virtual void setInterpolation( const AQLString& interpolation ) = 0;

        /* @brief		Method that returns trade ID
        *  @return		Trade ID
        */
        AQLString getTradeID() const
        {
            return tradeID_;
        }

    private:

        AQLString	tradeID_;
    };

    typedef boost::shared_ptr<BaseInstrument> BaseInstrumentPtr;

}
