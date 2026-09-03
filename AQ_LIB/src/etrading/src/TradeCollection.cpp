#include "TradeCollection.h"
#include "CommonConstants.h"

namespace etrading
{
    /* @brief		Default constructor
    */
    TradeCollection::TradeCollection()
    {
        trades_.clear();
    }

    /* @brief		Constructor
    */
    TradeCollection::TradeCollection( const std::vector<BaseInstrumentPtr>& portfolio )
    {
        trades_ = portfolio;
    }

    /* @brief		Copy Constructor
    */
    TradeCollection::TradeCollection( const TradeCollection& rhs )
    {
        trades_			= rhs.trades_;
        curveSet_		= rhs.curveSet_;
        forecastCurve_	= rhs.forecastCurve_;
        discountCurve_	= rhs.discountCurve_;
        interpolation_	= rhs.interpolation_;
        riskGen_		= rhs.riskGen_;
    }

    /* @brief		Method that sets market data
    */
    void TradeCollection::setMarketData( const LabelValueBlock& marketDataLVB )
    {
        curveSet_		= marketDataLVB.getCompulsoryValueAsLAString( MARKET_KEY::CURVE_COLLECTION, "CurveCollections" );
        forecastCurve_	= marketDataLVB.getCompulsoryValueAsLAString( MARKET_KEY::FORECAST_CURVE, "CurveCollections" );
        discountCurve_	= marketDataLVB.getCompulsoryValueAsLAString( MARKET_KEY::DISCOUNT_CURVE, "CurveCollections" );

        if ( forecastCurve_.size() == 0 )
        {
            throw LACoreInvalidData( "#Error: Forecast curve has not been specified for PV", __FILE__, __LINE__ );
        }

        if ( discountCurve_.size() == 0 )
        {
            throw LACoreInvalidData( "#Error: Discount curve has not been specified for PV", __FILE__, __LINE__ );
        }
    }

    /* @brief		Set interpolation for pricing
    *  @param [in]	interpolation	Yield curve interpolation
    */
    void TradeCollection::setInterpolation( const LAString& interpolation )
    {
        interpolation_ = interpolation;
        interpolation_.toUpper();
        if ( interpolation_.size() == 0 )
        {
            interpolation_ = "SPLINE";
        }
    }

    /* @brief		Method that returns swap PV
        *  @return		instrument PV
        */
    double TradeCollection::pv()
    {
        double ret( 0.0 );
        for ( size_t i = 0; i < trades_.size(); ++i )
        {
            ret += trades_[i]->pv();
        }

        return ret;
    }

}

