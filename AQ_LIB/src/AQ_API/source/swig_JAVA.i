%module AlgoQuantLib
%include <std_wstring.i>
%include <std_string.i>
%include <std_vector.i>
%include <cpointer.i>
%include <carrays.i>
%include <cdata.i>
%include <cstring.i>
%include <exception.i>
%include <arrays_java.i>;


%typemap(throws, throws="java.lang.Exception") std::exception {
  jclass excep = jenv->FindClass("java/io/IOException");
  if (excep)
    jenv->ThrowNew(excep, $1.what());
  return $null; 
}
%typemap(javabase) std::exception "java.lang.Exception";


%typemap(throws, throws="java.lang.IllegalArgumentException") std::invalid_argument {
  jclass excep = jenv->FindClass("java/lang/IllegalArgumentException");
  if (excep)
    jenv->ThrowNew(excep, $1.what());
  return $null;
}
%typemap(javabase) std::invalid_argument "java.lang.Exception";


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
#include "aqCMSObject.h"
#include "aqAssetSwapObject.h"
#include "aqIRObject.h"
#include "aqBondOptionObject.h"
#include "aqInflationObject.h"
#include "aqSwaptionObject.h"
#include "aqCapFloorObject.h"
#include "aqFXObject.h"
#include "aqVolatilityObject.h"
#include "aqVolatilitySABR.h"
#include "aqBondFutureOptionObject.h"
#include "aqFutureTicker.h"
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
%include "aqCMSObject.h"
%include "aqAssetSwapObject.h"
%include "aqIRObject.h"
%include "aqBondOptionObject.h"
%include "aqInflationObject.h"
%include "aqSwaptionObject.h"
%include "aqCapFloorObject.h"
%include "aqFXObject.h"
%include "aqVolatilityObject.h"
%include "aqVolatilitySABR.h"
%include "aqBondFutureOptionObject.h"
%include "aqFutureTicker.h"
%include "aqCurveResults.h"
%include "aqCurveObject.h"
%include "aqToolGrids.h"