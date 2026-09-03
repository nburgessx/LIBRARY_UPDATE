#ifndef LAMathBaseFuncUtility_h
#define LAMathBaseFuncUtility_h

#ifdef __GNUG__
#pragma interface
#endif

#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include "LADataInstance.h"
#include "LAString.h"
#include "ConstantDeclarations.h"
#include "LATime.h"

class LAMathBaseFuncUtility
{
public:
	//option unit
	static void adjustunit(double& ret, LAString& optiontype);

private:
	LAMathBaseFuncUtility(void);
	~LAMathBaseFuncUtility(void);
	LAMathBaseFuncUtility(const LAMathBaseFuncUtility &rhs);
	LAMathBaseFuncUtility &operator=(const LAMathBaseFuncUtility &rhs);
};
#endif 
