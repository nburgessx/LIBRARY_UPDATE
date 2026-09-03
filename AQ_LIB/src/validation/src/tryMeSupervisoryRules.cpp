
#include "tryMeSupervisoryRules.h"
#include "RecordMacros.h"
#include "StructuredExceptionHandler.h"


namespace validation
{
	/* @brief		Calculates the risk-weight using the (non-IRBA) supervisory formula method
	*				described in article 262 of the following document:
	*				https://eur-lex.europa.eu/legal-content/EN/TXT/PDF/?uri=CELEX:32013R0575&from=EN
	* @param[in]	creditEnhancementLevel	the ratio of the nominal amount of all tranches subordinate to the
											tranche in which the position is held to the sum of the nominal amounts of the exposures that have been securitised.
	* @param[in]	trancheThickness		The thickness of the tranche in which the position is held (detachment level - attachment level)
	* @param[in]	kirb					The capital requirement on the securitized exposures, in accortance with the Internal Ratings-Based Approach
	* @param[in]	elgd					The exposure-weighted average loss given default: The share of an asset that is lost if a borrower defaults
	* @param[in]	nEffectiveExposures		A measure of the portfolio diversification, which in turn implies the granularity
	* @returns	The calculated risk-weight	
	*/
	double tryMeSRTSupervisoryFormula( const double& creditEnhancementLevel, const double& trancheThickness, const double& kirb, const double& elgd, const int& nEffectiveExposures )
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS( creditEnhancementLevel, trancheThickness, kirb, elgd, nEffectiveExposures );

		const double riskWeight = etrading::legacyRules::supervisoryFormula( creditEnhancementLevel, trancheThickness, kirb, elgd,  nEffectiveExposures );
		
		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT( riskWeight );

		VALID_EXCEPTION_END
	}

	/* @brief	Calculates the risk-weighting for assets in a tranche,following the SEC IRBA ( Internal Ratings-Based Approach ).
	*
	* @param[in]	poolType			The type of the pool ("Wholesale" or "Retail")
	* @param[in]	nEffectiveExposures	A measure of the portfolio diversification, which in turn implies the granularity
	* @param[in]	rankType			The rank of the tranche ("Senior" or "Non-senior")
	* @param[in]	kirb				The capital requirement on the securitized exposures, in accortance with the  Internal Ratings-Based Approach
	* @param[in]	lgd					The loss given default: The share of an asset that is lost if a borrower defaults
	* @param[in]	trancheMaturity		The tranches' remaining effective maturity in years.
	* @param[in]	attachmentPoint		Specifies the level of subordination of the tranche.
	* @param[in]	detachmentPoint	Specifies the level where the tranche exposure ends.
	* @param[in]	rwFloor				A lower bound on the calculated risk-weight value
	*
	* @returns		The risk-weight value
	*/
	double tryMeSRTSecIrbaRiskWeight( const std::string& poolType,
									  const int& nEffectiveExposures,
									  const std::string& rankType,
									  const double& kirb,
									  const double& lgd,
									  const double& trancheMaturity,
									  const double& attachmentPoint,
									  const double& detachmentPoint,
									  const double& rwFloor )
	{
		VALID_EXCEPTION_START
	
		// Record Inputs for logs, tests and playback
        RECORD_INPUTS( poolType, nEffectiveExposures, rankType, kirb, lgd, trancheMaturity, attachmentPoint, detachmentPoint, rwFloor );


		const etrading::PoolTypeEnum poolTypeEnum = etrading::toPoolTypeEnum( poolType );
		const etrading::RankTypeEnum rankTypeEnum = etrading::toRankTypeEnum( rankType );

		const double riskWeight = etrading::secIrbaRules::secIrbaRiskWeight( poolTypeEnum,
																			 nEffectiveExposures,
																			 rankTypeEnum,
																			 kirb,
																			 lgd,
																			 trancheMaturity,
																			 attachmentPoint,
																			 detachmentPoint,
																			 rwFloor );

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT( riskWeight );

		VALID_EXCEPTION_END
	}
}

