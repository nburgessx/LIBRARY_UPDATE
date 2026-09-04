#include "TypeUtilities.h"
#include "AQLCoreTemplateType.h"
#include "aqCurvesCalibrateOIS.h"
#include "tryAqCurvesCalibrateOIS.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros

/* @brief			swig interface for aqCurvesCalibrateOIS
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
const std::string aqCurvesCalibrateOIS(const std::string& curveCollection, 
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
	AQ_API_START
    AQLString ret;
	
	// marshall all inputs
	AQLString tmp_curveCollection(curveCollection.c_str());
	AQLString tmp_staticDataTable(staticDataTable.c_str());
	AQLString tmp_curveIndex(curveIndex.c_str());

	AQLStringMatrix tmp_curveConv;
	swig::buildStringMatrix(tmp_curveConv, curveConv);

	AQLStringMatrix tmp_oisConv;
	swig::buildStringMatrix(tmp_oisConv, oisConv);

	AQLStringMatrix tmp_oisRates;
	swig::buildStringMatrix(tmp_oisRates, oisRates);

	AQLStringMatrix tmp_oisHistoricalRates;
	swig::buildStringMatrix(tmp_oisHistoricalRates, oisHistoricalRates);

	AQLStringMatrix tmp_liborOisBasisConv;
	swig::buildStringMatrix(tmp_liborOisBasisConv, liborOisBasisConv);

	AQLStringMatrix tmp_liborOisBasisRates;
	swig::buildStringMatrix(tmp_liborOisBasisRates, liborOisBasisRates);

	AQLStringMatrix tmp_swapConv;
	swig::buildStringMatrix(tmp_swapConv, swapConv);

	AQLStringMatrix tmp_swapRates;
	swig::buildStringMatrix(tmp_swapRates, swapRates);

	ret = validation::tryAqCurvesCalibrateOIS(
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
    AQ_API_END
}