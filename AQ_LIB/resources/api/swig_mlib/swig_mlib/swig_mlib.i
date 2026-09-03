%module SwigMLIB
%include <std_wstring.i>
%include <std_string.i>
%include <std_vector.i>

namespace std {
  %template(VecInteger) vector<int>;
  %template(VecVecInteger) vector< vector<int> >;
  %template(VecDouble) vector<double>;
  %template(VecVecDouble) vector< vector<double> >;
  %template(VecString) vector<string>;
  %template(VecVecString) vector< vector<string> >;
}
%include <exception.i>
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

%include <arrays_java.i>;


%{
#include "exposed_functions.h"	
#include "mirSetUpOISCurve.h"	
#include "mirSetUpSwapCurve.h"	
#include "mirSetUpBasisSwapCurve.h"	
#include "mirGetDateFromTerm.h"	
#include "mirGetDate.h"	
#include "mirGetIMMDate.h"	
/* #include "mirGetParRate.h"	*/
/* #include "mirOISParRate.h"	*/
/* #include "mirOISSwapPV.h"  */
/* #include "mirSwapPV.h"	 */
#include "mirGetDF.h"	
#include "mirGetPCAResults.h"	
#include "mirSetupPCA.h"	
#include "mirInterpolation.h"	
#include "mirGetForwardRate.h"	
#include "mirSetUpFwdFXConstantCurve.h"	
/* #include "mirSwapStubRate.h"	*/
#include "mirGetZeroRate.h"
#include "mirOutputCurve.h"	
/* #include "mirSwapPV01.h"	*/
#include "mirCompound.h"	
#include "mirGetAnnuity.h"	
#include "mirGetBusinessDays.h"	
#include "mirGetECBStartDate.h"	
#include "mirGetNextCBDate.h"	
#include "mirGetNextECBDate.h"	
#include "mirGetTerm.h"	
#include "mirSwapSchedule.h"	
%}



%include "exposed_functions.h";
%include "mirSetUpOISCurve.h";
%include "mirSetUpSwapCurve.h";
%include "mirSetUpBasisSwapCurve.h";
%include "mirGetDateFromTerm.h";
%include "mirGetDate.h";
%include "mirGetIMMDate.h";
/* %include "mirGetParRate.h"; */
/* %include "mirOISParRate.h"	*/
/* %include "mirOISSwapPV.h"	 */
/* %include "mirSwapPV.h"	 */
%include "mirGetDF.h"	
%include "mirGetPCAResults.h"	
%include "mirSetupPCA.h"	
%include "mirInterpolation.h"	
%include "mirGetForwardRate.h"	
%include "mirSetUpFwdFXConstantCurve.h"	
/* %include "mirSwapStubRate.h"	*/
%include "mirGetZeroRate.h"
%include "mirOutputCurve.h"	
/* %include "mirSwapPV01.h"	*/
%include "mirCompound.h"	
%include "mirGetAnnuity.h"	
%include "mirGetBusinessDays.h"	
%include "mirGetECBStartDate.h"	
%include "mirGetNextCBDate.h"	
%include "mirGetNextECBDate.h"	
%include "mirGetTerm.h"	
%include "mirSwapSchedule.h"	
