#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LAMathBaseFuncUtility.h"

////////////////////////////AlgoQuantLib-Userfunc
void 
LAMathBaseFuncUtility::adjustunit(double& ret, LAString& optiontype)
{
	if(VEGA == optiontype)
		ret*= 0.01;
	else if(THETA == optiontype)
		ret/= 365;
	else if(RHO == optiontype)
		ret*= 0.01;
	else if(PHI == optiontype)
		ret*= 0.01;
	else if(VOLGA == optiontype)
		ret*= 0.01;
};		
