#pragma once

#include "AQLCoreTemplateType.h"

namespace validation
{
	
	/* @brief			validation interface for tryAqVolatilitySABRGetPrem
	*  @param [in]		expPoint		Expiry tenor
	*  @param [in]		tenorPoint		Swap tenor tenor
	*  @param [in]		strike			Strike
	*  @param [in]		sign			1 is Call; -1 is Put
	*  @param [in]		forwardID		ID for forward rate matrix
	*  @param [in]		annuityID		ID for annuity matrix
	*  @param [in]		alphaID			ID for alpha matrix
	*  @param [in]		betaID			ID for beta matrix
	*  @param [in]		rhoID			ID for rho matrix
	*  @param [in]		approxMethod	Approximation method, HAGAN or ANTONOVE?
	*  @param [in]		shift			shift size of the Shifted SABR
	*  @param [in]		volType			volType of the SABR, default to lognormal
	*/
	double tryAqVolatilitySABRGetPrem(const AQLString& expPoint, 
							const AQLString& tenorPoint, 
							double strike, 
							int sign, 
							const AQLString& fowardID, 
							const AQLString& numeraireID,
							const AQLString& alphaID, 
							const AQLString& betaID, 
							const AQLString& nuID, 
							const AQLString& rhoID, 
							const AQLString& approxMethod,
							const double shift = 0.0,
							const std::string& volType = "");

}
