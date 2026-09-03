#ifndef LAPriceIRSwaptionValue_h
#define LAPriceIRSwaptionValue_h

#ifdef __GNUG__
#pragma interface
#endif

#include "LALinearRatesOptionValue.h"


class AQLObject;
class LARatesPathElementCurve;
class AQLPriceDataManager;
class LABlackScholesBase;
class LAMathYieldCurve;
class LAMathPlainVanillaEntity;


class LAPriceIRSwaptionValue : public LALinearRatesOptionValue
{
public:
	// Default constructor
	LAPriceIRSwaptionValue();
	// Destructor
	~LAPriceIRSwaptionValue();
								
								//======================================
								// Return this class ID
	virtual function_t			getType() const;
								//======================================
								
								// Check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
								//======================================
	
								// Return option funcname
	virtual AQLString			getOptionPayoffName() const;

	// Make copy(clone) of this class
								//======================================
	virtual AQLCoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%

	// register dataValues that this class uses
	virtual void				registerData(AQLPriceDataManager& dm) const;
	
	// calc payoff after maturity
	virtual double				calcPayOffAterMaturity(const AQLDataValuation& att, AQLDataProvider* dp, AQLObject& e) const;

	//crealte analytic param
	virtual std::vector< std::vector<AnalyticParam*> > createAnalyticParam(AQLObject& object, AQLDataProvider* dp) const;

	//analytic method manager
	virtual std::vector< std::vector<LABlackScholesBase* > > getAnalyticMethod(AQLObject& object, AQLDataProvider* dp) const;

	//payoff method manager
	virtual std::vector< std::vector<LABlackScholesBase* > > getPayoffMethod(AQLObject& object, AQLDataProvider* dp) const;

	//set up analytic param
	virtual void setUpAnalyticParam(AQLObject& object, AQLDataProvider* dp) const;
	//get delivery date
	virtual const AQLDate& getDeliveryDate(const AQLObject& object, AQLDataProvider* dp) const;
	
	// set up dataProvider
	virtual AQLDataProvider*					setUpDataProvider(const AQLDate& basedate, AQLObject& object, const AQLDataValuation& att) const;

	//get nearest tenorstring
	AQLString getNearestTenorString(const AQLObject& object, const AQLString& freq) const;
	
	//get nearest Frequency
	AQLString getFrequencyFromIndexGenerator(const AQLObject& object, AQLDataProvider* dp, bool isMonthString = false) const;
	
	// Get AnalyticParam result
	virtual void getAnalyticParamResult(const AQLObject& object,
										AQLDataProvider* dp,
										AQLStringVector& names,
										DoubleVector& params
									   ) const;

    virtual double value(const AQLDate& basedate, AQLObject& inst, const AQLDataValuation& att) const;

    bool hasCashflow(const AQLObject& trade) const;
private:
	// create new cache class
	virtual	AQLDataProvider*			createNewDataProvider() const;
    
    virtual void setUpNumeraireCurrency(const AQLObject& trade, LALinearRatesOptionValueDataProvider* dp) const;

	AQLStringMatrix getBSComponentMat(AQLObject& trade, const AQLString& BSFuncType) const;
};
#endif

