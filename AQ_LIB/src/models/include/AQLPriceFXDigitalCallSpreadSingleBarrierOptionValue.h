#ifndef AQLPriceFXDigitalCallSpreadSingleBarrierOptionValue_h
#define AQLPriceFXDigitalCallSpreadSingleBarrierOptionValue_h

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLPriceFXSingleBarrierOptionValue.h"
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


class AQLPriceFXDigitalCallSpreadSingleBarrierOptionValue : public AQLPriceFXSingleBarrierOptionValue
{
public:
	// Default constructor
	AQLPriceFXDigitalCallSpreadSingleBarrierOptionValue();
	// Destructor
	~AQLPriceFXDigitalCallSpreadSingleBarrierOptionValue();
								//======================================
								// Return this class ID
	virtual function_t			getType() const;
								//======================================
								// Check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
								//hishida vannavolga
								//======================================
								// Return option funcname
	virtual AQLString			getOptionPayoffName() const;
								//======================================
								// Make copy(clone) of this class
	virtual AQLCoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%

	// register dataValues that this class uses
	virtual void				registerData(AQLPriceDataManager& dm) const;
	// calc option
	virtual double				calcOption(const AQLDataValuation& att, AQLDataProvider* dataProvider, AQLObject& e) const;
	// calc payoff after maturity
	virtual double				calcPayOffAterMaturity(const AQLDataValuation& att, AQLDataProvider* dataProvider, AQLObject& e) const;

	//crealte analytic param
	virtual std::vector< std::vector<AnalyticParam*> > createAnalyticParam(AQLObject& object, AQLDataProvider* dataProvider) const;
	
	
	// set up dataProvider
	virtual AQLDataProvider*					setUpDataProvider(const AQLDate& basedate, AQLObject& object, 
											const AQLDataValuation& att) const;
private:
	
};
#endif

