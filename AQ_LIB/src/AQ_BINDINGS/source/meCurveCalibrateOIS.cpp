#include "TypeUtilities.h"
#include "LACoreTemplateType.h"
#include "meCurveCalibrateOIS.h"
#include "tryMeCurveCalibrateOIS.h"
#include "APISetUp.h"               // MLIB_API_START and MLIB_API_END Macros

/* @brief			swig interface for meCurveCalibrateOIS
*  @param [in]		curveCollection		The curve collection ID
*  @param [in]		staticDataTable		Name of the curve constructed by this method
*  @param [in]		curveIndex			Equivalent names of the curve being built
*  @param [in]		curveConv			General curve properties such as asofdate, ccy, interp, etc
*  @param [in]		oisConv				The OIS curve configuration info
*  @param [in]		oisRates			Constituent OIS instrument rates
*  @param [in]		oisHistoricalRates	Historical OIS fixings
*  @param [in]		liborOisBasisConv	Libor-OIS swap conventions
*  @param [in]		liborOisBasisRates	Libor-OIS basis spreads
*  @param [in]		swapConv			Libor swap conventions
*  @param [in]		swapRates			Libor swap market rates
*/
const std::string meCurveCalibrateOIS(const std::string& curveCollection, 
								const std::string& staticDataTable, 
								const std::string& curveIndex, 
								const SWIG_STRINGMATRIX & curveConv, 
								const SWIG_STRINGMATRIX & oisConv, 
								const SWIG_STRINGMATRIX & oisRates, 
								const SWIG_STRINGMATRIX & oisHistoricalRates, 
								const SWIG_STRINGMATRIX & liborOisBasisConv, 
								const SWIG_STRINGMATRIX & liborOisBasisRates, 
								const SWIG_STRINGMATRIX & swapConv, 
								const SWIG_STRINGMATRIX & swapRates) 
{
	MLIB_API_START
    LAString ret;
	
	// marshall all inputs
	LAString tmp_curveCollection(curveCollection.c_str());
	LAString tmp_staticDataTable(staticDataTable.c_str());
	LAString tmp_curveIndex(curveIndex.c_str());

	LAStringMatrix tmp_curveConv;
	swig::buildStringMatrix(tmp_curveConv, curveConv);

	LAStringMatrix tmp_oisConv;
	swig::buildStringMatrix(tmp_oisConv, oisConv);

	LAStringMatrix tmp_oisRates;
	swig::buildStringMatrix(tmp_oisRates, oisRates);

	LAStringMatrix tmp_oisHistoricalRates;
	swig::buildStringMatrix(tmp_oisHistoricalRates, oisHistoricalRates);

	LAStringMatrix tmp_liborOisBasisConv;
	swig::buildStringMatrix(tmp_liborOisBasisConv, liborOisBasisConv);

	LAStringMatrix tmp_liborOisBasisRates;
	swig::buildStringMatrix(tmp_liborOisBasisRates, liborOisBasisRates);

	LAStringMatrix tmp_swapConv;
	swig::buildStringMatrix(tmp_swapConv, swapConv);

	LAStringMatrix tmp_swapRates;
	swig::buildStringMatrix(tmp_swapRates, swapRates);

	ret = validation_api::tryMeCurveCalibrateOIS(
										tmp_curveCollection,
                                        tmp_staticDataTable,
                                        tmp_curveIndex,
                                        tmp_curveConv,
                                        tmp_oisConv,
                                        tmp_oisRates,
                                        tmp_oisHistoricalRates,
                                        tmp_liborOisBasisConv,
                                        tmp_liborOisBasisRates,
                                        tmp_swapConv,
                                        tmp_swapRates);
	return ret.getCString();
    MLIB_API_END
}