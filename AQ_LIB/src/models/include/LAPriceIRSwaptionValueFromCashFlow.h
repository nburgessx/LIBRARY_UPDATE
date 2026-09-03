#ifndef LAPriceIRSwaptionValueFromCashFlow_h
#define LAPriceIRSwaptionValueFromCashFlow_h

#ifdef __GNUG__
#pragma interface
#endif

#include "LAPriceIRSwaptionValue.h"


class LAObject;
class LARatesPathElementCurve;
class LAPriceDataManager;
class LABlackScholesBase;
class LAMathYieldCurve;
class LAMathPlainVanillaEntity;

const double eps_NegativeFLibor = 0.000001;

class LAPriceIRSwaptionValueFromCashFlow : public LAPriceIRSwaptionValue
{
public:
	// Default constructor
	LAPriceIRSwaptionValueFromCashFlow();
	// Destructor
	~LAPriceIRSwaptionValueFromCashFlow();
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

	//set up analytic param
	virtual void setUpAnalyticParam(LAObject& object, LADataProvider* dp) const;
	
	// set up dataProvider
	virtual LADataProvider*					setUpDataProvider(const LADate& basedate, LAObject& object, const LADataValuation& att) const;
	
	//get nearest Frequency
	LAString getFrequencyFromIndexGenerator(const LAObject& object, unsigned int floatLegNo, bool isMonthString = false) const;

	//get Libor Object
	const LAObject& getLiborEntity(const LAObject& object, unsigned int floatLegNo) const;

	//get float leg number
	unsigned int getFloatLegNum(const LAObject& object) const;

	//getCalcDataFromCashFlow
	double getCalcDataFromCashFlow(const LAObject& object, LADataProvider* dp, double& frate, double& annuity, double& equivstrike, double& avenotional) const;
	
	virtual void outputResult( LAObject& object, LADataProvider* dp ) const;

private:
};
#endif

