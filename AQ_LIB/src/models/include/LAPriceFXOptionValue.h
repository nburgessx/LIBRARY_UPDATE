#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LALinearRatesOptionValue.h"
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
	virtual LAString			getOptionPayoffName() const;

								// Make copy(clone) of this class
								//======================================
	virtual LACoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%

	// register dataValues that this class uses
	virtual void				registerData(LAPriceDataManager& dm) const;
	
	// calc option
	virtual double				calcOption(const LADataValuation& att, LADataProvider* dp, LAObject& e) const;
	virtual double				calcFXDigitalCallSpreadOption(const LADataValuation& att, LADataProvider* dp, LAObject& e) const;
	
	// calc payoff after maturity
	virtual double				calcPayOffAterMaturity(const LADataValuation& att, LADataProvider* dp, LAObject& e) const;
	virtual double				calcPayOffFXDigitalCallSpreadAterMaturity(const LADataValuation& att, LADataProvider* dp, LAObject& e) const;			
	
	// adjust val for anlytical risk
	virtual double				adjustForAnalyticalRisk(const LADataValuation& att, LADataProvider* dp, LAObject& e, AnalyticParam& param) const;


	//crealte analytic param
	virtual std::vector< std::vector<AnalyticParam*> > createAnalyticParam(LAObject& object, LADataProvider* dp) const;
	
	//set up analytic param
	virtual void setUpAnalyticParam(LAObject& object, LADataProvider* dp) const;

	// set up dataProvider
	virtual LADataProvider*					setUpDataProvider(const LADate& basedate, LAObject& object, 
											const LADataValuation& att) const;
	//get additional premium
	virtual double getAdditionalPremium(const LAObject& object, LADataProvider* dp) const;

private:
	// create new cache class
	virtual	LADataProvider*			createNewDataProvider() const;
};
