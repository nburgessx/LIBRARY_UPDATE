#include "meCurveDualBootstrap.h"
#include "TypeUtilities.h"
#include "AQLCoreTemplateType.h"
#include "tryMeCurveDualBootstrap.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros


/* @brief			Swig interface to for curve calibration method using dual-bootstrapping
*  @param [in]		curveCollection		The curve collection ID
*  @param [in]		curveName_db		Name of the dual-bootstrapping curve object
*  @param [in]		curveName_OIS		Name of the OIS curve
*  @param [in]		curveName_swap		Name of the swap curve
*  @param [in]		curveIndex_OIS		Curve index of OIS curve
*  @param [in]		curveIndex_swap		Curve index of swap curve
*  @param [in]		commonParams		Parameters common to the curves
*  @param [in]		generateProp_OIS	Curve build properties of OIS curve
*  @param [in]		oisRates_OIS		OIS rates
*  @param [in]		oisConv_OIS			OIS conventions
*  @param [in]		histRates_OIS		Historical OIS rates
*  @param [in]		lobasisRates_OIS	Libor OIS basis rates
*  @param [in]		lobasisConv_OIS		Libor OIS basis conventions
*  @param [in]		swapConv_OIS		Swap conventions
*  @param [in]		generateProp_swap	Curve build properties of swap curve
*  @param [in]		moneyConv_swap	    Money market conventions
*  @param [in]		liborRates_swap 	Libor rates
*  @param [in]		liborConv_swap 		Libor conventions
*  @param [in]		swapRates_swap 		Swap rates in swap curve
*  @param [in]		swapConv_swap 		Swap conventions in swap curve
*  @param [in]		fra3mRates_swap 	3M FRA rates
*  @param [in]		fra6mRates_swap 	6M FRA rates
*  @param [in]		fraConv_swap 		FRA conventions
*  @param [in]		futureRates_swap 	Futures rates
*  @param [in]		futureConv_swap 	Futures convention
*  @param [in]		adjustSwapConv_swap 	Tenor Basis market data
*  @param [in]		adjustSwapRates_swap 	Tenor Basis convention
*/
const std::string meCurveDualBootstrap(const std::string& curveCollection,
										const std::string& curveName_db,
										const std::string& curveName_OIS,
										const std::string& curveName_swap,
										const std::string& curveIndex_OIS,
										const std::string& curveIndex_swap,
										const SWIG_STRINGMATRIX & commonParams,
										const SWIG_STRINGMATRIX & generateProp_OIS,
										const SWIG_STRINGMATRIX & oisRates_OIS,
										const SWIG_STRINGMATRIX & oisConv_OIS,
										const SWIG_STRINGMATRIX & histRates_OIS,
										const SWIG_STRINGMATRIX & lobasisRates_OIS,
										const SWIG_STRINGMATRIX & lobasisConv_OIS,
										const SWIG_STRINGMATRIX & swapConv_OIS,
										const SWIG_STRINGMATRIX & generateProp_swap,
										const SWIG_STRINGMATRIX & moneyConv_swap,
										const SWIG_STRINGMATRIX & liborRates_swap,
										const SWIG_STRINGMATRIX & liborConv_swap,
										const SWIG_STRINGMATRIX & swapRates_swap,
										const SWIG_STRINGMATRIX & swapConv_swap,
										const SWIG_STRINGMATRIX & fra3mRates_swap,
										const SWIG_STRINGMATRIX & fra6mRates_swap,
										const SWIG_STRINGMATRIX & fraConv_swap,
										const SWIG_STRINGMATRIX & futureRates_swap,
										const SWIG_STRINGMATRIX & futureConv_swap,
										const SWIG_STRINGMATRIX & tenorBasisConv_swap,
										const SWIG_STRINGMATRIX & tenorBasisRates_swap)
{
    AQ_API_START
	AQLString ret;
	
	// marshall all inputs
	AQLString tmp_curveCollection(curveCollection.c_str());
	AQLString tmp_curveName_db(curveName_db.c_str());
	AQLString tmp_curveName_OIS(curveName_OIS.c_str());
	AQLString tmp_curveName_swap(curveName_swap.c_str());
	AQLString tmp_curveIndex_OIS(curveIndex_OIS.c_str());
	AQLString tmp_curveIndex_swap(curveIndex_swap.c_str());

	AQLStringMatrix tmp_commonParams;
	swig::buildStringMatrix(tmp_commonParams, commonParams);

	AQLStringMatrix tmp_generateProp_OIS;
	swig::buildStringMatrix(tmp_generateProp_OIS, generateProp_OIS);

	AQLStringMatrix tmp_oisRates_OIS;
	swig::buildStringMatrix(tmp_oisRates_OIS, oisRates_OIS);

	AQLStringMatrix tmp_oisConv_OIS;
	swig::buildStringMatrix(tmp_oisConv_OIS, oisConv_OIS);

	AQLStringMatrix tmp_histRates_OIS;
	swig::buildStringMatrix(tmp_histRates_OIS, histRates_OIS);

	AQLStringMatrix tmp_lobasisRates_OIS;
	swig::buildStringMatrix(tmp_lobasisRates_OIS, lobasisRates_OIS);

	AQLStringMatrix tmp_lobasisConv_OIS;
	swig::buildStringMatrix(tmp_lobasisConv_OIS, lobasisConv_OIS);

	AQLStringMatrix tmp_swapConv_OIS;
	swig::buildStringMatrix(tmp_swapConv_OIS, swapConv_OIS);

	AQLStringMatrix tmp_generateProp_swap;
	swig::buildStringMatrix(tmp_generateProp_swap, generateProp_swap);

	AQLStringMatrix tmp_moneyConv_swap;
	swig::buildStringMatrix(tmp_moneyConv_swap, moneyConv_swap);

	AQLStringMatrix tmp_liborRates_swap;
	swig::buildStringMatrix(tmp_liborRates_swap, liborRates_swap);

	AQLStringMatrix tmp_liborConv_swap;
	swig::buildStringMatrix(tmp_liborConv_swap, liborConv_swap);

	AQLStringMatrix tmp_swapRates_swap;
	swig::buildStringMatrix(tmp_swapRates_swap, swapRates_swap);

	AQLStringMatrix tmp_swapConv_swap;
	swig::buildStringMatrix(tmp_swapConv_swap, swapConv_swap);
		
	AQLStringMatrix tmp_fra3mRates_swap;
	swig::buildStringMatrix(tmp_fra3mRates_swap, fra3mRates_swap);

	AQLStringMatrix tmp_fra6mRates_swap;
	swig::buildStringMatrix(tmp_fra6mRates_swap, fra6mRates_swap);

	AQLStringMatrix tmp_fraConv_swap;
	swig::buildStringMatrix(tmp_fraConv_swap, fraConv_swap);

	AQLStringMatrix tmp_futureRates_swap;
	swig::buildStringMatrix(tmp_futureRates_swap, futureRates_swap);

	AQLStringMatrix tmp_futureConv_swap;
	swig::buildStringMatrix(tmp_futureConv_swap, futureConv_swap);

	AQLStringMatrix tmp_tenorBasisConv_swap;
	swig::buildStringMatrix(tmp_tenorBasisConv_swap, tenorBasisConv_swap);

	AQLStringMatrix tmp_tenorBasisRates_swap;
	swig::buildStringMatrix(tmp_tenorBasisRates_swap, tenorBasisRates_swap);

	ret = validation::tryMeCurveDualBootstrap(tmp_curveCollection,
													tmp_curveName_db,
													tmp_curveName_swap,
													tmp_curveName_OIS,
													tmp_curveIndex_OIS,
													tmp_curveIndex_swap,
													tmp_commonParams,
													tmp_generateProp_OIS,
													tmp_oisRates_OIS,
													tmp_oisConv_OIS,
													tmp_histRates_OIS,
													tmp_lobasisRates_OIS,
													tmp_lobasisConv_OIS,
													tmp_swapConv_OIS,
													tmp_generateProp_swap,
													tmp_moneyConv_swap,
													tmp_liborRates_swap,
													tmp_liborConv_swap,
													tmp_swapRates_swap,
													tmp_swapConv_swap,
													tmp_fra3mRates_swap,
													tmp_fra6mRates_swap,
													tmp_fraConv_swap,
													tmp_futureRates_swap,
													tmp_futureConv_swap,
													tmp_tenorBasisConv_swap,
													tmp_tenorBasisRates_swap);
	
    return ret.getCString();
    AQ_API_END
}