#include "InitializeAQETrading.h"
#include "mirSetUpFwdFXConstantCurve.h"
#include "AQLDate.h"
#include "AQLString.h"
#include "AQLCoreTemplateType.h"
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
	AQLString ret;
	try 
	{
		// marshall all inputs
		AQLString curveID(CurveID.c_str());
		AQLString marketName(MarketName.c_str());
		AQLString curveNames(CurveNames.c_str());

		AQLStringMatrix generalProps;
		swig::buildStringMatrix(generalProps, GeneralProps);

		AQLStringMatrix fwdFXConstantConv;
		swig::buildStringMatrix(fwdFXConstantConv, FwdFXConstantConv);

		ret = validation::tryMirSetUpFwdFXConstantCurve(etrading::InitializeAQETrading::instance().dataInstance(), 
																	curveID, 
																	marketName, 
																	generalProps, 
																	fwdFXConstantConv,
																	curveNames).getCString();
	} 
	catch (AQLCoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 

	return ret.getCString();

}