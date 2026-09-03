#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LALinearRatesOptionValue.h"
#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLBasic.h"
#include "AQLDist.h"
#include "AQLDataValuation.h"
#include "AQLFindRootBrent.h"
#include "AQLGaussLegendre.h"

#include "LAAnalyticFormula.h"
#include "LABlackScholesCalc.h"
#include "LAMathVolFuncFXStrangleSolver.h"


class AQLObject;
class LARatesPathElementCurve;
class AQLPriceDataManager;
class LABlackScholesBase;
class LAMathYieldCurve;
class LAMathPlainVanillaEntity;


class LAPriceFXOptionValue : public LALinearRatesOptionValue
{
public:
	// Default constructor
	LAPriceFXOptionValue();
	// Destructor
	~LAPriceFXOptionValue();
								//======================================
								// Return this class ID
	virtual function_t			getType() const;
								//======================================
								// Check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
								//hishida vannavolga
								//======================================
								// Return option method name
	virtual AQLString			getOptionPayoffName() const;

								// Make copy(clone) of this class
								//======================================
	virtual AQLCoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%

	// register dataValues that this class uses
	virtual void				registerData(AQLPriceDataManager& dm) const;
	
	// calc option
	virtual double				calcOption(const AQLDataValuation& att, AQLDataProvider* dp, AQLObject& e) const;
	virtual double				calcFXDigitalCallSpreadOption(const AQLDataValuation& att, AQLDataProvider* dp, AQLObject& e) const;
	
	// calc payoff after maturity
	virtual double				calcPayOffAterMaturity(const AQLDataValuation& att, AQLDataProvider* dp, AQLObject& e) const;
	virtual double				calcPayOffFXDigitalCallSpreadAterMaturity(const AQLDataValuation& att, AQLDataProvider* dp, AQLObject& e) const;			
	
	// adjust val for anlytical risk
	virtual double				adjustForAnalyticalRisk(const AQLDataValuation& att, AQLDataProvider* dp, AQLObject& e, AnalyticParam& param) const;


	//crealte analytic param
	virtual std::vector< std::vector<AnalyticParam*> > createAnalyticParam(AQLObject& object, AQLDataProvider* dp) const;
	
	//set up analytic param
	virtual void setUpAnalyticParam(AQLObject& object, AQLDataProvider* dp) const;

	// set up dataProvider
	virtual AQLDataProvider*					setUpDataProvider(const AQLDate& basedate, AQLObject& object, 
											const AQLDataValuation& att) const;
	//get additional premium
	virtual double getAdditionalPremium(const AQLObject& object, AQLDataProvider* dp) const;

private:
	// create new cache class
	virtual	AQLDataProvider*			createNewDataProvider() const;
};
