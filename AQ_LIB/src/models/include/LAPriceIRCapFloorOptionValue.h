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

#include "LAAnalyticFormula.h"
#include "LABlackScholesCalc.h"


class AQLObject;
class LARatesPathElementCurve;
class AQLPriceDataManager;
class LABlackScholesBase;
class LAMathYieldCurve;
class LAMathPlainVanillaEntity;


class LAPriceIRCapFloorOptionValue : public LALinearRatesOptionValue
{
public:
	// Default constructor
	LAPriceIRCapFloorOptionValue();
	// Destructor
	~LAPriceIRCapFloorOptionValue();
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
	virtual std::vector< std::vector<LABlackScholesBase* > > getAnalyticMethod(AQLObject& object, AQLDataProvider* dp) const;
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
    virtual void setUpNumeraireCurrency(const AQLObject& trade, LALinearRatesOptionValueDataProvider* dp) const;
};

