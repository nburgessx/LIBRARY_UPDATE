/* 
 * @brief			Swig interface to Java for function mirSetUpFwdFXConstantCurve
 * @Created:		30 March 2016 
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "InitializeMLibETrading.h"
#include "mirSetUpFwdFXConstantCurve.h"
#include "LADate.h"
#include "LAString.h"
#include "LACoreTemplateType.h"
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
											const SWIG_STRINGMATRIX & GeneralProps, 
											const SWIG_STRINGMATRIX & FwdFXConstantConv,
											const std::string& CurveNames) 
{
	LAString ret;
	try 
	{
		// marshall all inputs
		LAString curveID(CurveID.c_str());
		LAString marketName(MarketName.c_str());
		LAString curveNames(CurveNames.c_str());

		LAStringMatrix generalProps;
		swig::buildStringMatrix(generalProps, GeneralProps);

		LAStringMatrix fwdFXConstantConv;
		swig::buildStringMatrix(fwdFXConstantConv, FwdFXConstantConv);

		ret = validation_api::tryMirSetUpFwdFXConstantCurve(etrading::InitializeMLibETrading::instance().dataInstance(), 
																	curveID, 
																	marketName, 
																	generalProps, 
																	fwdFXConstantConv,
																	curveNames).getCString();
	} 
	catch (LACoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 

	return ret.getCString();

}