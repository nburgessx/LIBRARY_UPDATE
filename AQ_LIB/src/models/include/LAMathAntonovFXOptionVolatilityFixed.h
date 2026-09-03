/*! @file
    @brief Class declaration to LAMathAntonovFXOption model.

*/
//  2008, AlgoQuantHub.

#ifndef LAMathAntonovFXOptionVolatilityFixed_h
#define LAMathAntonovFXOptionVolatilityFixed_h

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreValuation.h"
#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLBasic.h"
#include "AQLDist.h"
#include "AQLDataValuation.h"
#include "AQLFindRootBrent.h"
#include "AQL1DDataSet.h"
#include "AQLGaussLegendre.h"
#include "LAMathAntonovFXOption.h"

#include "LAAnalyticFormula.h"
#include "LABlackScholesCalc.h"



// Funciton ID of AQLShiftMethod
#define FN_ANTONOVFXOPTIOINVOLATILITYFIXED	10046
// Function Name of AQLShiftMethod
#define FN_ANTONOVFXOPTIOINVOLATILITYFIXED_STR	"fn_antonovfxoptionvolatilityfixed"


class AQLObject;
class LARatesPathElementCurve;
class AQLPriceDataManager;


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
	virtual AQLCoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%

	
protected:
	// Copy constructor
	LAMathAntonovFXOptionVolatilityFixed(const LAMathAntonovFXOptionVolatilityFixed& v);

	 virtual void			setCalibParamFirst(LAMathAntonovFXOptionDataProvider* dataProvider, double vol, double beta) const;
	 virtual void			setCalibParam(LAMathAntonovFXOptionDataProvider* dataProvider, double vol, double beta) const;
 private:
	

};
#endif

