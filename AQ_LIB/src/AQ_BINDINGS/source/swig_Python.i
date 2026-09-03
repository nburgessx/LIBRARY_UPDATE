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
#include "mirSetUpOISCurve.h"	
#include "mirSetUpSwapCurve.h"	
#include "mirSetUpBasisSwapCurve.h"	
#include "mirGetDateFromTerm.h"	
#include "mirGetDate.h"	
#include "mirGetIMMDate.h"	
#include "mirGetParRate.h"	
#include "mirOISParRate.h"	
#include "mirOISSwapPV.h"	
#include "mirSwapPV.h"	
#include "mirGetDF.h"	
#include "mirGetPCAResults.h"	
#include "mirSetupPCA.h"	
#include "mirInterpolation.h"	
#include "mirGetForwardRate.h"	
#include "mirSetUpFwdFXConstantCurve.h"	
#include "mirSwapStubRate.h"	
#include "mirGetZeroRate.h"
#include "mirOutputCurve.h"	
#include "mirSwapPV01.h"	
#include "mirCompound.h"	
#include "mirGetAnnuity.h"	
#include "mirGetBusinessDays.h"	
#include "mirGetECBStartDate.h"	
#include "mirGetNextCBDate.h"	
#include "mirGetNextECBDate.h"	
#include "mirGetTerm.h"	
#include "mirSwapSchedule.h"	
#include "mirClearEntityPool.h"
#include "meCurveCalibrateBasis.h"
#include "meCurveCalibrateFXForwards.h"
#include "meCurveCalibrateOIS.h"
#include "meCurveCalibrateSwap.h"
#include "meCurveCompoundRate.h"
#include "meCurveDiscountFactor.h"
#include "meCurveDisplay.h"
#include "meCurveForwardRate.h"
#include "meCurveZeroRate.h"
#include "meDate.h"
#include "meDateCentralBank.h"
#include "meDateIMM.h"
#include "meProductAssetSwapSpread.h"
#include "meProductOISParRate.h"
#include "meProductOISPV.h"
#include "meProductSwapDV01.h"
#include "meProductSwapParRate.h"
#include "meProductSwapPV.h"
#include "meProductSwapPV01.h"
#include "meProductSwapSchedule.h"
#include "meProductSwapStubRate.h"
#include "meUtilityInterpolation.h"
#include "meUtilityPCA.h"
#include "meUtilityRecord.h"
#include "meUtilitySetup.h"
#include "meCurveInterpolationJoinDate.h"
#include "meCurveDualBootstrap.h"
#include "meLWO.h"
#include "meLWOBondCreate.h"
#include "meLWOBondPrice.h"
#include "meUtilityLVB.h"
#include "meLWOSwapCreate.h"
#include "meLWOSwapPrice.h"
#include "BondTypes.h"
#include "meBondCurve.h"
#include "meLWOCredit.h"
#include "meCurveResults.h"
#include "meLWOCurve.h"
#include "meLWOGrids.h"
%}


%include "SwigTypes.h"
%include "exposed_functions.h"
%include "mirSetUpOISCurve.h"
%include "mirSetUpSwapCurve.h"
%include "mirSetUpBasisSwapCurve.h"
%include "mirGetDateFromTerm.h"
%include "mirGetDate.h"
%include "mirGetIMMDate.h"
%include "mirGetParRate.h"
%include "mirOISParRate.h"	
%include "mirOISSwapPV.h"	
%include "mirSwapPV.h"	
%include "mirGetDF.h"	
%include "mirGetPCAResults.h"	
%include "mirSetupPCA.h"	
%include "mirInterpolation.h"	
%include "mirGetForwardRate.h"	
%include "mirSetUpFwdFXConstantCurve.h"	
%include "mirSwapStubRate.h"	
%include "mirGetZeroRate.h"
%include "mirOutputCurve.h"	
%include "mirSwapPV01.h"	
%include "mirCompound.h"	
%include "mirGetAnnuity.h"	
%include "mirGetBusinessDays.h"	
%include "mirGetECBStartDate.h"	
%include "mirGetNextCBDate.h"	
%include "mirGetNextECBDate.h"	
%include "mirGetTerm.h"	
%include "mirSwapSchedule.h"	
%include "mirClearEntityPool.h"
%include "meCurveCalibrateBasis.h"
%include "meCurveCalibrateFXForwards.h"
%include "meCurveCalibrateOIS.h"
%include "meCurveCalibrateSwap.h"
%include "meCurveCompoundRate.h"
%include "meCurveDiscountFactor.h"
%include "meCurveDisplay.h"
%include "meCurveForwardRate.h"
%include "meCurveZeroRate.h"
%include "meDate.h"
%include "meDateCentralBank.h"
%include "meDateIMM.h"
%include "meProductAssetSwapSpread.h"
%include "meProductOISParRate.h"
%include "meProductOISPV.h"
%include "meProductSwapDV01.h"
%include "meProductSwapParRate.h"
%include "meProductSwapPV.h"
%include "meProductSwapPV01.h"
%include "meProductSwapSchedule.h"
%include "meProductSwapStubRate.h"
%include "meUtilityInterpolation.h"
%include "meUtilityPCA.h"
%include "meUtilityRecord.h"
%include "meUtilitySetup.h"
%include "meCurveInterpolationJoinDate.h"
%include "meCurveDualBootstrap.h"
%include "meLWO.h"
%include "meLWOBondCreate.h"
%include "meLWOBondPrice.h"
%include "meUtilityLVB.h"
%include "meLWOSwapCreate.h"
%include "meLWOSwapPrice.h"
%include "BondTypes.h"
%include "meBondCurve.h"
%include "meLWOCredit.h"
%include "meCurveResults.h"
%include "meLWOCurve.h"
%include "meLWOGrids.h"