#ifndef AQLPriceFXKnockoutRebateValue_h
#define AQLPriceFXKnockoutRebateValue_h

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLPriceFXOptionValue.h"
#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLBasic.h"
#include "AQLDist.h"
#include "AQLDataValuation.h"
#include "AQLFindRootBrent.h"
#include "AQLGaussLegendre.h"

#include "AQLAnalyticFormula.h"
#include "AQLBlackScholesCalc.h"
#include "AQLMathVolFuncFXStrangleSolver.h"


class AQLObject;
class AQLRatesPathElementCurve;
class AQLPriceDataManager;
class AQLBlackScholesBase;
class AQLMathYieldCurve;
class AQLMathPlainVanillaEntity;


class AQLPriceFXKnockoutRebateValue : public AQLPriceFXOptionValue
{
public:
	// Default constructor
	AQLPriceFXKnockoutRebateValue();
	// Destructor
	~AQLPriceFXKnockoutRebateValue();
								//======================================
								// Return this class ID
	virtual function_t			getType() const;
								//======================================
								// Check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
								// vanna-volga
								//======================================
								// Return option funcname
	virtual AQLString			getOptionPayoffName() const;
								// Make copy(clone) of this class
								//======================================
	virtual AQLCoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%

	// register dataValues that this class uses
	virtual void				registerData(AQLPriceDataManager& dm) const;

	// calc option
	virtual double				calcOption(const AQLDataValuation& att, AQLDataProvider* dp, AQLObject& e) const;

	//crealte analytic param
	virtual std::vector< std::vector<AnalyticParam*> > createAnalyticParam(AQLObject& object, AQLDataProvider* dp) const;
	
	//set up analytic param
	virtual void setUpAnalyticParam(AQLObject& object, AQLDataProvider* dp) const;

	//// set up dataProvider
	virtual AQLDataProvider*					setUpDataProvider(const AQLDate& basedate, AQLObject& object, const AQLDataValuation& att) const;

private:
	// create new cache class
	virtual	AQLDataProvider*			createNewDataProvider() const;
};
#endif

