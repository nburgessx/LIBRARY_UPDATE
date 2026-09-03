/*
 * @brief			Structured Credit Supervisory Rules
 *					Contains functions to calculate regulatory risk weighting of loans
 *
 *					This code was ported from MGEN MHI R analytics library
 *					Author Andrew Friend
 * @Created:		20th March 2019
 * @Author:			Ian Castleton
 * @Department:	    Quant Research & Analytics
 *
 */

#pragma once

#include "CoreEnumerations.h"

namespace etrading
{
	/* @brief For details of the LARule Supervisory Formula calculation below please see this URL:
	*  https://eba.europa.eu/regulation-and-policy/single-rulebook/interactive-single-rulebook/-/interactive-single-rulebook/article-id/1922
	*/
	namespace legacyRules
	{
		/* @brief		Calculates the risk-weight using the (non-IRBA) supervisory formula method
		* @param[in]	creditEnhancementLevel	 the ratio of the nominal amount of all tranches subordinate to the
						tranche in which the position is held to the sum of the nominal amounts of the exposures that have been securitised.
		* @param[in]	trancheThickness	The thickness of the tranche in which the position is held (detachment level - attachment level)
		* @param[in]	kirb				The capital requirement on the securitized exposures, in accortance with the  Internal Ratings-Based Approach
		* @param[in]	elgd				The exposure-weighted average loss given default: The share of an asset that is lost if a borrower defaults
		* @param[in]	nEffectiveExposures	A measure of the portfolio diversification, which in turn implies the granularity
		* @returns	The calculated risk-weight	
		*/
		double supervisoryFormula( const double& creditEnhancementLevel, const double& trancheThickness, const double& kirb, const double& elgd, const int& nEffectiveExposures );
	}

	/* @brief For details of the new SEC IRBA rules ( Securitization Internal-Ratings-Based-Approach ), see for example this URL:
	*  https://digital.pwc-tools.de/basel-iv/wp-content/uploads/sites/23/2017/03/4173_RZ_Booklet_Toolbox_Securitisations_A6_SCREEN_gesch.pdf
	*
	* This other URL is also useful for an explanation of terms used:
	* http://www.riskcontrollimited.com/public/Calibration_of_SSFA.pdf
	*/
	namespace secIrbaRules
	{
		/* 
		*/
		struct SecIrbaTableParameters
		{
			// These three keys are lookup parameters
			std::vector<PoolTypeEnum> poolTypeKey;
			std::vector<RankTypeEnum> rankTypeKey;
			std::vector<SecIrDataProviderranularityTypeEnum> granularityKey;

			// These parameters contain the data used to calculate the overall risk-weight
			std::vector<double> aParams;
			std::vector<double> bParams;
			std::vector<double> cParams;
			std::vector<double> dParams;
			std::vector<double> eParams;
		};

		/* @brief Helper function which returns the populated internal-ratings-based-approach table
		*/
		SecIrbaTableParameters secIrbaTableParametersLoad();

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
		SecIrDataProviderranularityTypeEnum determineSecIrDataProviderranularityType( const int& nEffectiveExposures );
		
		/*
		* @brief Returns the relevant row of SEC-IRBA ( Security Internal-Ratings-Based Approach ) table given the specified key parameters.
		*  The row decision is based on:
		* @param[in]	poolTypeEnum		The type of the pool ("Wholesale" or "Retail")
		* @param[in]	nEffectiveExposures	A measure of the portfolio diversification, which in turn implies the granularity
		* @param[in]	rankTypeEnum		The rank of the tranche ("Senior" or "Non-senior")
		*
		* @returns	The row number of the table, which can be used to look up table parameters.
		*/
		size_t secIrbaTableParametersChooseRow( const PoolTypeEnum& poolTypeEnum, const int& nEffectiveExposures, const RankTypeEnum& rankTypeEnum );
	

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
		double secIrbaSupervisoryParameter( const PoolTypeEnum& poolTypeEnum, const int& nEffectiveExposures, const RankTypeEnum& rankTypeEnum, const double& kirb, const double& lgd, const double& trancheMaturity );

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
							 const double& detachmentPoint );

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
								  const double& lgd,
								  const double& trancheMaturity,
								  const double& attachmentPoint,
								  const double& detachmentPoint,
								  const double& rwFloor = 0.15 );

	}

}


