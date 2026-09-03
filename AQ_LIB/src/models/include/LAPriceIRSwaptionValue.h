#ifndef LAPriceIRSwaptionValue_h
#define LAPriceIRSwaptionValue_h

#ifdef __GNUG__
#pragma interface
#endif

#include "LALinearRatesOptionValue.h"


class LAObject;
class LARatesPathElementCurve;
class LAPriceDataManager;
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
	virtual LAString			getOptionPayoffName() const;

	// Make copy(clone) of this class
								//======================================
	virtual LACoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%

	// register dataValues that this class uses
	virtual void				registerData(LAPriceDataManager& dm) const;
	
	// calc payoff after maturity
	virtual double				calcPayOffAterMaturity(const LADataValuation& att, LADataProvider* dp, LAObject& e) const;

	//crealte analytic param
	virtual std::vector< std::vector<AnalyticParam*> > createAnalyticParam(LAObject& object, LADataProvider* dp) const;

	//analytic method manager
	virtual std::vector< std::vector<LABlackScholesBase* > > getAnalyticMethod(LAObject& object, LADataProvider* dp) const;

	//payoff method manager
	virtual std::vector< std::vector<LABlackScholesBase* > > getPayoffMethod(LAObject& object, LADataProvider* dp) const;

	//set up analytic param
	virtual void setUpAnalyticParam(LAObject& object, LADataProvider* dp) const;
	//get delivery date
	virtual const LADate& getDeliveryDate(const LAObject& object, LADataProvider* dp) const;
	
	// set up dataProvider
	virtual LADataProvider*					setUpDataProvider(const LADate& basedate, LAObject& object, const LADataValuation& att) const;

	//get nearest tenorstring
	LAString getNearestTenorString(const LAObject& object, const LAString& freq) const;
	
	//get nearest Frequency
	LAString getFrequencyFromIndexGenerator(const LAObject& object, LADataProvider* dp, bool isMonthString = false) const;
	
	// Get AnalyticParam result
	virtual void getAnalyticParamResult(const LAObject& object,
										LADataProvider* dp,
										LAStringVector& names,
										DoubleVector& params
									   ) const;

    virtual double value(const LADate& basedate, LAObject& inst, const LADataValuation& att) const;

    bool hasCashflow(const LAObject& trade) const;
private:
	// create new cache class
	virtual	LADataProvider*			createNewDataProvider() const;
    
    virtual void setUpNumeraireCurrency(const LAObject& trade, LALinearRatesOptionValueDataProvider* dp) const;

	LAStringMatrix getBSComponentMat(LAObject& trade, const LAString& BSFuncType) const;
};
#endif

