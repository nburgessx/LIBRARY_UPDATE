#include "LACoreTemplateType.h"
#include "TypeUtilities.h"
#include "meCurveCalibrateFXForwards.h"
#include "tryMeCurveCalibrateFXForwards.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros


/* @brief			swig interface for meCurveCalibrateFXForwards
*  @param [in]		curveCollection		The curve collection ID
*  @param [in]		staticDataTable		Name of the curve constructed by this method
*  @param [in]		curveIndex			Equivalent names of the curve being built
*  @param [in]		curveConv			General curve properties such as asofdate, ccy, interp, etc
*  @param [in]		fxFwdConv			FX forward conventions
*/
const std::string meCurveCalibrateFXForwards(const std::string& curveCollection, 
											const std::string& staticDataTable, 
											const std::string& curveIndex,
											const SWIG_STRINGMATRIX & curveConv, 
											const SWIG_STRINGMATRIX & fxFwdConv)
{
    AQ_API_START
	LAString ret;
	
    // marshall all inputs
	LAString tmp_curveCollection(curveCollection.c_str());
	LAString tmp_staticDataTable(staticDataTable.c_str());
	LAString tmp_curveIndex(curveIndex.c_str());

	LAStringMatrix tmp_curveConv;
	swig::buildStringMatrix(tmp_curveConv, curveConv);

	LAStringMatrix tmp_fxFwdConv;
	swig::buildStringMatrix(tmp_fxFwdConv, fxFwdConv);

	ret = validation_api::tryMeCurveCalibrateFXForwards(
										tmp_curveCollection,
                                        tmp_staticDataTable,
                                        tmp_curveIndex,
                                        tmp_curveConv,
                                        tmp_fxFwdConv).getCString();
    return ret.getCString();
    AQ_API_END
}