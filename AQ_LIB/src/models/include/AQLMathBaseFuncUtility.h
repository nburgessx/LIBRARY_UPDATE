#ifndef AQLMathBaseFuncUtility_h
#define AQLMathBaseFuncUtility_h

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLDataInstance.h"
#include "AQLString.h"
#include "ConstantDeclarations.h"
#include "AQLTime.h"

class AQLMathBaseFuncUtility
{
public:
	//option unit
	static void adjustunit(double& ret, AQLString& optiontype);

private:
	AQLMathBaseFuncUtility(void);
	~AQLMathBaseFuncUtility(void);
	AQLMathBaseFuncUtility(const AQLMathBaseFuncUtility &rhs);
	AQLMathBaseFuncUtility &operator=(const AQLMathBaseFuncUtility &rhs);
};
#endif 
