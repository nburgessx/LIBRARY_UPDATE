#pragma once
#include "CoreEnumerations.h"
#include "BlackScholes.h"

// AAD library
#include <adept.h>

namespace etrading
{

	struct AnnuityTerm
	{
		double timeToPayment_; //yearFraction of the annuityFactor, i.e. yearFraction from valuation date to paymentDate
		double accrualTerm_;  // accrualTerm of the annuityFactor
		double discountTerm_; // DF of the annuityFactor
		double annuityFactor_; //derived from accrualTerm * discountTerm

		//Static Helper funtion to calculate the annuity factor
		double static annuityFactor(const double& accrualTerm, const double& discountTerm) { return accrualTerm * discountTerm; };

		//Zero rate from discount factor
		const double zeroRate() const
		{ 
			//DF = exp(-r*T) => r = (-1/T) * ln(DF)
			const double r = (-1.0 / timeToPayment_) * std::log(discountTerm_);
			return r;
		};

	};

	struct NumericalGreekBump
	{
		double deltaBump = 0.0;
		double gammaBump = 0.0;
		double vegaBump = 0.0;
		double thetaBump = 0.0;
		double rhoBump = 0.0;
	};

	namespace BlackModelUtils
	{

		// Calculate the Probability factors based on Normal Volatility
		ProbabilityFactors<double> normalBlackProbabilityFactors(const double& phi, const double& fwd, const double& strike, const double& vol, const double& time);

		// Calculate the Normal Price (normal volatility - supports negative rates)
		double normalBlackPrice(const CallOrPutEnum& callOrPut, double fwd, double strike, double vol, double time, double annuityFactor);

		// Calculate the Lognormal Price (Lognormal volatility)
		double lognormalBlackPrice(const CallOrPutEnum& callOrPut, double fwd, double strike, double vol, double time, double shift, double annuityFactor);

		// Calculate price based on volatility type
		double blackPrice(const VolatilityTypeEnum & volatilityType, const CallOrPutEnum& callOrPut, double fwd, double strike, double vol, double time, double shift, double annuityFactor);

		// Calculate the Normal Vol Analytical Greek
		BlackScholesGreeks normalBlackGreeksAnalytical(const CallOrPutEnum& callOrPut, const double& fwd, const double& strike, const double& vol, const double& time, const AnnuityTerm& annuityTerm, const bool& constantDF = false);

		// Calculate the Lognormal Vol Analytical Greek
		BlackScholesGreeks lognormalBlackGreeksAnalytical(const CallOrPutEnum& callOrPut, const double& fwd, const double& strike, const double& vol, const double& time, double shift, const AnnuityTerm& annuityTerm, const bool& constantDF = false);

		// Calculate Analytical Greek based on volatility type
		BlackScholesGreeks blackGreeksAnalytical(const VolatilityTypeEnum & volatilityType, const CallOrPutEnum& callOrPut, const double& fwd, const double& strike, const double& vol, const double& time, double shift, const AnnuityTerm& annuityTerm, const bool& constantDF = false);

		// Calculate the Numerical Greek based on volatility type
		BlackScholesGreeks blackGreeksNumerical(const VolatilityTypeEnum & volatilityType, const CallOrPutEnum& callOrPut, const double& fwd, const double& strike, const double& vol, const double& time, const double&  shift, const AnnuityTerm& annuityTerm, const NumericalGreekBump & greekBump, const bool& constantDF = false);

		// Calculate the Numerical Delta based on volatility type
		double blackNumericalDelta(const VolatilityTypeEnum & volatilityType, const CallOrPutEnum& callOrPut, const double& fwd, const double& strike, const double& vol, const double& time, const double&  shift, const AnnuityTerm& annuityTerm, const double& bump, const bool& constantDF = false);
		
		// Calculate the Numerical Gamma based on volatility type
		double blackNumericalGamma(const VolatilityTypeEnum & volatilityType, const CallOrPutEnum& callOrPut, const double& fwd, const double& strike, const double& vol, const double& time, const double&  shift, const AnnuityTerm& annuityTerm, const double& bump, const bool& constantDF = false);
		
		// Calculate the Numerical Vega based on volatility type
		double blackNumericalVega(const VolatilityTypeEnum & volatilityType, const CallOrPutEnum& callOrPut, const double& fwd, const double& strike, const double& vol, const double& time, const double&  shift, const AnnuityTerm& annuityTerm, const double& bump);
		
		// Calculate the Numerical Theta based on volatility type
		double blackNumericalTheta(const VolatilityTypeEnum & volatilityType, const CallOrPutEnum& callOrPut, const double& fwd, const double& strike, const double& vol, const double& time, const double&  shift, const AnnuityTerm& annuityTerm, const double& bump, const bool& constantDF = false);

		// Calculate the Numerical Rho based on volatility type
		double blackNumericalRho(const VolatilityTypeEnum & volatilityType, const CallOrPutEnum& callOrPut, const double& fwd, const double& strike, const double& vol, const double& time, const double&  shift, const AnnuityTerm& annuityTerm, const double& bump);

    };

	
}
