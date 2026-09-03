#pragma once

#include "LACoreTemplateType.h"

namespace validation_api
{
	
	/* @brief			validation interface for tryMeSABRGetPrem
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
	double tryMeSABRGetPrem(const LAString& expPoint, 
							const LAString& tenorPoint, 
							double strike, 
							int sign, 
							const LAString& fowardID, 
							const LAString& numeraireID,
							const LAString& alphaID, 
							const LAString& betaID, 
							const LAString& nuID, 
							const LAString& rhoID, 
							const LAString& approxMethod,
							const double shift = 0.0,
							const std::string& volType = "");

}
