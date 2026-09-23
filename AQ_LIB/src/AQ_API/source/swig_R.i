%module AlgoQuantLib
// %include <std_wstring.i>
%include <std_string.i>
%include <std_vector.i>
%include <cpointer.i>
%include <carrays.i>
%include <cdata.i>
%include <cstring.i>
%include <exception.i>

%exception
{
  try
  {
    $action
  }
  catch (const std::exception& e)
  {
    SWIG_exception(SWIG_RuntimeError, e.what());
  }
}

namespace std
{
	%template(VecInteger)		vector<int>;
	%template(VecDouble)		vector<double>;
	%template(VecString)		vector<string>;

	%template(VecVecInteger)	vector< vector<int> >;
	%template(VecVecDouble)		vector< vector<double> >;
	%template(VecVecString)		vector< vector<string> >;
}

// -----------------------------------------------------------
// SWIG HEADER BLOCK
//
// Headers to verbatim copied into the swig wrapper file *.cxx
// Includes here are INSIDE the swig verbatim scope operators %{ ... }% 
// Includes have the '#' prefix
//
// -----------------------------------------------------------

%{

// -----------------------------------------------------------
//  Support Methods (These must come first!)
//
//  These must come first -- SwigTypes.h #defines SWIG_STRINGMATRIX,
//  which many other headers below use as a type; being a macro, it has no
//  forward declaration and must be textually #included first.
//
// -----------------------------------------------------------
#include "exposed_functions.h"
#include "SwigTypes.h"

//  Category: AssetSwap
// --------------------
#include "aqAssetSwapObject.h"
#include "aqAssetSwapSpread.h"

//  Category: Bond
// ---------------
#include "aqBondCurve.h"
#include "aqBondObjectCreate.h"
#include "aqBondObjectFRN.h"
#include "aqBondObjectFuture.h"
#include "aqBondObjectPrice.h"
#include "aqBondObjectRisk.h"
#include "aqBondSchedule.h"
#include "BondTypes.h"

//  Category: BondFutureOption
// ---------------------------
#include "aqBondFutureOptionObject.h"

//  Category: BondOption
// ---------------------
#include "aqBondOptionObject.h"

//  Category: CapFloor
// -------------------
#include "aqCapFloorObject.h"

//  Category: CMS
// --------------
#include "aqCMSObject.h"

//  Category: Credit
// -----------------
#include "aqCDSObject.h"
#include "aqCreditObject.h"

//  Category: Curve
// ----------------
#include "aqCurveCalibrateBasis.h"
#include "aqCurveCalibrateFXForwards.h"
#include "aqCurveCalibrateOIS.h"
#include "aqCurveCalibrateSwap.h"
#include "aqCurveCompoundRate.h"
#include "aqCurveDiscountFactor.h"
#include "aqCurveDisplay.h"
#include "aqCurveDualBootstrap.h"
#include "aqCurveForwardRate.h"
#include "aqCurveInterpolationJoinDate.h"
#include "aqCurveObject.h"
#include "aqCurveResults.h"
#include "aqCurveZeroRate.h"

//  Category: Date
// ---------------
#include "aqDate.h"
#include "aqDateCentralBank.h"
#include "aqDateIMM.h"

//  Category: Future
// -----------------
#include "aqFutureTicker.h"

//  Category: FX
// -------------
#include "aqFXObject.h"

//  Category: Generator
// --------------------
#include "aqGenerator.h"

//  Category: Inflation
// --------------------
#include "aqInflationObject.h"

//  Category: IR
// -------------
#include "aqIRObject.h"

//  Category: Math
// ---------------
#include "aqMathBlackScholes.h"
#include "aqMathCapletFloorlet.h"
#include "aqMathConvexity.h"
#include "aqMathInterpolation.h"
#include "aqMathNumerical.h"
#include "aqMathPCA.h"
#include "aqMathRates.h"
#include "aqMathStatistics.h"
#include "aqMathSwaption.h"
#include "aqMathVolatilityConversion.h"

//  Category: Model
// ----------------

//  Category: Object
// -----------------
#include "aqObject.h"

//  Category: Swap
// ---------------
#include "aqSwapDelta.h"
#include "aqSwapDV01.h"
#include "aqSwapLeg.h"
#include "aqSwapObjectCreate.h"
#include "aqSwapObjectPrice.h"
#include "aqSwapOisParRate.h"
#include "aqSwapOisPV.h"
#include "aqSwapParRate.h"
#include "aqSwapPV.h"
#include "aqSwapPV01.h"
#include "aqSwapResults.h"
#include "aqSwapSchedule.h"
#include "aqSwapStubRate.h"

//  Category: Swaption
// -------------------
#include "aqSwaptionObject.h"

//  Category: Tool
// ---------------
#include "aqGridObject.h"
#include "aqToolData.h"
#include "aqToolDate.h"
#include "aqToolDiagnostics.h"
#include "aqToolGrids.h"
#include "aqToolLVB.h"
#include "aqToolRecord.h"
#include "aqToolSetup.h"

//  Category: TRS
// --------------

//  Category: Volatility
// ---------------------
#include "aqVolatilityObject.h"
#include "aqVolatilitySABR.h"
%}

