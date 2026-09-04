#include "SupervisoryRules.h"
#include "ExceptionMacros.h"

#include <boost/math/distributions/beta.hpp>
#include <cmath>


namespace etrading
{
	/* @brief For details of the AQLRule Supervisory Formula calculation below please see these URLs:
	*  https://eur-lex.europa.eu/legal-content/EN/TXT/PDF/?uri=CELEX:32013R0575&from=EN
	*  https://eba.europa.eu/regulation-and-policy/single-rulebook/interactive-single-rulebook/-/interactive-single-rulebook/article-id/1922
	*/
	namespace legacyRules
	{
		namespace
		{
			/* @brief	Modeled after the "R" pBeta function, which calculates the cumulative beta distribution
			*  @param[in]	xVariable	The  input variable for which to calculate the cumulative density
			*  @param[in]	aShape		The "alpha" shape parameter of the beta distribution
			*  @param[in]	bShape		The "beta" shape parameter of the beta distribution
			*/
			const double pBeta( const double& xVariable, const double& aShape, const double& bShape )
			{
			
				// Check if the input xVariable lies in the range [0,1] and apply a cap / floor provided
				// the value is within tolerance of the limit
				const double tolerance = 1.0e-10;
				double xValue = xVariable;
				if ( AQ_IS_EQUAL_WITH_TOLERANCE( xValue, 0.0, tolerance ) )		// Equal to zero with tolerance
				{
					xValue = 0.0;
				}
				if (AQ_IS_EQUAL_WITH_TOLERANCE( xValue, 1.0, tolerance  ) )		// Equal to one with tolerance
				{
					xValue = 1.0;
				}
				AQ_REQUIRE( xValue <= 1.0, "In Legacy Supervisory Rules: pbeta called with xVariable greater than 1.0: " << xVariable );
				AQ_REQUIRE( xValue >= 0.0, "In Legacy Supervisory Rules: pbeta called with xVariable less than 0.0: " << xVariable );

				boost::math::beta_distribution<> mybeta( aShape, bShape );
				const double density = boost::math::cdf( mybeta, xValue );
				return density;
			}

			/* @brief Computes  the  K[x] parameter in article 262 of the supervisory formula method document:
			* https://eur-lex.europa.eu/legal-content/EN/TXT/PDF/?uri=CELEX:32013R0575&from=EN
			*
			*  @param[in]	xVariable	The  input variable for which to calculate the cumulative density
			*  @param[in]	aShape		The "alpha" shape parameter of the beta distribution
			*  @param[in]	bShape		The "beta" shape parameter of the beta distribution
			*  @param[in]	h			
			*  @param[in]	c			
			*/
			double calculateKparameter( const double& xVariable, const double& aShape, const double& bShape, const double& h, const double& c )
			{
				const double kParameter = ( 1 - h ) * ( (1.0 - pBeta( xVariable, aShape, bShape )) * xVariable 
										+ pBeta( xVariable, aShape + 1.0, bShape ) * c );
			
				return kParameter;
			}
		}


