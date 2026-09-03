#include "InitializeAQETrading.h"
#include "TypeUtilities.h"
#include "mirSetUpOISCurve.h"
#include "AQLDate.h"
#include "AQLString.h"
#include "AQLCoreTemplateType.h"
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
	AQLString ret;
	try 
	{
		// marshall all inputs
		AQLString curveID(CurveID.c_str());
		AQLString marketName(MarketName.c_str());
		AQLString curveNames(CurveNames.c_str());

		AQLStringMatrix generalProps;
		swig::buildStringMatrix(generalProps, GeneralProps);

		AQLStringMatrix oisConv;
		swig::buildStringMatrix(oisConv, OISConv);

		AQLStringMatrix oisRates;
		swig::buildStringMatrix(oisRates, OISRates);

		AQLStringMatrix oisHistRates;
		swig::buildStringMatrix(oisHistRates, OISHistRates);

		AQLStringMatrix loBasisRates;
		swig::buildStringMatrix(loBasisRates, LOBasisRates);

		AQLStringMatrix swapConv;
		swig::buildStringMatrix(swapConv, SwapConv);

		AQLStringMatrix swapRates;
		swig::buildStringMatrix(swapRates, SwapRates);

		AQLStringMatrix loBasisConv;
		swig::buildStringMatrix(loBasisConv, LOBasisConv);

		ret = validation::tryMirSetUpOISCurve(etrading::InitializeAQETrading::instance().dataInstance(), 
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
	catch (AQLCoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 

	return ret.getCString();

}