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

%{
#include "SwigTypes.h"
#include "exposed_functions.h"	
#include "aqCurvesCalibrateBasis.h"
#include "aqCurvesCalibrateFXForwards.h"
#include "aqCurvesCalibrateOIS.h"
#include "aqCurvesCalibrateSwap.h"
#include "aqCurvesCompoundRate.h"
#include "aqCurvesDiscountFactor.h"
#include "aqCurvesDisplay.h"
#include "aqCurvesForwardRate.h"
#include "aqCurvesZeroRate.h"
#include "aqDates.h"
#include "aqDatesCentralBank.h"
#include "aqDatesIMM.h"
#include "aqAssetSwapSpread.h"
#include "aqOISParRate.h"
#include "aqOISPV.h"
#include "aqSwapDV01.h"
#include "aqSwapParRate.h"
#include "aqSwapPV.h"
#include "aqSwapPV01.h"
#include "aqSwapSchedule.h"
#include "aqSwapStubRate.h"
#include "aqToolsInterpolation.h"
#include "aqToolsPCA.h"
#include "aqToolsRecord.h"
#include "aqToolsSetup.h"
#include "aqCurvesInterpolationJoinDate.h"
#include "aqCurvesDualBootstrap.h"
#include "aqObjects.h"
#include "aqObjectsBondCreate.h"
#include "aqObjectsBondPrice.h"
#include "aqToolsLVB.h"
#include "aqObjectsSwapCreate.h"
#include "aqObjectsSwapPrice.h"
#include "BondTypes.h"
#include "aqBondsCurve.h"
#include "aqObjectsCredit.h"
#include "aqCurvesResults.h"
#include "aqObjectsCurve.h"
#include "aqObjectsGrids.h"
%}


%include "SwigTypes.h"
%include "exposed_functions.h"
%include "aqCurvesCalibrateBasis.h"
%include "aqCurvesCalibrateFXForwards.h"
%include "aqCurvesCalibrateOIS.h"
%include "aqCurvesCalibrateSwap.h"
%include "aqCurvesCompoundRate.h"
%include "aqCurvesDiscountFactor.h"
%include "aqCurvesDisplay.h"
%include "aqCurvesForwardRate.h"
%include "aqCurvesZeroRate.h"
%include "aqDates.h"
%include "aqDatesCentralBank.h"
%include "aqDatesIMM.h"
%include "aqAssetSwapSpread.h"
%include "aqOISParRate.h"
%include "aqOISPV.h"
%include "aqSwapDV01.h"
%include "aqSwapParRate.h"
%include "aqSwapPV.h"
%include "aqSwapPV01.h"
%include "aqSwapSchedule.h"
%include "aqSwapStubRate.h"
%include "aqToolsInterpolation.h"
%include "aqToolsPCA.h"
%include "aqToolsRecord.h"
%include "aqToolsSetup.h"
%include "aqCurvesInterpolationJoinDate.h"
%include "aqCurvesDualBootstrap.h"
%include "aqObjects.h"
%include "aqObjectsBondCreate.h"
%include "aqObjectsBondPrice.h"
%include "aqToolsLVB.h"
%include "aqObjectsSwapCreate.h"
%include "aqObjectsSwapPrice.h"
%include "BondTypes.h"
%include "aqBondsCurve.h"
%include "aqObjectsCredit.h"
%include "aqCurvesResults.h"
%include "aqObjectsCurve.h"
%include "aqObjectsGrids.h"