		/* @brief		Calculates the risk-weight using the (non-IRBA) supervisory formula method
		*				described in article 262 of the following document:
		*				https://eur-lex.europa.eu/legal-content/EN/TXT/PDF/?uri=CELEX:32013R0575&from=EN
		* @param[in]	creditEnhancementLevel	 the ratio of the nominal amount of all tranches subordinate to the
						tranche in which the position is held to the sum of the nominal amounts of the exposures that have been securitised.
		* @param[in]	trancheThickness	The thickness of the tranche in which the position is held (detachment level - attachment level)
		* @param[in]	kirb				The capital requirement on the securitized exposures, in accortance with the Internal Ratings-Based Approach
		* @param[in]	elgd				The exposure-weighted average loss given default: The share of an asset that is lost if a borrower defaults
		* @param[in]	nEffectiveExposures	A measure of the portfolio diversification, which in turn implies the granularity
		* @returns	The calculated risk-weight	
		*/
		double supervisoryFormula( const double& creditEnhancementLevel, const double& trancheThickness, const double& kirb, const double& elgd, const int& nEffectiveExposures )
		{
			AQ_REQUIRE( elgd > 0.0, "ELGD must be positive." );

			// # should be optional:
			// 7% is default
			// 20% for re-securitisations
			const double floor = 0.07;

			const double tau = 1000.0;
			const double omega = 20.0;

			const double h = std::pow( 1.0 - ( kirb / elgd ), nEffectiveExposures );
	
		    const double c = kirb / (1.0 - h );
		    const double v  = ( ( (elgd - kirb ) * kirb ) + (0.25 * (1.0 - elgd ) * kirb) ) / (double) nEffectiveExposures;
		    const double f = ( ( ( v + kirb * kirb ) / (1 - h) ) - c * c ) + ( (1 - kirb ) * kirb - v ) / ( ( 1 - h ) * tau );
		    const double g = ( ( ( 1 - c ) * c) / f ) - 1.0;
		    const double a = g * c;
		    const double b = g * ( 1 - c );

		    const double d = 1 - ( 1 - h ) * (1 - pBeta( kirb, a, b ) );

		    // # K[x]
		    // #K[L]
			const double K_L = calculateKparameter( creditEnhancementLevel, a, b, h, c );

		    // #K[L+T]
			const double combinedThickness = creditEnhancementLevel + trancheThickness;
			const double K_LT = calculateKparameter( combinedThickness, a, b, h, c );

			// #K[KIRB]
			const double K_kirb = calculateKparameter( kirb, a, b, h, c );

		    // # S[L+T]
			double S_LT = 0.0;
		    if ( combinedThickness <= kirb )
			{
				S_LT = combinedThickness; // # x
		    }
			else
			{
				S_LT =  kirb + K_LT - K_kirb + ( 1.0 - std::exp ( omega * ( kirb - combinedThickness ) / kirb) )  * (d * kirb) / omega;
		    }

		    // # S[L]
			double S_L = 0.0;
		    if( creditEnhancementLevel <= kirb )
			{
				S_L = creditEnhancementLevel; // # x
		    }
		    else
			{
				S_L  = kirb + K_L - K_kirb + ( 1.0 - std::exp( omega * ( kirb - creditEnhancementLevel  ) / kirb ) )  * (d * kirb) / omega;
		    }

		    // # SFM
		  
			double sf = 12.5 * (S_LT - S_L) / trancheThickness;

		    sf = std::max( sf, floor );

			return sf;
		}
	}

	/* @brief For details of the new SEC IRBA rules ( Securitization Internal-Ratings-Based-Approach ), see for example this URL:
	*  https://digital.pwc-tools.de/basel-iv/wp-content/uploads/sites/23/2017/03/4173_RZ_Booklet_Toolbox_Securitisations_A6_SCREEN_gesch.pdf
	*
	* This other URL is also useful for an explanation of terms used:
	* http://www.riskcontrollimited.com/public/Calibration_of_SSFA.pdf
	*/
	namespace secIrbaRules
	{
		SecIrbaTableParameters secIrbaTableParametersLoad()
		{
			SecIrbaTableParameters secIrbaTableParameters;

			secIrbaTableParameters.poolTypeKey = { WHOLESALE_POOL_TYPE, WHOLESALE_POOL_TYPE, WHOLESALE_POOL_TYPE, WHOLESALE_POOL_TYPE, RETAIL_POOL_TYPE, RETAIL_POOL_TYPE, RETAIL_POOL_TYPE, RETAIL_POOL_TYPE };
			secIrbaTableParameters.rankTypeKey = { SENIOR_RANK_TYPE, SENIOR_RANK_TYPE, NON_SENIOR_RANK_TYPE, NON_SENIOR_RANK_TYPE, SENIOR_RANK_TYPE, NON_SENIOR_RANK_TYPE, SENIOR_RANK_TYPE, NON_SENIOR_RANK_TYPE };
			secIrbaTableParameters.granularityKey = { GRANULAR_TYPE, NON_GRANULAR_TYPE, GRANULAR_TYPE, NON_GRANULAR_TYPE, GRANULAR_TYPE, GRANULAR_TYPE, NON_GRANULAR_TYPE, NON_GRANULAR_TYPE };
			secIrbaTableParameters.aParams = { 0.00, 0.11, 0.16, 0.22, 0.00, 0.00, 0.00, 0.00 };
			secIrbaTableParameters.bParams = { 3.56, 2.61, 2.87, 2.35, 0.00, 0.00, 0.00, 0.00 };
			secIrbaTableParameters.cParams = { -1.85, -2.91, -1.03, -2.46, -7.48, -5.78, -7.48, -5.78 };
			secIrbaTableParameters.dParams = { 0.55, 0.68, 0.21, 0.48, 0.71, 0.55, 0.71, 0.55 };
			secIrbaTableParameters.eParams = { 0.07, 0.07, 0.07, 0.07, 0.24, 0.27, 0.24, 0.27 };

			return secIrbaTableParameters;
		}
	
