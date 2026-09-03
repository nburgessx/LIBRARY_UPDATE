#include "AQLCoreTemplateType.h"
#include "TypeUtilities.h"
#include "meCurveCalibrateBasis.h"
#include "tryMeCurveCalibrateBasis.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros


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
	AQ_API_START

    AQLString ret;
	
	// marshall all inputs
	AQLString tmp_curveCollection(curveCollection.c_str());
	AQLString tmp_staticDataTable(staticDataTable.c_str());
	AQLString tmp_curveIndex(curveIndex.c_str());

	AQLStringMatrix tmp_curveConv;
	swig::buildStringMatrix(tmp_curveConv, curveConv);

	AQLStringMatrix tmp_basisConv;
	swig::buildStringMatrix(tmp_basisConv, basisConv);

	AQLStringMatrix tmp_basisRates;
	swig::buildStringMatrix(tmp_basisRates, basisRates);

	AQLStringMatrix tmp_fxFwdConv;
	swig::buildStringMatrix(tmp_fxFwdConv, fxFwdConv);

	AQLStringMatrix tmp_fxFwdRates;
	swig::buildStringMatrix(tmp_fxFwdRates, fxFwdRates);

	AQLStringMatrix tmp_spotFxRates;
	swig::buildStringMatrix(tmp_spotFxRates, spotFxRates);
		
	ret = validation::tryMeCurveCalibrateBasis(
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

    AQ_API_END
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
    AQ_API_START
	
    AQLString ret;
	
    // marshall all inputs
	AQLString tmp_curveCollection(curveCollection.c_str());
	AQLString tmp_staticDataTable(staticDataTable.c_str());
	AQLString tmp_curveIndex(curveIndex.c_str());

	AQLStringMatrix tmp_curveConv;
	swig::buildStringMatrix(tmp_curveConv, curveConv);

	AQLStringMatrix tmp_basisConv;
	swig::buildStringMatrix(tmp_basisConv, basisConv);

	AQLStringMatrix tmp_basisRates;
	swig::buildStringMatrix(tmp_basisRates, basisRates);

	AQLStringMatrix tmp_fxFwdConv;
	swig::buildStringMatrix(tmp_fxFwdConv, fxFwdConv);

	AQLStringMatrix tmp_fxFwdRates;
	swig::buildStringMatrix(tmp_fxFwdRates, fxFwdRates);

	AQLStringMatrix tmp_spotFxRates;
	swig::buildStringMatrix(tmp_spotFxRates, spotFxRates);

	AQLStringMatrix fraConv;
	swig::buildStringMatrix(fraConv, FRAConv);

	AQLStringMatrix fraRates;
	swig::buildStringMatrix(fraRates, FRARates);
		
	ret = validation::tryMeCurveCalibrateBasis(
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

    AQ_API_END
}