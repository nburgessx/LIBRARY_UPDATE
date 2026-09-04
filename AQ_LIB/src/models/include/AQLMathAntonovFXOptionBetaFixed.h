/*! @file
    @brief Class declaration to AQLMathAntonovFXOption model.
*/

#ifndef AQLMathAntonovFXOptionBetaFixed_h
#define AQLMathAntonovFXOptionBetaFixed_h

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
#define FN_ANTONOVFXOPTIOINBETAFIXED	10045
// Function Name of AQLShiftMethod
#define FN_ANTONOVFXOPTIOINBETAFIXED_STR	"fn_antonovfxoptionbetafixed"


class AQLObject;
class AQLRatesPathElementCurve;
class AQLPriceDataManager;


class AQLMathAntonovFXOptionBetaFixed : public AQLMathAntonovFXOption
{
public:
	// Default constructor
	AQLMathAntonovFXOptionBetaFixed();
	// Destructor
	~AQLMathAntonovFXOptionBetaFixed();
								//======================================
								// Return this class ID
	virtual function_t			getType() const;
								//======================================
								// Make copy(clone) of this class
	virtual AQLCoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%

	
protected:
	// Copy constructor
	AQLMathAntonovFXOptionBetaFixed(const AQLMathAntonovFXOptionBetaFixed& v);
	// virtual method
	virtual void			setCalibParamFirst(AQLMathAntonovFXOptionDataProvider* dataProvider, double vol, double beta) const;
	// virtual method 
	virtual void			setCalibParam(AQLMathAntonovFXOptionDataProvider* dataProvider, double vol, double beta) const;
 private:
	

};
#endif

