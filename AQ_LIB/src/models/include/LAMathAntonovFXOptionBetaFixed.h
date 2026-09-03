/*! @file
    @brief Class declaration to LAMathAntonovFXOption model.

*/
//  2008, AlgoQuantHub.

#ifndef LAMathAntonovFXOptionBetaFixed_h
#define LAMathAntonovFXOptionBetaFixed_h

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
#define FN_ANTONOVFXOPTIOINBETAFIXED	10045
// Function Name of LAShiftMethod
#define FN_ANTONOVFXOPTIOINBETAFIXED_STR	"fn_antonovfxoptionbetafixed"


class LAObject;
class LARatesPathElementCurve;
class LAPriceDataManager;


class LAMathAntonovFXOptionBetaFixed : public LAMathAntonovFXOption
{
public:
	// Default constructor
	LAMathAntonovFXOptionBetaFixed();
	// Destructor
	~LAMathAntonovFXOptionBetaFixed();
								//======================================
								// Return this class ID
	virtual function_t			getType() const;
								//======================================
								// Make copy(clone) of this class
	virtual LACoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%

	
protected:
	// Copy constructor
	LAMathAntonovFXOptionBetaFixed(const LAMathAntonovFXOptionBetaFixed& v);
	// virtual method
	virtual void			setCalibParamFirst(LAMathAntonovFXOptionDataProvider* dataProvider, double vol, double beta) const;
	// virtual method 
	virtual void			setCalibParam(LAMathAntonovFXOptionDataProvider* dataProvider, double vol, double beta) const;
 private:
	

};
#endif

