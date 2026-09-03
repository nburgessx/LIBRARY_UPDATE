#include "InitializeETrading.h"
#include "mirSetUpBasisSwapCurve.h"
#include "AQLDate.h"
#include "AQLString.h"
#include "AQLCoreTemplateType.h"
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
	AQLString ret;
	try 
	{
		// marshall all inputs
		AQLString curveID(CurveID.c_str());
		AQLString marketName(MarketName.c_str());
		AQLString curveNames(CurveNames.c_str());

		AQLStringMatrix generalProps;
		swig::buildStringMatrix(generalProps, GeneralProps);

		AQLStringMatrix basisConv;
		swig::buildStringMatrix(basisConv, BasisConv);

		AQLStringMatrix basisRates;
		swig::buildStringMatrix(basisRates, BasisRates);

		AQLStringMatrix fwdConv;
		swig::buildStringMatrix(fwdConv, FwdConv);

		AQLStringMatrix fwdFXs;
		swig::buildStringMatrix(fwdFXs, FwdFXs);

		AQLStringMatrix spotFXs;
		swig::buildStringMatrix(spotFXs, SpotFXs);
		
		ret = validation::tryMirSetUpBasisSwapCurve(etrading::InitializeETrading::instance().dataInstance(), 
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
	catch (AQLCoreError& mesx) 
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
	AQLString ret;
	try 
	{
		// marshall all inputs
		AQLString curveID(CurveID.c_str());
		AQLString marketName(MarketName.c_str());
		AQLString curveNames(CurveNames.c_str());

		AQLStringMatrix generalProps;
		swig::buildStringMatrix(generalProps, GeneralProps);

		AQLStringMatrix basisConv;
		swig::buildStringMatrix(basisConv, BasisConv);

		AQLStringMatrix basisRates;
		swig::buildStringMatrix(basisRates, BasisRates);

		AQLStringMatrix fwdConv;
		swig::buildStringMatrix(fwdConv, FwdConv);

		AQLStringMatrix fwdFXs;
		swig::buildStringMatrix(fwdFXs, FwdFXs);

		AQLStringMatrix spotFXs;
		swig::buildStringMatrix(spotFXs, SpotFXs);

		AQLStringMatrix fraConv;
		swig::buildStringMatrix(fraConv, FRAConv);

		AQLStringMatrix fraRates;
		swig::buildStringMatrix(fraRates, FRARates);
		
		ret = validation::tryMirSetUpBasisSwapCurve(etrading::InitializeETrading::instance().dataInstance(), 
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
	catch (AQLCoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 

	return ret.getCString();
}