/* 
 * @brief			Swig interface to Java for function meCurveCalibrateBasis
 * @Created:		03 June 2016 
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */


#include "LACoreTemplateType.h"
#include "TypeUtilities.h"
#include "meCurveCalibrateBasis.h"
#include "tryMeCurveCalibrateBasis.h"
#include "APISetUp.h"               // MLIB_API_START and MLIB_API_END Macros


/* @brief			swig interface for meCurveCalibrateBasis
*  @param [in]		curveCollection		The curve collection ID
*  @param [in]		staticDataTable		Name of the curve constructed by this method
*  @param [in]		curveIndex			Equivalent names of the curve being built
*  @param [in]		curveConv			General curve properties such as asofdate, ccy, interp, etc
*  @param [in]		basisConv			Basis swap conventions
*  @param [in]		basisRates			Basis swap market rates
*  @param [in]		fxFwdConv			FX forward market convention
*  @param [in]		fxFwdRates			Forward FX rates
*  @param [in]		spotFxRates			Spot FX rates
*/
const std::string meCurveCalibrateBasis(const std::string& curveCollection, 
										const std::string& staticDataTable, 
										const std::string& curveIndex, 
										const SWIG_STRINGMATRIX & curveConv, 
										const SWIG_STRINGMATRIX & basisConv,
										const SWIG_STRINGMATRIX & basisRates,
										const SWIG_STRINGMATRIX & fxFwdConv, 
										const SWIG_STRINGMATRIX & fxFwdRates, 
										const SWIG_STRINGMATRIX & spotFxRates) 
{
	MLIB_API_START

    LAString ret;
	
	// marshall all inputs
	LAString tmp_curveCollection(curveCollection.c_str());
	LAString tmp_staticDataTable(staticDataTable.c_str());
	LAString tmp_curveIndex(curveIndex.c_str());

	LAStringMatrix tmp_curveConv;
	swig::buildStringMatrix(tmp_curveConv, curveConv);

	LAStringMatrix tmp_basisConv;
	swig::buildStringMatrix(tmp_basisConv, basisConv);

	LAStringMatrix tmp_basisRates;
	swig::buildStringMatrix(tmp_basisRates, basisRates);

	LAStringMatrix tmp_fxFwdConv;
	swig::buildStringMatrix(tmp_fxFwdConv, fxFwdConv);

	LAStringMatrix tmp_fxFwdRates;
	swig::buildStringMatrix(tmp_fxFwdRates, fxFwdRates);

	LAStringMatrix tmp_spotFxRates;
	swig::buildStringMatrix(tmp_spotFxRates, spotFxRates);
		
	ret = validation_api::tryMeCurveCalibrateBasis(
												tmp_curveCollection,
												tmp_staticDataTable,
												tmp_curveIndex,
												tmp_curveConv,
												tmp_basisConv,
												tmp_basisRates,
												tmp_fxFwdConv,
												tmp_fxFwdRates,
												tmp_spotFxRates);
	return ret.getCString();

    MLIB_API_END
}



/* @brief			swig interface for meCurveCalibrateBasis
*  @param [in]		curveCollection		The curve collection ID
*  @param [in]		staticDataTable		Name of the curve constructed by this method
*  @param [in]		curveIndex			Equivalent names of the curve being built
*  @param [in]		curveConv			General curve properties such as asofdate, ccy, interp, etc
*  @param [in]		basisConv			Basis swap conventions
*  @param [in]		basisRates			Basis swap market rates
*  @param [in]		fxFwdConv			FX forward market convention
*  @param [in]		fxFwdRates			Forward FX rates
*  @param [in]		spotFxRates			Spot FX rates
*  @param [in]		FRAConv				FRA convention
*  @param [in]		FRARates			FRA market data
*/
const std::string meCurveCalibrateBasis(const std::string& curveCollection, 
										const std::string& staticDataTable, 
										const std::string& curveIndex, 
										const SWIG_STRINGMATRIX & curveConv, 
										const SWIG_STRINGMATRIX & basisConv,
										const SWIG_STRINGMATRIX & basisRates,
										const SWIG_STRINGMATRIX & fxFwdConv, 
										const SWIG_STRINGMATRIX & fxFwdRates, 
										const SWIG_STRINGMATRIX & spotFxRates,
										const SWIG_STRINGMATRIX & FRAConv,
										const SWIG_STRINGMATRIX & FRARates)
{
    MLIB_API_START
	
    LAString ret;
	
    // marshall all inputs
	LAString tmp_curveCollection(curveCollection.c_str());
	LAString tmp_staticDataTable(staticDataTable.c_str());
	LAString tmp_curveIndex(curveIndex.c_str());

	LAStringMatrix tmp_curveConv;
	swig::buildStringMatrix(tmp_curveConv, curveConv);

	LAStringMatrix tmp_basisConv;
	swig::buildStringMatrix(tmp_basisConv, basisConv);

	LAStringMatrix tmp_basisRates;
	swig::buildStringMatrix(tmp_basisRates, basisRates);

	LAStringMatrix tmp_fxFwdConv;
	swig::buildStringMatrix(tmp_fxFwdConv, fxFwdConv);

	LAStringMatrix tmp_fxFwdRates;
	swig::buildStringMatrix(tmp_fxFwdRates, fxFwdRates);

	LAStringMatrix tmp_spotFxRates;
	swig::buildStringMatrix(tmp_spotFxRates, spotFxRates);

	LAStringMatrix fraConv;
	swig::buildStringMatrix(fraConv, FRAConv);

	LAStringMatrix fraRates;
	swig::buildStringMatrix(fraRates, FRARates);
		
	ret = validation_api::tryMeCurveCalibrateBasis(
												tmp_curveCollection,
												tmp_staticDataTable,
												tmp_curveIndex,
												tmp_curveConv,
												tmp_basisConv,
												tmp_basisRates,
												tmp_fxFwdConv,
												tmp_fxFwdRates,
												tmp_spotFxRates,
												fraConv,
												fraRates);
	
	return ret.getCString();

    MLIB_API_END
}