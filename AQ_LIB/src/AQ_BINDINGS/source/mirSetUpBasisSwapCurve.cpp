/* 
 * @brief			Swig interface to Java for function mirSetUpBasisSwapCurve
 * @Created:		14 March 2016 
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "InitializeMLibETrading.h"
#include "mirSetUpBasisSwapCurve.h"
#include "LADate.h"
#include "LAString.h"
#include "LACoreTemplateType.h"
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
								const SWIG_STRINGMATRIX & GeneralProps, 
								const SWIG_STRINGMATRIX & BasisConv,
								const SWIG_STRINGMATRIX & BasisRates,
								const std::string& CurveNames, 
								const SWIG_STRINGMATRIX & FwdConv, 
								const SWIG_STRINGMATRIX & FwdFXs, 
								const SWIG_STRINGMATRIX & SpotFXs) 
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

		LAStringMatrix basisConv;
		swig::buildStringMatrix(basisConv, BasisConv);

		LAStringMatrix basisRates;
		swig::buildStringMatrix(basisRates, BasisRates);

		LAStringMatrix fwdConv;
		swig::buildStringMatrix(fwdConv, FwdConv);

		LAStringMatrix fwdFXs;
		swig::buildStringMatrix(fwdFXs, FwdFXs);

		LAStringMatrix spotFXs;
		swig::buildStringMatrix(spotFXs, SpotFXs);
		
		ret = validation_api::tryMirSetUpBasisSwapCurve(etrading::InitializeMLibETrading::instance().dataInstance(), 
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
	catch (LACoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 

	return ret.getCString();

}



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
*  @param [in]		FRAConv			FRA convention
*  @param [in]		FRARates		FRA market data
*/
const std::string mirSetUpBasisSwapCurve(const std::string& CurveID, 
										const std::string& MarketName, 
										const SWIG_STRINGMATRIX & GeneralProps, 
										const SWIG_STRINGMATRIX & BasisConv,
										const SWIG_STRINGMATRIX & BasisRates,
										const std::string& CurveNames, 
										const SWIG_STRINGMATRIX & FwdConv, 
										const SWIG_STRINGMATRIX & FwdFXs, 
										const SWIG_STRINGMATRIX & SpotFXs,
										const SWIG_STRINGMATRIX & FRAConv,
										const SWIG_STRINGMATRIX & FRARates)
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

		LAStringMatrix basisConv;
		swig::buildStringMatrix(basisConv, BasisConv);

		LAStringMatrix basisRates;
		swig::buildStringMatrix(basisRates, BasisRates);

		LAStringMatrix fwdConv;
		swig::buildStringMatrix(fwdConv, FwdConv);

		LAStringMatrix fwdFXs;
		swig::buildStringMatrix(fwdFXs, FwdFXs);

		LAStringMatrix spotFXs;
		swig::buildStringMatrix(spotFXs, SpotFXs);

		LAStringMatrix fraConv;
		swig::buildStringMatrix(fraConv, FRAConv);

		LAStringMatrix fraRates;
		swig::buildStringMatrix(fraRates, FRARates);
		
		ret = validation_api::tryMirSetUpBasisSwapCurve(etrading::InitializeMLibETrading::instance().dataInstance(), 
															curveID, 
															marketName, 
															generalProps, 
															basisConv, 
															basisRates, 
															curveNames, 
															fwdConv, 
															fwdFXs,
															spotFXs,
															fraConv,
															fraRates);
	} 
	catch (LACoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 

	return ret.getCString();
}