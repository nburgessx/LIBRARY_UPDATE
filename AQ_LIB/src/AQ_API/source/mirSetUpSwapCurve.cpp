#include "InitializeAQETrading.h"
#include "TypeUtilities.h"
#include "mirSetUpSwapCurve.h"
#include "LADate.h"
#include "LAString.h"
#include "LACoreTemplateType.h"
#include "tryMirSetUpSwapCurve.h"



/* @brief			swig interface for mirSetUpOISCurve
*  @param [in]		CurveID			The curve collection ID
*  @param [in]		MarketName		Name of the curve constructed by this method
*  @param [in]		YldData			General yield curve data
*  @param [in]		MData			Money market conventions
*  @param [in]		LData			Libor market conventions
*  @param [in]		LGrid			Libor market data
*  @param [in]		SData			Swap market conventions
*  @param [in]		SGrid			Swap market data
*  @param [in]		FRAData			FRA market conventions
*  @param [in]		FRA3MGrid		3M FRA market data
*  @param [in]		FRA6MGrid		6M FRA market data
*  @param [in]		FData			Futures market conventions
*  @param [in]		FGrid			Futures market data
*  @param [in]		AdjData			Basis swaps market conventions
*  @param [in]		AdjGrid			Basis swaps market data
*  @param [in]		CurveNames		Equivalent names of the curve being built
*  @param [in]		CurveName_DF2	Trade discount curve
*/
const std::string mirSetUpSwapCurve(const std::string& CurveID, 
									const std::string& MarketName, 
									const SWIG_STRINGMATRIX & YldData, 
									const SWIG_STRINGMATRIX & MData, 
									const SWIG_STRINGMATRIX & LData, 
									const SWIG_STRINGMATRIX & LGrid, 
									const SWIG_STRINGMATRIX & SData, 
									const SWIG_STRINGMATRIX & SGrid, 
									const SWIG_STRINGMATRIX & FRAData, 
									const SWIG_STRINGMATRIX & FRA3MGrid, 
									const SWIG_STRINGMATRIX & FRA6MGrid, 
									const SWIG_STRINGMATRIX & FData, 
									const SWIG_STRINGMATRIX & FGrid, 
									const SWIG_STRINGMATRIX & AdjData, 
									const SWIG_STRINGMATRIX & AdjGrid, 
									const std::string& CurveNames,
									const std::string& CurveName_DF2) 
{
	LAString ret;
	try 
	{
		// marshall all inputs
		LAString curveID(CurveID.c_str());
		LAString marketName(MarketName.c_str());
		LAString curveNames(CurveNames.c_str());
		LAString curveName_DF2(CurveName_DF2.c_str());

		LAStringMatrix yldData;
		swig::buildStringMatrix(yldData, YldData);

		LAStringMatrix moneyConv;
		swig::buildStringMatrix(moneyConv, MData);

		LAStringMatrix liborConv;
		swig::buildStringMatrix(liborConv, LData);

		LAStringMatrix liborRates;
		swig::buildStringMatrix(liborRates, LGrid);

		LAStringMatrix swapRates;
		swig::buildStringMatrix(swapRates, SGrid);

		LAStringMatrix swapConv;
		swig::buildStringMatrix(swapConv, SData);

		LAStringMatrix fraConv;
		swig::buildStringMatrix(fraConv, FRAData);

		LAStringMatrix fra3mRates;
		swig::buildStringMatrix(fra3mRates, FRA3MGrid);

		LAStringMatrix fra6mRates;
		swig::buildStringMatrix(fra6mRates, FRA6MGrid);

		LAStringMatrix futureConv;
		swig::buildStringMatrix(futureConv, FData);

		LAStringMatrix futureRates;
		swig::buildStringMatrix(futureRates, FGrid);

		LAStringMatrix adjustSwapConv;
		swig::buildStringMatrix(adjustSwapConv, AdjData);

		LAStringMatrix adjustSwapRates;
		swig::buildStringMatrix(adjustSwapRates, AdjGrid);

		ret = validation::tryMirSetUpSwapCurve(etrading::InitializeAQETrading::instance().dataInstance(),
																curveID,
																marketName,
																yldData, 
																moneyConv,
																liborRates, 
																liborConv,
																swapRates, 
																swapConv,
																fra3mRates,
																fra6mRates,
																fraConv,
																futureRates, 
																futureConv,
																adjustSwapConv,
																adjustSwapRates,
																curveNames,
																curveName_DF2);
		
	} 
	catch (LACoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 

	return ret.getCString();

}