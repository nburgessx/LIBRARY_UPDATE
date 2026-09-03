
/*
 * @brief			Generalized Black-Scholes
 * @Created:		11th August 2017
 * @Author:			Nicholas Burgess
 * @Department:	    Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#include "BlackUtils.h"
#include "NormalDistribution.h"
#include "ExceptionMacros.h"
#include <cmath>
#include <algorithm>
#include <functional>

namespace etrading
{
	namespace BlackModelUtils
	{

		// Calculate the Probability factors based on Normal Volatility
		ProbabilityFactors<double> normalBlackProbabilityFactors(const double& phi, const double& fwd, const double& strike, const double& vol, const double& time)
		{
			// Validation
			MLIB_REQUIRE(MLIB_IS_GREATER_THAN_OR_EQUAL_TO_ZERO(time), "Option time to expiry must be greater than zero");

			double volSqrtTime = vol * std::sqrt(time);

			if (MLIB_IS_EQUAL_ZERO(volSqrtTime)) volSqrtTime = MLIB_EPSILON;    // Don't allow divide by zero

			const double d1 = (fwd - strike) / volSqrtTime;

			ProbabilityFactors<double> probabilityFactors;

			probabilityFactors.d1 = d1;
			probabilityFactors.Nd1 = standardNormalDistribution(phi * d1);              // Standard Normal CDF i.e. N(phi.d1)
			probabilityFactors.nd1 = standardNormalDistributionPDF(d1);                 // Standard Normal PDF i.e. n(d1) with no phi term

			return probabilityFactors;
		}


		// Calculate the Normal Price (normal volatility - supports negative rates)
		double normalBlackPrice(const CallOrPutEnum& callOrPut, double fwd, double strike, double vol, double time, double annuityFactor)
		{

			// Boundary Conditions
			// -----------------------------------

			// Manage the Option zero value Boundaries by adding a small precision epsilon value to the underlying parameter
			if (MLIB_IS_EQUAL_ZERO(fwd))       fwd += MLIB_EPSILON;
			if (MLIB_IS_EQUAL_ZERO(strike))     strike += MLIB_EPSILON;
			if (MLIB_IS_EQUAL_ZERO(vol))        vol += MLIB_EPSILON;
			if (MLIB_IS_EQUAL_ZERO(time))       time += MLIB_EPSILON;

			// Calculation Parameters
			// ------------------------------------
			double phi = (callOrPut == CALL_OPTION) ? 1.0 : -1.0;

			ProbabilityFactors<double> probabilityFactors = normalBlackProbabilityFactors(phi, fwd, strike, vol, time);

			const double volSqrtTime = vol * std::sqrt(time);

			// Normal Black Formulae
			// ------------------------------------
			double price = ((phi * (fwd - strike) * probabilityFactors.Nd1) + (volSqrtTime * probabilityFactors.nd1));

			price *= annuityFactor;

			return price;
		}

		// Calculate the Lognormal Price (Lognormal volatility)
		double lognormalBlackPrice(const CallOrPutEnum& callOrPut, double fwd, double strike, double vol, double time, double shift, double annuityFactor)
		{

			// Use Black-76 Model, Carry = 0.0; 
			double bkPrice = BlackScholes::price(callOrPut,
												fwd,   // Spot = SwapRate 
												strike,
												vol,
												time,
												0.0,        // Zero Rate = 0.0 - We are discounting using the Annuity measure not the risk neutral savings account
												0.0,        // Black-76 Carry = 0.0
												shift);

			bkPrice *= annuityFactor;

			return bkPrice;
		}

		// Calculate price based on volatility type
		double blackPrice(const VolatilityTypeEnum & volatilityType, const CallOrPutEnum& callOrPut, double fwd, double strike, double vol, double time, double shift, double annuityFactor)
		{
			double price = 0.0;

			switch (volatilityType)
			{
			case LOGNORMAL_VOLATILITY:
            case SHIFTED_LOGNORMAL_VOLATILITY:

				// Volatility in Percent
				price = lognormalBlackPrice(callOrPut, fwd, strike, vol, time, shift, annuityFactor);
				break;

			case NORMAL_VOLATILITY:

				// Volatility in Percent
				price = normalBlackPrice(callOrPut, fwd, strike, vol, time, annuityFactor); // Note: No Shift term
				break;

			default:
				// Should never reach here
				MLIB_THROW("Invalid volatility type. Must be 'LOGNORMAL', 'SHIFTED_LOGNORMAL' or 'NORMAL'");
				break;
			}

			return price;
		}

		//----Delta, Gamma, Theta need to include risk due to discount zero rate (OIS) change -- //
		void addZeroRateTermToDeltaGammaTheta(BlackScholesGreeks& greeks, const AnnuityTerm& annuityTerm, const double& futureUnitPrice)
		{
			// deltaZeroRateTerm (same as rho): dV/dr = -T * V, where V is the option price
			const double deltaZeroRateTerm = (-1.0) * annuityTerm.timeToPayment_ * futureUnitPrice;

			//Save the delta respect to fwd
			const double deltaFwd = greeks.deltaSpot;

			//Update delta to include both zeroRate(Ois) term
			greeks.deltaSpot += deltaZeroRateTerm;

			//--Gamma Calculation --//
			//gammaFwdTerm: dDelta/dF = d(dV/dF + dV/dr)/dF = gammaF + d(-T*V)/dF = gammaF + (-T) * deltaFwd, 
			const double gammDeltaZeroRespectToFwd = (-1.0) * annuityTerm.timeToPayment_ * deltaFwd;
			greeks.gamma += gammDeltaZeroRespectToFwd;

			//gammaZeroRateTerm: dDelta/dr = d(dV/dF + dV/dr)/dr = d(deltaFwd)/dr + T^2 * V  
			const double gammaZeroRateTerm = (-1.0) * annuityTerm.timeToPayment_ * deltaFwd + annuityTerm.timeToPayment_ * annuityTerm.timeToPayment_ * futureUnitPrice;

			//gammaTotal = gammaFwdTerm + gammaZeroRateTerm
			greeks.gamma += gammaZeroRateTerm;

			// (-dV/dT)zeroRateTerm = r * V, where V is the option price
			const double thetaZeroRateTerm = annuityTerm.zeroRate() * futureUnitPrice;
			greeks.theta += thetaZeroRateTerm;
		}

		void discountAndScaleGreeks(BlackScholesGreeks& greeks, const AnnuityTerm& annuityTerm, const VolatilityTypeEnum & volatilityType)
		{
			//----Discount Greek -- //
			const double annuityFactor = annuityTerm.annuityFactor_;
			greeks.deltaSpot *= annuityFactor;
			greeks.gamma *= annuityFactor;
			greeks.vega *= annuityFactor;
			greeks.theta *= annuityFactor;
			greeks.rho *= annuityFactor;

			//----Scale Greek -- //
			const double bpFactor = 0.0001;
			// If it is Lognormal Volatility, Scale to 1% Shift, i.e. by 1/100 
			// If it is Normal Volatility, Scale to 1bps Shift, i.e. by 1/10000
			const double vegaScaleFactor = (volatilityType == LOGNORMAL_VOLATILITY) ? 0.01 : 0.0001;

			// Scale to 1bps Shift, i.e. by 1/10000 
			greeks.deltaSpot *= bpFactor;

			// Opposite sign to match the numerical delta risk (downBumpPrice - upBumpPrice)/(2*bump)
			greeks.deltaSpot *= -1.0;

			// Scale to 1bps Shift, i.e. by 1/(10000^2)
			greeks.gamma *= bpFactor * bpFactor;

			greeks.vega *= vegaScaleFactor;

			// Theta: Scale to 1-day Shift, i.e. by 1/365
			greeks.theta *= 1.0 / 365.0;

		}
	

		// Calculate the Normal Vol Analytical Greek
		BlackScholesGreeks normalBlackGreeksAnalytical(const CallOrPutEnum& callOrPut, const double& fwd, const double& strike, const double& vol, const double& time, const AnnuityTerm& annuityTerm, const bool& constantDF)
		{
			BlackScholesGreeks greeks;

			const double phi = (callOrPut == CALL_OPTION) ? 1.0 : -1.0;

			ProbabilityFactors<double> probabilityFactors = normalBlackProbabilityFactors(phi, fwd, strike, vol, time);

			const double sqrtTime = sqrt(time);

			// N(d1)
			greeks.deltaSpot = phi * probabilityFactors.Nd1;

			// Gamma - note nd1 here is the PDF not the CDF
			greeks.gamma = probabilityFactors.nd1 / (vol * sqrtTime);

			// Vega
			greeks.vega = probabilityFactors.nd1 * sqrtTime;

			// Note nd1 is the PDF and ND1 the CDF
			// *** Note:Theta's full formula is as below, but market/BB normally does not include  the second term.
			//greeks.theta = -(probabilityFactors.nd1 * vol) / (2.0 * sqrtTime) - (annuityTerm.zeroRate() * (phi * (fwd - strike) * probabilityFactors.Nd1 + vol * sqrtTime * probabilityFactors.nd1));
			greeks.theta = -vol / (2.0 * sqrtTime) * probabilityFactors.nd1;

			//----Delta, Gamma, Theta need to include risk due to discount zero rate (OIS) change -- //
			if (!constantDF)
			{
				const double futureUnitPrice = normalBlackPrice(callOrPut, fwd, strike, vol, time, 1.0);

				addZeroRateTermToDeltaGammaTheta(greeks, annuityTerm, futureUnitPrice);
			}

			discountAndScaleGreeks(greeks, annuityTerm, NORMAL_VOLATILITY);

			return greeks;
		}

	
		// Calculate the Lognormal Vol Analytical Greek
		BlackScholesGreeks lognormalBlackGreeksAnalytical(const CallOrPutEnum& callOrPut, const double& fwd, const double& strike, const double& vol, const double& time, double shift, const AnnuityTerm& annuityTerm, const bool& constantDF)
		{
																																		 
			BlackScholesGreeks greeks = BlackScholes::calculatePriceAndGreeks(callOrPut, fwd, strike, vol, time, annuityTerm.timeToPayment_ , 0.0, 0.0, shift); // discountRate=0.0, carry=0.0

			greeks.theta *= 365.25; // theta is scaled by 1/365.25 in the BlackScholes function, we unscale here as we will scale in a cetral function next
			greeks.vega *= 100.0; // vega is scaled by 1/100 in the BlackScholes function, we unscale here as we will scale in a cetral function next
			
			//price with annuityFactor 1
			const double futureUnitPrice = lognormalBlackPrice(callOrPut, fwd, strike, vol, time, shift, 1.0);

			// *** Black's rho is different from BS's rho ***
			greeks.rho = (-1.0) * annuityTerm.timeToPayment_ * futureUnitPrice ;

 		   //----Delta, Gamma, Theta need to include risk due to discount zero rate (OIS) change -- //
			if (!constantDF)
			{
				addZeroRateTermToDeltaGammaTheta(greeks, annuityTerm, futureUnitPrice);
			}

			discountAndScaleGreeks(greeks, annuityTerm, LOGNORMAL_VOLATILITY);

			return greeks;
		}

		// Calculate Analytical Greek based on volatility type
		BlackScholesGreeks blackGreeksAnalytical(const VolatilityTypeEnum & volatilityType, const CallOrPutEnum& callOrPut, const double& fwd, const double& strike, const double& vol, const double& time, double shift, const AnnuityTerm& annuityTerm, const bool& constantDF)
		{
			BlackScholesGreeks greeks;

			switch (volatilityType)
			{
				case LOGNORMAL_VOLATILITY:
					greeks = lognormalBlackGreeksAnalytical(callOrPut, fwd, strike, vol, time, shift, annuityTerm, constantDF);
					break;
				case NORMAL_VOLATILITY:
					greeks = normalBlackGreeksAnalytical(callOrPut, fwd, strike, vol, time, annuityTerm, constantDF);
					break;
				default:
					throw LACoreInvalidData("#Error: Only Lognormal or Normal Vol is supported", __FILE__, __LINE__);
					break;
			}
			return greeks;

		}

		// Calculate the Numerical Greek based on volatility type
		BlackScholesGreeks blackGreeksNumerical(const VolatilityTypeEnum & volatilityType, const CallOrPutEnum& callOrPut, const double& fwd, const double& strike, const double& vol, const double& time, const double& shift, const AnnuityTerm& annuityTerm, const NumericalGreekBump & greekBump, const bool& constantDF)
		{
			BlackScholesGreeks greeks;

			greeks.deltaSpot = blackNumericalDelta(volatilityType, callOrPut, fwd, strike, vol, time, shift, annuityTerm, greekBump.deltaBump, constantDF);
			greeks.gamma = blackNumericalGamma(volatilityType, callOrPut, fwd, strike, vol, time, shift, annuityTerm, greekBump.gammaBump, constantDF);
			greeks.vega = blackNumericalVega(volatilityType, callOrPut, fwd, strike, vol, time, shift, annuityTerm, greekBump.vegaBump);
			greeks.theta = blackNumericalTheta(volatilityType, callOrPut, fwd, strike, vol, time, shift, annuityTerm, greekBump.thetaBump, constantDF);
			greeks.rho = blackNumericalRho(volatilityType, callOrPut, fwd, strike, vol, time, shift, annuityTerm, greekBump.rhoBump);

			return greeks;
		}

		// Calculate the Numerical Delta based on volatility type
		double blackNumericalDelta(const VolatilityTypeEnum & volatilityType, const CallOrPutEnum& callOrPut, const double& fwd, const double& strike, const double& vol, const double& time, const double&  shift, const AnnuityTerm& annuityTerm, const double& bump, const bool& constantDF)
		{

			if (MLIB_IS_EQUAL_ZERO(bump))
			{
				return 0.0;
			}

			const double priceDownBumpAnnuityFactor = constantDF ? annuityTerm.annuityFactor_ : AnnuityTerm::annuityFactor(annuityTerm.accrualTerm_, annuityTerm.discountTerm_ * exp(bump * annuityTerm.timeToPayment_));
			const double priceDownBump = blackPrice(volatilityType, callOrPut, fwd - bump, strike, vol, time, shift, priceDownBumpAnnuityFactor);

			const double priceUpBumpAnnuityFactor = constantDF ? annuityTerm.annuityFactor_ : AnnuityTerm::annuityFactor(annuityTerm.accrualTerm_, annuityTerm.discountTerm_ * exp(-bump * annuityTerm.timeToPayment_));
			const double priceUpBump = blackPrice(volatilityType, callOrPut, fwd + bump, strike, vol, time, shift, priceUpBumpAnnuityFactor);

			double deltaNumerical = (priceDownBump - priceUpBump) / (2*bump);

			// Scale to 1bps Shift, i.e. by 1/10000
			const double bpsFactor = 0.0001;
			deltaNumerical *= bpsFactor;

			return deltaNumerical;
		}

		// Calculate the Numerical Gamma based on volatility type
		double blackNumericalGamma(const VolatilityTypeEnum & volatilityType, const CallOrPutEnum& callOrPut, const double& fwd, const double& strike, const double& vol, const double& time, const double&  shift, const AnnuityTerm& annuityTerm, const double& bump, const bool& constantDF)
		{

			if (MLIB_IS_EQUAL_ZERO(bump))
			{
				return 0.0;
			}

			const double priceNoBump = blackPrice(volatilityType, callOrPut, fwd, strike, vol, time, shift, annuityTerm.annuityFactor_);

			const double priceUpBumpAnnuityFactor = constantDF ? annuityTerm.annuityFactor_ : AnnuityTerm::annuityFactor(annuityTerm.accrualTerm_, annuityTerm.discountTerm_ * exp(-bump * annuityTerm.timeToPayment_));
			const double priceUpBump = blackPrice(volatilityType, callOrPut, fwd + bump, strike, vol, time, shift, priceUpBumpAnnuityFactor);
			
			const double priceDownBumpAnnuityFactor = constantDF ? annuityTerm.annuityFactor_ : AnnuityTerm::annuityFactor(annuityTerm.accrualTerm_, annuityTerm.discountTerm_ * exp(bump * annuityTerm.timeToPayment_));
			const double priceDownBump = blackPrice(volatilityType, callOrPut, fwd - bump, strike, vol, time, shift, priceDownBumpAnnuityFactor);
			
			double gammaNumerical = (priceUpBump - 2 * priceNoBump + priceDownBump) / (bump * bump);

			// Scale to 1bps Shift, i.e. by 1/(10000^2)
			const double bpsFactor = 0.0001;
			gammaNumerical *= bpsFactor * bpsFactor;

			return gammaNumerical;
		}

		// Calculate the Numerical Vega based on volatility type
		double blackNumericalVega(const VolatilityTypeEnum & volatilityType, const CallOrPutEnum& callOrPut, const double& fwd, const double& strike, const double& vol, const double& time, const double&  shift, const AnnuityTerm& annuityTerm, const double& bump)
		{
			if (MLIB_IS_EQUAL_ZERO(bump))
			{
				return 0.0;
			}

			const double priceNoBump = blackPrice(volatilityType, callOrPut, fwd, strike, vol, time, shift, annuityTerm.annuityFactor_);
			const double priceUpBump = blackPrice(volatilityType, callOrPut, fwd, strike, vol + bump, time, shift, annuityTerm.annuityFactor_);

			double vegaNumerical = (priceUpBump - priceNoBump) / bump;

			// If it is Lognormal Volatility, Scale to 1% Shift, i.e. by 1/100 
			// If it is Normal Volatility, Scale to 1bps Shift, i.e. by 1/10000
			const double scaleFactor = (volatilityType == LOGNORMAL_VOLATILITY) ? 0.01 : 0.0001;
			vegaNumerical *= scaleFactor;

			return vegaNumerical;
		}

		// Calculate the Numerical Theta based on volatility type
		double blackNumericalTheta(const VolatilityTypeEnum & volatilityType, const CallOrPutEnum& callOrPut, const double& fwd, const double& strike, const double& vol, const double& time, const double&  shift, const AnnuityTerm& annuityTerm, const double& bump, const bool& constantDF)
		{
			if (MLIB_IS_EQUAL_ZERO(bump))
			{
				return 0.0;
			}

			const double priceNoBump = blackPrice(volatilityType, callOrPut, fwd, strike, vol, time, shift, annuityTerm.annuityFactor_);
			
			double priceDownBumpAnnuityFactor = annuityTerm.annuityFactor_;
			
			if (!constantDF)
			{
				//DF = exp(-r*T) => r = -1/T * ln(DF)
				const double r = annuityTerm.zeroRate();

				//Assuming zeroRate r is not changing from T to T-bump, we have DF' = exp(-r*(T-bump)) => DF' = DF * exp(-r*-bump) 
				//TODO: we should get the newDF out of the curve based on the new valuationDate (originalValutionDate + 1 day), and get the price
				priceDownBumpAnnuityFactor = AnnuityTerm::annuityFactor(annuityTerm.accrualTerm_, annuityTerm.discountTerm_ * exp(r * bump));
			}

			const double priceDownBump = blackPrice(volatilityType, callOrPut, fwd, strike, vol, time - bump, shift, priceDownBumpAnnuityFactor);

			const double thetaNumerical = priceDownBump - priceNoBump;
			
			return thetaNumerical;
		}

		double blackNumericalRho(const VolatilityTypeEnum & volatilityType, const CallOrPutEnum& callOrPut, const double& fwd, const double& strike, const double& vol, const double& time, const double&  shift, const AnnuityTerm& annuityTerm, const double& bump)
		{
			if (MLIB_IS_EQUAL_ZERO(bump))
			{
				return 0.0;
			}

			const double priceNoBump = blackPrice(volatilityType, callOrPut, fwd, strike, vol, time, shift, annuityTerm.annuityFactor_);

			// DF = exp(-r*T)
			// newDF = exp(-(r + bump)* T) = exp(-r*T) * exp(-bump*T)
			const double priceUpBumpAnnuityFactor = AnnuityTerm::annuityFactor(annuityTerm.accrualTerm_, annuityTerm.discountTerm_ * exp(-bump * annuityTerm.timeToPayment_));

			const double priceUpBump = blackPrice(volatilityType, callOrPut, fwd, strike, vol, time, shift, priceUpBumpAnnuityFactor);

			const double rhoNumerical = (priceUpBump - priceNoBump) / bump;

			return rhoNumerical;
		}

	}


  
}