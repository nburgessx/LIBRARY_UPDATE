/* 
 * @brief			Swig interface to Java for function mirSetUpFwdFXConstantCurve
 * @Created:		30 March 2016 
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#include "stdafx.h"
#include "mirSetUpFwdFXConstantCurve.h"

#include "LADate.h"
#include "LAString.h"
#include "MBTemplateType.h"
#include "tryMirSetUpFwdFXConstantCurve.h"
#include "TypeUtilities.h"


/* @brief			swig interface for mirSetUpFwdFXConstantCurve
*  @param [in]		CurveID				The curve collection ID
*  @param [in]		MarketName			Name of the curve constructed by this method
*  @param [in]		GeneralProps		General curve properties such as asofdate, ccy, interp, etc
*  @param [in]		FwdFXConstantConv	FX forward conventions
*  @param [in]		CurveNames			Equivalent names of the curve being built
*/
const std::string mirSetUpFwdFXConstantCurve(const std::string& CurveID, 
											const std::string& MarketName, 
											const std::vector<std::vector <std::string> >& GeneralProps, 
											const std::vector<std::vector <std::string> >& FwdFXConstantConv,
											const std::string& CurveNames) throw(std::exception) 
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

		StringMatrix fwdFXConstantConv;
		swig::buildStringMatrix(fwdFXConstantConv, FwdFXConstantConv);

		ret = validation_api::tryMirSetUpFwdFXConstantCurve(g_root, 
																	curveID, 
																	marketName, 
																	generalProps, 
																	fwdFXConstantConv,
																	curveNames).getCString();
	} 
	catch (MEError& mesx) 
	{
		throw std::exception(mesx.getMsg());
	} 

	return ret.getCString();

}