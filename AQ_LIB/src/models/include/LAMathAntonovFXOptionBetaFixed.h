/*! @file
    @brief Class declaration to LAMathAntonovFXOption model.

*/
//  2008, AlgoQuantHub.

#ifndef LAMathAntonovFXOptionBetaFixed_h
#define LAMathAntonovFXOptionBetaFixed_h

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
#define FN_ANTONOVFXOPTIOINBETAFIXED	10045
// Function Name of AQLShiftMethod
#define FN_ANTONOVFXOPTIOINBETAFIXED_STR	"fn_antonovfxoptionbetafixed"


class AQLObject;
class LARatesPathElementCurve;
class AQLPriceDataManager;


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
	virtual AQLCoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%

	
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

