/* 
 * @brief			Swig interface to Java for function mirSetUpOISCurve
 * @Created:		14 March 2016 
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#include "stdafx.h"
#include "TypeUtilities.h"
#include "mirSetUpOISCurve.h"
#include "LADate.h"
#include "LAString.h"
#include "MBTemplateType.h"
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
								const std::vector<std::vector <std::string> >& GeneralProps, 
								const std::vector<std::vector <std::string> >& OISConv, 
								const std::vector<std::vector <std::string> >& OISRates,
								const std::string& CurveNames, 
								const std::vector<std::vector <std::string> >& OISHistRates, 
								const std::vector<std::vector <std::string> >& LOBasisRates, 
								const std::vector<std::vector <std::string> >& SwapConv, 
								const std::vector<std::vector <std::string> >& SwapRates, 
								const std::vector<std::vector <std::string> >& LOBasisConv) throw(std::exception) 
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

		StringMatrix oisConv;
		swig::buildStringMatrix(oisConv, OISConv);

		StringMatrix oisRates;
		swig::buildStringMatrix(oisRates, OISRates);

		StringMatrix oisHistRates;
		swig::buildStringMatrix(oisHistRates, OISHistRates);

		StringMatrix loBasisRates;
		swig::buildStringMatrix(loBasisRates, LOBasisRates);

		StringMatrix swapConv;
		swig::buildStringMatrix(swapConv, SwapConv);

		StringMatrix swapRates;
		swig::buildStringMatrix(swapRates, SwapRates);

		StringMatrix loBasisConv;
		swig::buildStringMatrix(loBasisConv, LOBasisConv);

		ret = validation_api::tryMirSetUpOISCurve(g_root, 
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
	catch (MEError& mesx) 
	{
		throw std::exception(mesx.getMsg());
	} 

	return ret.getCString();

}