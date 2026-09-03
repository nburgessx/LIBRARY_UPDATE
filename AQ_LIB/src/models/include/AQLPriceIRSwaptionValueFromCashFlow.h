#ifndef AQLPriceIRSwaptionValueFromCashFlow_h
#define AQLPriceIRSwaptionValueFromCashFlow_h

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLPriceIRSwaptionValue.h"


class AQLObject;
class AQLRatesPathElementCurve;
class AQLPriceDataManager;
class AQLBlackScholesBase;
class AQLMathYieldCurve;
class AQLMathPlainVanillaEntity;

const double eps_NegativeFLibor = 0.000001;

class AQLPriceIRSwaptionValueFromCashFlow : public AQLPriceIRSwaptionValue
{
public:
	// Default constructor
	AQLPriceIRSwaptionValueFromCashFlow();
	// Destructor
	~AQLPriceIRSwaptionValueFromCashFlow();
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

	//set up analytic param
	virtual void setUpAnalyticParam(AQLObject& object, AQLDataProvider* dp) const;
	
	// set up dataProvider
	virtual AQLDataProvider*					setUpDataProvider(const AQLDate& basedate, AQLObject& object, const AQLDataValuation& att) const;
	
	//get nearest Frequency
	AQLString getFrequencyFromIndexGenerator(const AQLObject& object, unsigned int floatLegNo, bool isMonthString = false) const;

	//get Libor Object
	const AQLObject& getLiborEntity(const AQLObject& object, unsigned int floatLegNo) const;

	//get float leg number
	unsigned int getFloatLegNum(const AQLObject& object) const;

	//getCalcDataFromCashFlow
	double getCalcDataFromCashFlow(const AQLObject& object, AQLDataProvider* dp, double& frate, double& annuity, double& equivstrike, double& avenotional) const;
	
	virtual void outputResult( AQLObject& object, AQLDataProvider* dp ) const;

private:
};
#endif

