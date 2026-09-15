%module AlgoQuantLib
%include <std_wstring.i>
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


%{
#include "SwigTypes.h"
#include "exposed_functions.h"	
#include "aqCurveCalibrateBasis.h"
#include "aqCurveCalibrateFXForwards.h"
#include "aqCurveCalibrateOIS.h"
#include "aqCurveCalibrateSwap.h"
#include "aqCurveCompoundRate.h"
#include "aqCurveDiscountFactor.h"
#include "aqCurveDisplay.h"
#include "aqCurveForwardRate.h"
#include "aqCurveZeroRate.h"
#include "aqDate.h"
#include "aqDateCentralBank.h"
#include "aqDateIMM.h"
#include "aqAssetSwapSpread.h"
#include "aqSwapOisParRate.h"
#include "aqSwapOisPV.h"
#include "aqSwapDV01.h"
#include "aqSwapParRate.h"
#include "aqSwapPV.h"
#include "aqSwapPV01.h"
#include "aqSwapSchedule.h"
#include "aqSwapStubRate.h"
#include "aqMathInterpolation.h"
#include "aqMathPCA.h"
#include "aqToolRecord.h"
#include "aqToolSetup.h"
#include "aqCurveInterpolationJoinDate.h"
#include "aqCurveDualBootstrap.h"
#include "aqObject.h"
#include "aqGenerator.h"
#include "aqBondObjectCreate.h"
#include "aqBondObjectPrice.h"
#include "aqToolLVB.h"
#include "aqSwapObjectCreate.h"
#include "aqSwapObjectPrice.h"
#include "BondTypes.h"
#include "aqBondCurve.h"
#include "aqCreditObject.h"
#include "aqCurveResults.h"
#include "aqCurveObject.h"
#include "aqToolGrids.h"
%}


%include "SwigTypes.h"
%include "exposed_functions.h"
%include "aqCurveCalibrateBasis.h"
%include "aqCurveCalibrateFXForwards.h"
%include "aqCurveCalibrateOIS.h"
%include "aqCurveCalibrateSwap.h"
%include "aqCurveCompoundRate.h"
%include "aqCurveDiscountFactor.h"
%include "aqCurveDisplay.h"
%include "aqCurveForwardRate.h"
%include "aqCurveZeroRate.h"
%include "aqDate.h"
%include "aqDateCentralBank.h"
%include "aqDateIMM.h"
%include "aqAssetSwapSpread.h"
%include "aqSwapOisParRate.h"
%include "aqSwapOisPV.h"
%include "aqSwapDV01.h"
%include "aqSwapParRate.h"
%include "aqSwapPV.h"
%include "aqSwapPV01.h"
%include "aqSwapSchedule.h"
%include "aqSwapStubRate.h"
%include "aqMathInterpolation.h"
%include "aqMathPCA.h"
%include "aqToolRecord.h"
%include "aqToolSetup.h"
%include "aqCurveInterpolationJoinDate.h"
%include "aqCurveDualBootstrap.h"
%include "aqObject.h"
%include "aqGenerator.h"
%include "aqBondObjectCreate.h"
%include "aqBondObjectPrice.h"
%include "aqToolLVB.h"
%include "aqSwapObjectCreate.h"
%include "aqSwapObjectPrice.h"
%include "BondTypes.h"
%include "aqBondCurve.h"
%include "aqCreditObject.h"
%include "aqCurveResults.h"
%include "aqCurveObject.h"
%include "aqToolGrids.h"