// -----------------------------------------------------------
// SWIG SOURCE BLOCK (or Parse Block)
//
// Functions here are parsed into swig wrapper file *.cxx
// Includes here are OUTSIDE the swig verbatim scope operators %{ ... }% 
// Includes have '%' prefix
//
// -----------------------------------------------------------

// -----------------------------------------------------------
//  Support Methods (These must come first!)
//
//  These must come first -- SwigTypes.h #defines SWIG_STRINGMATRIX,
//  which many other headers below use as a type; being a macro, it has no
//  forward declaration and must be textually #included first.
//
// -----------------------------------------------------------
%include "exposed_functions.h"
%include "SwigTypes.h"

//  Category: AssetSwap
// --------------------
%include "aqAssetSwapObject.h"
%include "aqAssetSwapSpread.h"

//  Category: Bond
// ---------------
%include "aqBondCurve.h"
%include "aqBondObjectCreate.h"
%include "aqBondObjectFRN.h"
%include "aqBondObjectFuture.h"
%include "aqBondObjectPrice.h"
%include "aqBondObjectRisk.h"
%include "aqBondSchedule.h"
%include "BondTypes.h"

//  Category: BondFutureOption
// ---------------------------
%include "aqBondFutureOptionObject.h"

//  Category: BondOption
// ---------------------
%include "aqBondOptionObject.h"

//  Category: CapFloor
// -------------------
%include "aqCapFloorObject.h"

//  Category: CMS
// --------------
%include "aqCMSObject.h"

//  Category: Credit
// -----------------
%include "aqCDSObject.h"
%include "aqCreditObject.h"

//  Category: Curve
// ----------------
%include "aqCurveCalibrateBasis.h"
%include "aqCurveCalibrateFXForwards.h"
%include "aqCurveCalibrateOIS.h"
%include "aqCurveCalibrateSwap.h"
%include "aqCurveCompoundRate.h"
%include "aqCurveDiscountFactor.h"
%include "aqCurveDisplay.h"
%include "aqCurveDualBootstrap.h"
%include "aqCurveForwardRate.h"
%include "aqCurveInterpolationJoinDate.h"
%include "aqCurveObject.h"
%include "aqCurveResults.h"
%include "aqCurveZeroRate.h"

//  Category: Date
// ---------------
%include "aqDate.h"
%include "aqDateCentralBank.h"
%include "aqDateIMM.h"

//  Category: Future
// -----------------
%include "aqFutureTicker.h"

//  Category: FX
// -------------
%include "aqFXObject.h"

//  Category: Generator
// --------------------
%include "aqGenerator.h"

//  Category: Inflation
// --------------------
%include "aqInflationObject.h"

//  Category: IR
// -------------
%include "aqIRObject.h"

//  Category: Math
// ---------------
%include "aqMathBlackScholes.h"
%include "aqMathCapletFloorlet.h"
%include "aqMathConvexity.h"
%include "aqMathInterpolation.h"
%include "aqMathNumerical.h"
%include "aqMathPCA.h"
%include "aqMathRates.h"
%include "aqMathStatistics.h"
%include "aqMathSwaption.h"
%include "aqMathVolatilityConversion.h"

//  Category: Model
// ----------------

//  Category: Object
// -----------------
%include "aqObject.h"

//  Category: Swap
// ---------------
%include "aqSwapDelta.h"
%include "aqSwapDV01.h"
%include "aqSwapLeg.h"
%include "aqSwapObjectCreate.h"
%include "aqSwapObjectPrice.h"
%include "aqSwapOisParRate.h"
%include "aqSwapOisPV.h"
%include "aqSwapParRate.h"
%include "aqSwapPV.h"
%include "aqSwapPV01.h"
%include "aqSwapResults.h"
%include "aqSwapSchedule.h"
%include "aqSwapStubRate.h"

//  Category: Swaption
// -------------------
%include "aqSwaptionObject.h"

//  Category: Tool
// ---------------
%include "aqGridObject.h"
%include "aqToolData.h"
%include "aqToolDate.h"
%include "aqToolDiagnostics.h"
%include "aqToolGrids.h"
%include "aqToolLVB.h"
%include "aqToolRecord.h"
%include "aqToolSetup.h"

//  Category: TRS
// --------------

//  Category: Volatility
// ---------------------
%include "aqVolatilityObject.h"
%include "aqVolatilitySABR.h"