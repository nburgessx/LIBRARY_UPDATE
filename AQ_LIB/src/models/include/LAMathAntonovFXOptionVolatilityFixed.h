/*! @file
    @brief Class declaration to LAMathAntonovFXOption model.

*/
//  2008, Mizuho International London.

#ifndef LAMathAntonovFXOptionVolatilityFixed_h
#define LAMathAntonovFXOptionVolatilityFixed_h

#ifdef __GNUG__
#pragma interface
#endif

#include "LACoreValuation.h"
#include "LAFunctionBase.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include "LABasic.h"
#include "LADist.h"
#include "LADataValuation.h"
#include "LAFindRootBrent.h"
#include "LA1DDataSet.h"
#include "LAGaussLegendre.h"
#include "LAMathAntonovFXOption.h"

#include "LAAnalyticFormula.h"
#include "LABlackScholesCalc.h"



// Funciton ID of LAShiftMethod
#define FN_ANTONOVFXOPTIOINVOLATILITYFIXED	10046
// Function Name of LAShiftMethod
#define FN_ANTONOVFXOPTIOINVOLATILITYFIXED_STR	"fn_antonovfxoptionvolatilityfixed"


class LAObject;
class LARatesPathElementCurve;
class LAPriceDataManager;


class LAMathAntonovFXOptionVolatilityFixed : public LAMathAntonovFXOption
{
public:
	// Default constructor
	LAMathAntonovFXOptionVolatilityFixed();
	// Destructor
	~LAMathAntonovFXOptionVolatilityFixed();
								//======================================
								// Return this class ID
	virtual function_t			getType() const;
								//======================================
								// Make copy(clone) of this class
	virtual LACoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%

	
protected:
	// Copy constructor
	LAMathAntonovFXOptionVolatilityFixed(const LAMathAntonovFXOptionVolatilityFixed& v);

	 virtual void			setCalibParamFirst(LAMathAntonovFXOptionDataProvider* dataProvider, double vol, double beta) const;
	 virtual void			setCalibParam(LAMathAntonovFXOptionDataProvider* dataProvider, double vol, double beta) const;
 private:
	

};
#endif

