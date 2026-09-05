#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLLinearRatesOptionValue.h"
#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLBasic.h"
#include "AQLDist.h"
#include "AQLDataValuation.h"
#include "AQLFindRootBrent.h"

#include "AQLAnalyticFormula.h"
#include "AQLBlackScholesCalc.h"


class AQLObject;
class AQLRatesPathElementCurve;
class AQLPriceDataManager;
class AQLBlackScholesBase;
class AQLMathYieldCurve;
class AQLMathPlainVanillaEntity;


class AQLPriceIRCapFloorOptionValue : public AQLLinearRatesOptionValue
{
public:
	// Default constructor
	AQLPriceIRCapFloorOptionValue();
	// Destructor
	~AQLPriceIRCapFloorOptionValue();
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
	// calc payoff after maturity
	virtual double				calcPayOffAterMaturity(const AQLDataValuation& att, AQLDataProvider* dp, AQLObject& e) const;

	//analytic method manager
	virtual std::vector< std::vector<AQLBlackScholesBase* > > getAnalyticMethod(AQLObject& object, AQLDataProvider* dp) const;
	//crealte analytic param
	virtual std::vector< std::vector<AnalyticParam*> > createAnalyticParam(AQLObject& object, AQLDataProvider* dp) const;
	
	//set up analytic param
	virtual void setUpAnalyticParam(AQLObject& object, AQLDataProvider* dp) const;
	//get cashlet size
	virtual unsigned int getCashletSize(const AQLObject& object, AQLDataProvider* dp) const;

	// set up dataProvider
	virtual AQLDataProvider*					setUpDataProvider(const AQLDate& basedate, AQLObject& object, const AQLDataValuation& att) const;
	//get maturity date
	virtual const AQLDate& getMaturityDate(const AQLObject& object, AQLDataProvider* dp) const;
	//get delivery date
	virtual const AQLDate& getDeliveryDate(const AQLObject& object, AQLDataProvider* dp) const;
	//multiple unit
	virtual double multipleUnit(const AQLObject& object, AQLDataProvider* dp) const;
	
	virtual void outputResult( AQLObject& object, AQLDataProvider* dp ) const;

    bool hasCashflow(const AQLObject& trade) const;

    double value(const AQLDate& basedate, AQLObject& inst, const AQLDataValuation& att) const;
private:
	// create new cache class
	virtual	AQLDataProvider*			createNewDataProvider() const;
    virtual void setUpNumeraireCurrency(const AQLObject& trade, AQLLinearRatesOptionValueDataProvider* dp) const;
};

