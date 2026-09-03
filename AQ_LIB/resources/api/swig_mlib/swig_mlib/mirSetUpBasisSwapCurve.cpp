/* 
 * @brief			Swig interface to Java for function mirSetUpBasisSwapCurve
 * @Created:		14 March 2016 
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#include "stdafx.h"
#include "mirSetUpBasisSwapCurve.h"

#include "LADate.h"
#include "LAString.h"
#include "MBTemplateType.h"
#include "tryMirSetUpBasisSwapCurve.h"
#include "TypeUtilities.h"


/* @brief			swig interface for mirSetUpBasisSwapCurve
*  @param [in]		CurveID			The curve collection ID
*  @param [in]		MarketName		Name of the curve constructed by this method
*  @param [in]		GeneralProps	General curve properties such as asofdate, ccy, interp, etc
*  @param [in]		BasisConv		Basis swap conventions
*  @param [in]		BasisRates		Basis swap market rates
*  @param [in]		CurveNames		Equivalent names of the curve being built
*  @param [in]		FwdConv			FX forward market convention
*  @param [in]		FwdFXs			Forward FX rates
*  @param [in]		SpotFXs			Spot FX rates
*/
const std::string mirSetUpBasisSwapCurve(const std::string& CurveID, 
								const std::string& MarketName, 
								const std::vector<std::vector <std::string> >& GeneralProps, 
								const std::vector<std::vector <std::string> >& BasisConv,
								const std::vector<std::vector <std::string> >& BasisRates,
								const std::string& CurveNames, 
								const std::vector<std::vector <std::string> >& FwdConv, 
								const std::vector<std::vector <std::string> >& FwdFXs, 
								const std::vector<std::vector <std::string> >& SpotFXs) throw(std::exception) 
{
	LAString ret;
	try 
	{
		// marshall all inputs
		LAString curveID(CurveID.c_str());
		LAString marketName(MarketName.c_str());
		LAString curveNames(CurveNames.c_str());

		StringMatrix generalProps;
		swig::buildStringMatrix(generalProps, GeneralProps);

		StringMatrix basisConv;
		swig::buildStringMatrix(basisConv, BasisConv);

		StringMatrix basisRates;
		swig::buildStringMatrix(basisRates, BasisRates);

		StringMatrix fwdConv;
		swig::buildStringMatrix(fwdConv, FwdConv);

		StringMatrix fwdFXs;
		swig::buildStringMatrix(fwdFXs, FwdFXs);

		StringMatrix spotFXs;
		swig::buildStringMatrix(spotFXs, SpotFXs);
		
		ret = validation_api::tryMirSetUpBasisSwapCurve(g_root, 
															curveID, 
															marketName, 
															generalProps, 
															basisConv, 
															basisRates, 
															curveNames, 
															fwdConv, 
															fwdFXs,
															spotFXs);
	} 
	catch (MEError& mesx) 
	{
		throw std::exception(mesx.getMsg());
	} 

	return ret.getCString();

}