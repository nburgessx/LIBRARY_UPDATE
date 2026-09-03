#ifndef LAMathBaseFuncUtility_h
#define LAMathBaseFuncUtility_h

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLDataInstance.h"
#include "AQLString.h"
#include "ConstantDeclarations.h"
#include "LATime.h"

class LAMathBaseFuncUtility
{
public:
	//option unit
	static void adjustunit(double& ret, AQLString& optiontype);

private:
	LAMathBaseFuncUtility(void);
	~LAMathBaseFuncUtility(void);
	LAMathBaseFuncUtility(const LAMathBaseFuncUtility &rhs);
	LAMathBaseFuncUtility &operator=(const LAMathBaseFuncUtility &rhs);
};
#endif 
