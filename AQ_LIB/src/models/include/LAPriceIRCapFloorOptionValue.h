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

#include "LAAnalyticFormula.h"
#include "LABlackScholesCalc.h"


class LAObject;
class LARatesPathElementCurve;
class LAPriceDataManager;
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
	virtual LAString			getOptionPayoffName() const;
								// Make copy(clone) of this class
								//======================================
	virtual LACoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%

	// register dataValues that this class uses
	virtual void				registerData(LAPriceDataManager& dm) const;
	
	// calc option
	virtual double				calcOption(const LADataValuation& att, LADataProvider* dp, LAObject& e) const;
	// calc payoff after maturity
	virtual double				calcPayOffAterMaturity(const LADataValuation& att, LADataProvider* dp, LAObject& e) const;

	//analytic method manager
	virtual std::vector< std::vector<LABlackScholesBase* > > getAnalyticMethod(LAObject& object, LADataProvider* dp) const;
	//crealte analytic param
	virtual std::vector< std::vector<AnalyticParam*> > createAnalyticParam(LAObject& object, LADataProvider* dp) const;
	
	//set up analytic param
	virtual void setUpAnalyticParam(LAObject& object, LADataProvider* dp) const;
	//get cashlet size
	virtual unsigned int getCashletSize(const LAObject& object, LADataProvider* dp) const;

	// set up dataProvider
	virtual LADataProvider*					setUpDataProvider(const LADate& basedate, LAObject& object, const LADataValuation& att) const;
	//get maturity date
	virtual const LADate& getMaturityDate(const LAObject& object, LADataProvider* dp) const;
	//get delivery date
	virtual const LADate& getDeliveryDate(const LAObject& object, LADataProvider* dp) const;
	//multiple unit
	virtual double multipleUnit(const LAObject& object, LADataProvider* dp) const;
	
	virtual void outputResult( LAObject& object, LADataProvider* dp ) const;

    bool hasCashflow(const LAObject& trade) const;

    double value(const LADate& basedate, LAObject& inst, const LADataValuation& att) const;
private:
	// create new cache class
	virtual	LADataProvider*			createNewDataProvider() const;
    virtual void setUpNumeraireCurrency(const LAObject& trade, LALinearRatesOptionValueDataProvider* dp) const;
};

