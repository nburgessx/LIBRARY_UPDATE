/*! @file
    @brief Class declaration to AQLMathAntonovFXOption model.
*/

#ifndef AQLMathAntonovFXOptionVolatilityFixed_h
#define AQLMathAntonovFXOptionVolatilityFixed_h

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
#include "AQLMathAntonovFXOption.h"

#include "AQLAnalyticFormula.h"
#include "AQLBlackScholesCalc.h"



// Funciton ID of AQLShiftMethod
#define FN_ANTONOVFXOPTIOINVOLATILITYFIXED	10046
// Function Name of AQLShiftMethod
#define FN_ANTONOVFXOPTIOINVOLATILITYFIXED_STR	"fn_antonovfxoptionvolatilityfixed"


class AQLObject;
class AQLRatesPathElementCurve;
class AQLPriceDataManager;


class AQLMathAntonovFXOptionVolatilityFixed : public AQLMathAntonovFXOption
{
public:
	// Default constructor
	AQLMathAntonovFXOptionVolatilityFixed();
	// Destructor
	~AQLMathAntonovFXOptionVolatilityFixed();
								//======================================
								// Return this class ID
	virtual function_t			getType() const;
								//======================================
								// Make copy(clone) of this class
	virtual AQLCoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%

	
protected:
	// Copy constructor
	AQLMathAntonovFXOptionVolatilityFixed(const AQLMathAntonovFXOptionVolatilityFixed& v);

	 virtual void			setCalibParamFirst(AQLMathAntonovFXOptionDataProvider* dataProvider, double vol, double beta) const;
	 virtual void			setCalibParam(AQLMathAntonovFXOptionDataProvider* dataProvider, double vol, double beta) const;
 private:
	

};
#endif

