/*
 * @brief			validation interface for meSABRGetVol
 * @Created:		30 October 2018
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */


#pragma once

#include "LACoreTemplateType.h"

namespace validation_api
{
	
	/* @brief			validation interface for tryMeSABRGetVol
    *  @param [in]		expPoint		Expiry tenor
	*  @param [in]		tenorPoint		Swap tenor tenor
	*  @param [in]		strike			Strike
	*  @param [in]		forwardID		ID for forward rate matrix
	*  @param [in]		alphaID			ID for alpha matrix
	*  @param [in]		betaID			ID for beta matrix
	*  @param [in]		rhoID			ID for rho matrix
	*  @param [in]		approxMethod	Approximation method, HAGAN or ANTONOVE?
	*  @param [in]		shift			shift size of the Shifted SABR
	*  @param [in]		volType			volType of the SABR, default to lognormal
	*/
	double tryMeSABRGetVol(const LAString& expPoint,
							const LAString& tenorPoint, 
							double strike, 
							const LAString& fowardID, 
							const LAString& alphaID, 
							const LAString& betaID, 
							const LAString& nuID, 
							const LAString& rhoID, 
							const LAString& approxMethod,
							double shift = 0.0,
							const std::string& volType = "");
}
