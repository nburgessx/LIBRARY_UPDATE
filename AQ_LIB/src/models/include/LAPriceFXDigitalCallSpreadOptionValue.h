#ifndef LAPriceFXDigitalCallSpreadOptionValue_h
#define LAPriceFXDigitalCallSpreadOptionValue_h

#ifdef __GNUG__
#pragma interface
#endif

#include "LAPriceFXOptionValue.h"
#include "LAFunctionBase.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include "LABasic.h"
#include "LADist.h"
#include "LADataValuation.h"
#include "LAFindRootBrent.h"
#include "LAGaussLegendre.h"

#include "LAAnalyticFormula.h"
#include "LABlackScholesCalc.h"
#include "LAMathVolFuncFXStrangleSolver.h"


class LAObject;
class LARatesPathElementCurve;
class LAPriceDataManager;
class LABlackScholesBase;
class LAMathYieldCurve;
class LAMathPlainVanillaEntity;


class LAPriceFXDigitalCallSpreadOptionValue : public LAPriceFXOptionValue
{
public:
	// Default constructor
	LAPriceFXDigitalCallSpreadOptionValue();
	// Destructor
	~LAPriceFXDigitalCallSpreadOptionValue();
								//======================================
								// Return this class ID
	virtual function_t			getType() const;
								//======================================
								// Check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
								//hishida vannavolga
								//======================================
								// Return option funcname
	virtual LAString			getOptionPayoffName() const;
								//======================================
								// Make copy(clone) of this class
	virtual LACoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%

	// register dataValues that this class uses
	virtual void				registerData(LAPriceDataManager& dm) const;

	// calc option
	virtual double				calcOption(const LADataValuation& att, LADataProvider* dataProvider, LAObject& e) const;
	// calc payoff after maturity
	virtual double				calcPayOffAterMaturity(const LADataValuation& att, LADataProvider* dataProvider, LAObject& e) const;

	//crealte analytic param
	virtual std::vector< std::vector<AnalyticParam*> > createAnalyticParam(LAObject& object, LADataProvider* dataProvider) const;
	
	// set up dataProvider
	virtual LADataProvider*					setUpDataProvider(const LADate& basedate, LAObject& object, 
											const LADataValuation& att) const;
	
	
private:
	
};
#endif