		/* @brief	Computes the granularity of the portfolio, given the effective number of exposures.
		*			A granular portfolio is an investment portfolio that is well diversified across a wide
		*			variety of assets.
		* @param[in] nEffectiveExposures	This input specifies the number of different exposures in the portfolio.
		*									Multiple exposures to the same obligor must be consolidated.
		*									Similarly, offsetting exposures-at-default are consolidated.
		* See: http://www.cbrc.gov.cn/chinese/files/2013/99CEBDE462C540D89F17822A377CD84B.pdf
		* Section 4.4.6
		* 
		* @returns	The granularity of the portfolio ( granular / non-granular )
		*/
		SecIrDataProviderranularityTypeEnum determineSecIrDataProviderranularityType( const int& n )
		{
			SecIrDataProviderranularityTypeEnum result = NONE_SPECIFIED_GRANULAR_TYPE;
			if ( n >= 25 )
			{
				result = GRANULAR_TYPE;
			}
			else
			{
				result = NON_GRANULAR_TYPE;
			}
			return result;
		}
	
		/*
		* @brief Returns the relevant row of SEC-IRBA table given the specified key parameters.
		*  The row decision is based on:
		* @param[in]	poolTypeEnum		The type of the pool ("Wholesale" or "Retail")
		* @param[in]	nEffectiveExposures	A measure of the portfolio diversification, which in turn implies the granularity
		* @param[in]	rankTypeEnum		The rank of the tranche ("Senior" or "Non-senior")
		*
		* @returns	The row number of the table, which can be used to look up table parameters.
		*/
		size_t secIrbaTableParametersChooseRow( const PoolTypeEnum& poolTypeEnum, const int& nEffectiveExposures, const RankTypeEnum& rankTypeEnum )
		{
			SecIrDataProviderranularityTypeEnum granularityTypeEnum = determineSecIrDataProviderranularityType( nEffectiveExposures );

			const SecIrbaTableParameters secIrbaTableParameters = secIrbaTableParametersLoad();
			const size_t nParameters = secIrbaTableParameters.poolTypeKey.size();
			size_t row = 0;
			for ( row = 0; row < nParameters;  row++ )
			{
				if ( secIrbaTableParameters.poolTypeKey[ row ] == poolTypeEnum
					&& secIrbaTableParameters.rankTypeKey[ row ] == rankTypeEnum
					&& secIrbaTableParameters.granularityKey[ row ] == granularityTypeEnum )
				{
					break;
				}
			}

			AQ_REQUIRE( row < nParameters, "Could not find matching criteria in SecIrbaTableParameters keys");
			
			return row;
		}
	
		/* @brief	Returns the calculated SEC-IRBA ( Security Internal-Ratings-Based Approach ) parameter, based on the specified inputs
		* @param[in]	poolTypeEnum		The type of the pool ("Wholesale" or "Retail")
		* @param[in]	nEffectiveExposures	A measure of the portfolio diversification, which in turn implies the granularity
		* @param[in]	rankTypeEnum		The rank of the tranche ("Senior" or "Non-senior")
		* @param[in]	kirb				The capital requirement on the securitized exposures, in accortance with the  Internal Ratings-Based Approach
		* @param[in]	lgd					The loss given default: The share of an asset that is lost if a borrower defaults
		* @param[in]	trancheMaturity		The tranches' remaining effective maturity in years.
		*
		* @returns		The supervisory parameter value
		*/
		double secIrbaSupervisoryParameter( const PoolTypeEnum& poolTypeEnum, const int& nEffectiveExposures, const RankTypeEnum& rankTypeEnum, const double& kirb, const double& lgd, const double& trancheMaturity )
		{
			const SecIrbaTableParameters secIrbaTableParameters = secIrbaTableParametersLoad();

			const size_t row = secIrbaTableParametersChooseRow( poolTypeEnum, nEffectiveExposures, rankTypeEnum );

			double result = secIrbaTableParameters.aParams[ row ] 
								+ secIrbaTableParameters.bParams[ row ] / (double) nEffectiveExposures 
								+ secIrbaTableParameters.cParams[ row ] * kirb
								+ secIrbaTableParameters.dParams[ row ] * lgd
								+ secIrbaTableParameters.eParams[ row ] * trancheMaturity;

			const double floor = 0.3;
			result = std::max( floor, result );
			return result;
		}

