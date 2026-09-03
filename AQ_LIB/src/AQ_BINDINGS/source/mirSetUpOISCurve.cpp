/* 
 * @brief			Swig interface to Java for function mirSetUpOISCurve
 * @Created:		14 March 2016 
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#include "InitializeMLibETrading.h"
#include "TypeUtilities.h"
#include "mirSetUpOISCurve.h"
#include "LADate.h"
#include "LAString.h"
#include "LACoreTemplateType.h"
#include "tryMirSetUpOISCurve.h"



/* @brief			swig interface for mirSetUpOISCurve
*  @param [in]		CurveID			The curve collection ID
*  @param [in]		MarketName		Name of the curve constructed by this method
*  @param [in]		GeneralProps	General curve properties such as asofdate, ccy, interp, etc
*  @param [in]		OISConv			The OIS curve configuration info
*  @param [in]		OISRates		Constituent OIS instrument rates
*  @param [in]		CurveNames		Equivalent names of the curve being built
*  @param [in]		OISHistRates	Historical OIS fixings
*  @param [in]		LOBasisRates	Libor-OIS basis spreads
*  @param [in]		SwapConv		Libor swap conventions
*  @param [in]		LOBasisConv		Libor-OIS swap conventions
*/
const std::string mirSetUpOISCurve(const std::string& CurveID, 
								const std::string& MarketName, 
								const SWIG_STRINGMATRIX & GeneralProps, 
								const SWIG_STRINGMATRIX & OISConv, 
								const SWIG_STRINGMATRIX & OISRates,
								const std::string& CurveNames, 
								const SWIG_STRINGMATRIX & OISHistRates, 
								const SWIG_STRINGMATRIX & LOBasisRates, 
								const SWIG_STRINGMATRIX & SwapConv, 
								const SWIG_STRINGMATRIX & SwapRates, 
								const SWIG_STRINGMATRIX & LOBasisConv) 
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

		LAStringMatrix oisConv;
		swig::buildStringMatrix(oisConv, OISConv);

		LAStringMatrix oisRates;
		swig::buildStringMatrix(oisRates, OISRates);

		LAStringMatrix oisHistRates;
		swig::buildStringMatrix(oisHistRates, OISHistRates);

		LAStringMatrix loBasisRates;
		swig::buildStringMatrix(loBasisRates, LOBasisRates);

		LAStringMatrix swapConv;
		swig::buildStringMatrix(swapConv, SwapConv);

		LAStringMatrix swapRates;
		swig::buildStringMatrix(swapRates, SwapRates);

		LAStringMatrix loBasisConv;
		swig::buildStringMatrix(loBasisConv, LOBasisConv);

		ret = validation_api::tryMirSetUpOISCurve(etrading::InitializeMLibETrading::instance().dataInstance(), 
												  curveID, 
												  marketName, 
												  generalProps, 
												  oisRates, 
												  oisConv, 
												  curveNames, 
												  oisHistRates, 
												  loBasisRates,
												  loBasisConv,
												  swapRates,
												  swapConv);
	} 
	catch (LACoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 

	return ret.getCString();

}