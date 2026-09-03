#pragma once

#include "Option.h"
#include "SwaptionTrade.h"
#include "Volatility.h"
#include "RateProvider.h"
#include "OptionCashflow.h"
#include "ScheduleValidation.h"
#include "OptionCashflow.h"
#include "BlackScholes.h"
#include "ObjectUtilities.h"

namespace etrading
{
    class SwaptionPricer 
    {
    public:
		
        // Constructors
		SwaptionPricer( const std::shared_ptr<SwaptionTrade>& swaption, const AQLStringMatrix& valuationSettingsLVB );
        SwaptionPricer( const SwaptionPricer& rhs );
		virtual ~SwaptionPricer() {}

        // Calculations
        const double pv() const;
        const double impliedVol( const double & price ) const;

        const double delta() const;
        const double gamma() const;
        const double vega() const;
        const double theta() const;

	private:
        
        // Struct to Hold Black-Scholes Parameters
        struct BlackScholesParameters
        {
            PayerReceiverSwaptionEnum payerReceiver_;
            double annuityWithNotional_;
            double parRate_;
            double strike_;
            double vol_;
            double expiryYearFraction_;
            double shiftSize_;
            VolatilityTypeEnum volatilityType_;

            // Other Helpful Parameters
            int longShortIndicator_;
            double leverage_;
        };

		// Helper methods
		BlackScholesParameters calcualateBlackScholesParameters() const;
        double calculateAnnuityWithNotional( const SwapPtr& underlyingSwap, const double parRate ) const;
        double discountFactor( const AQLDate& valuationDate ) const;
        double calculateFeePV() const;
        double calculateFeeDelta() const;
        double calculateFeeGamma() const;
        double calculateFeeTheta() const;
        double calculateAnnuityDelta( const BlackScholesParameters & bs ) const;
        double calculateAnnuityGamma( const BlackScholesParameters & bs ) const;
        double calculateAnnuityTheta( const BlackScholesParameters & bs ) const;

		std::shared_ptr<SwaptionTrade> swaptionTrade_;
        std::string curveCollection_;
        std::shared_ptr<Volatility> volProvider_;
		AQLStringMatrix valuationSettingsLVB_;

	};

}