		/* @brief	Returns the capital calculation of the "Simplified Supervisory Formula Approach of the "Internal Ratings-Based Approach" rules.
		*			For more details of the  calculation, see:
		*			https://digital.pwc-tools.de/basel-iv/wp-content/uploads/sites/23/2017/03/4173_RZ_Booklet_Toolbox_Securitisations_A6_SCREEN_gesch.pdf
		*
		* @param[in]	poolTypeEnum		The type of the pool ("Wholesale" or "Retail")
		* @param[in]	nEffectiveExposures	A measure of the portfolio diversification, which in turn implies the granularity
		* @param[in]	rankTypeEnum		The rank of the tranche ("Senior" or "Non-senior")
		* @param[in]	kirb				The capital requirement on the securitized exposures, in accortance with the  Internal Ratings-Based Approach
		* @param[in]	lgd					The loss given default: The share of an asset that is lost if a borrower defaults
		* @param[in]	trancheMaturity		The tranches' remaining effective maturity in years.
		* @param[in]	attachmentPoint		Specifies the level of subordination of the tranche.
		* @param[in]	detachmentPoint	Specifies the level where the tranche exposure ends.
		*
		* @returns		The capital requirement
		*/
		double secIrbaKssfa( const PoolTypeEnum& poolTypeEnum,
							 const int& nEffectiveExposures,
							 const RankTypeEnum& rankTypeEnum,
							 const double& kirb,
							 const double& lgd,
							 const double& trancheMaturity,
							 const double& attachmentPoint,
							 const double& detachmentPoint )
		{
			const double supervisoryParam = secIrbaSupervisoryParameter( poolTypeEnum, nEffectiveExposures, rankTypeEnum, kirb, lgd, trancheMaturity );
			const double a = -1.0 * ( 1.0 / ( supervisoryParam * kirb ) );
			const double u = detachmentPoint - kirb;
			const double l = std::max( attachmentPoint - kirb, 0.0 );

			double result = std::exp( a * u ) - std::exp( a * l );
			result = result / ( a * ( u - l ) );

			return result;
		}

		/* @brief	Calculates the risk-weighting for assets in a tranche,following the SEC IRBA ( Internal Ratings-Based Approach ).
		*
		* @param[in]	poolTypeEnum		The type of the pool ("Wholesale" or "Retail")
		* @param[in]	nEffectiveExposures	A measure of the portfolio diversification, which in turn implies the granularity
		* @param[in]	rankTypeEnum		The rank of the tranche ("Senior" or "Non-senior")
		* @param[in]	kirb				The capital requirement on the securitized exposures, in accortance with the  Internal Ratings-Based Approach
		* @param[in]	lgd					The loss given default: The share of an asset that is lost if a borrower defaults
		* @param[in]	trancheMaturity		The tranches' remaining effective maturity in years.
		* @param[in]	attachmentPoint		Specifies the level of subordination of the tranche.
		* @param[in]	detachmentPoint	Specifies the level where the tranche exposure ends.
		* @param[in]	rwFloor				A lower bound on the calculated risk-weight value
		*
		* @returns		The risk-weight value
		*/
		double secIrbaRiskWeight( const PoolTypeEnum& poolTypeEnum,
								  const int& nEffectiveExposures,
								  const RankTypeEnum& rankTypeEnum,
								  const double& kirb,
								  const double& ldg,
								  const double& trancheMaturity,
								  const double& attachmentPoint,
								  const double& detachmentPoint,
								  const double& rwFloor )
		{
			// # calculates the SEC-IRBA RW under for any different tranche and reference pool
			const double A_to_K_IRB_weight =  ( kirb - attachmentPoint ) / ( detachmentPoint - attachmentPoint );

			const double K_IRB_to_D_weight =  ( detachmentPoint - kirb ) / ( detachmentPoint - attachmentPoint );

			const double kssfa = secIrbaKssfa( poolTypeEnum, nEffectiveExposures, rankTypeEnum, kirb, ldg, trancheMaturity, attachmentPoint, detachmentPoint  );

			double result = 0.0;
			if ( detachmentPoint <= kirb )
			{
				result = 12.5;
			}
			else if ( attachmentPoint >= kirb )
			{
				result = 12.5 * kssfa;
			}
			else
			{
				result = 12.5 * ( A_to_K_IRB_weight + ( K_IRB_to_D_weight * kssfa ) );
			}

			result = std::max( result, rwFloor );

			return result;
		}
	
	}

}

