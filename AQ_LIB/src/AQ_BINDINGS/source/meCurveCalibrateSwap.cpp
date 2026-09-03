#include "TypeUtilities.h"
#include "LACoreTemplateType.h"
#include "meCurveCalibrateSwap.h"
#include "tryMeCurveCalibrateSwap.h"
#include "APISetUp.h"               // MLIB_API_START and MLIB_API_END Macros


/* @brief			swig interface for meCurveCalibrateSwap
*  @param [in]		curveCollection		The curve collection ID
*  @param [in]		staticDataTable		Name of the curve constructed by this method
*  @param [in]		curveIndex			Equivalent names of the curve being built
*  @param [in]		curveConv			General yield curve data
*  @param [in]		moneyMarketConv			Money market conventions
*  @param [in]		liborConv			Libor market conventions
*  @param [in]		liborRates			Libor market data
*  @param [in]		swapConv			Swap market conventions
*  @param [in]		swapRates			Swap market data
*  @param [in]		fraConv				FRA market conventions
*  @param [in]		fra3mRates			3M FRA market data
*  @param [in]		fra6mRates			6M FRA market data
*  @param [in]		futureConv			Futures market conventions
*  @param [in]		futureRates			Futures market data
*  @param [in]		tenorBasisConv	    Tenor Basis market conventions
*  @param [in]		tenorBasisRates	    Tenor Basis market data
*/
const std::string meCurveCalibrateSwap( const std::string& curveCollection, 
								        const std::string& staticDataTable, 
								        const std::string& curveIndex,
								        const SWIG_STRINGMATRIX & curveConv, 
								        const SWIG_STRINGMATRIX & moneyMarketConv,
								        const SWIG_STRINGMATRIX & liborConv,
								        const SWIG_STRINGMATRIX & liborRates,
								        const SWIG_STRINGMATRIX & swapConv,
								        const SWIG_STRINGMATRIX & swapRates,
								        const SWIG_STRINGMATRIX & fraConv,
								        const SWIG_STRINGMATRIX & fra3mRates, 
								        const SWIG_STRINGMATRIX & fra6mRates, 
								        const SWIG_STRINGMATRIX & futureConv,
								        const SWIG_STRINGMATRIX & futureRates,
								        const SWIG_STRINGMATRIX & tenorBasisConv,
								        const SWIG_STRINGMATRIX & tenorBasisRates ) 
{
    MLIB_API_START
	LAString ret;
	
	// marshall all inputs
	LAString tmp_curveCollection(curveCollection.c_str());
	LAString tmp_staticDataTable(staticDataTable.c_str());
	LAString tmp_curveIndex(curveIndex.c_str());

	LAStringMatrix tmp_curveConv;
	swig::buildStringMatrix(tmp_curveConv, curveConv);

	LAStringMatrix tmp_moneyMarketConv;
	swig::buildStringMatrix(tmp_moneyMarketConv, moneyMarketConv);

	LAStringMatrix tmp_liborConv;
	swig::buildStringMatrix(tmp_liborConv, liborConv);

	LAStringMatrix tmp_liborRates;
	swig::buildStringMatrix(tmp_liborRates, liborRates);

	LAStringMatrix tmp_swapConv;
	swig::buildStringMatrix(tmp_swapConv, swapConv);

	LAStringMatrix tmp_swapRates;
	swig::buildStringMatrix(tmp_swapRates, swapRates);

	LAStringMatrix tmp_fraConv;
	swig::buildStringMatrix(tmp_fraConv, fraConv);

	LAStringMatrix tmp_fra3mRates;
	swig::buildStringMatrix(tmp_fra3mRates, fra3mRates);

	LAStringMatrix tmp_fra6mRates;
	swig::buildStringMatrix(tmp_fra6mRates, fra6mRates);

	LAStringMatrix tmp_futureConv;
	swig::buildStringMatrix(tmp_futureConv, futureConv);

	LAStringMatrix tmp_futureRates;
	swig::buildStringMatrix(tmp_futureRates, futureRates);

	LAStringMatrix tmp_tenorBasisConv;
	swig::buildStringMatrix(tmp_tenorBasisConv, tenorBasisConv);

	LAStringMatrix tmp_tenorBasisRates;
	swig::buildStringMatrix(tmp_tenorBasisRates, tenorBasisRates);

	ret = validation_api::tryMeCurveCalibrateSwap( tmp_curveCollection,
                                                    tmp_staticDataTable,
                                                    tmp_curveIndex,
                                                    tmp_curveConv,
                                                    tmp_moneyMarketConv,
                                                    tmp_liborConv,
                                                    tmp_liborRates,
                                                    tmp_swapConv,
                                                    tmp_swapRates,
                                                    tmp_fraConv,
                                                    tmp_fra3mRates,
                                                    tmp_fra6mRates,
                                                    tmp_futureConv,
                                                    tmp_futureRates,
                                                    tmp_tenorBasisConv,
                                                    tmp_tenorBasisRates );
	return ret.getCString();
    MLIB_API_END
}
