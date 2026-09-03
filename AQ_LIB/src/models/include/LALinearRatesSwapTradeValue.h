/*! @file
    @brief Class declaration to evaluate trade.

*/
#ifndef LALinearRatesSwapTradeValue_h
#define LALinearRatesSwapTradeValue_h


#ifdef __GNUG__
#pragma interface
#endif

#include "LACoreValuation.h"
#include "LADataValuation.h"
#include "LACoreAppError.h"
#include "LAString.h"
#include "LACoreTemplateType.h"
#include "LAPricePayOff.h"
#include "LAPriceTradeValue.h"


// LALinearRatesSwapTradeValue's function id
#define FN_IR_PLAINVANILLASWAPTRADEVALUE				10049
// LALinearRatesSwapTradeValue's function name
#define FN_IR_PLAINVANILLASWAPTRADEVALUE_STR			"fn_ir_plainvanillaswaptradevalue"

#ifndef PRICING_DATA_ISSTUBSETCOMPLETE
#define PRICING_DATA_ISSTUBSETCOMPLETE "IsStubSetComplete"
#endif

#ifndef PRICING_DATA_PV
#define PRICING_DATA_PV "PV"
#endif

//#define __ITM_CHECK__ 
//// FROTOTYPE ////
class LADate;
class LAObject;
class LAPriceDataManager;
class LAMathFXEntity;
class LAMathIndexEntity;
class LAMathPathEntity;
class LARatesNumeraireBase;
class LAPriceAccruedInterest;
class LAPolynomialBase;
class LADataDoubleMatrix;
class LAMathPlainVanillaEntity;
class LAMathYieldCurve;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief class that evaluates trade

*/
class LALinearRatesSwapTradeValue : public LAPriceTradeValue
{
public:
    // constructor	
	explicit LALinearRatesSwapTradeValue(LAPriceAccruedInterest* pacc = 0);
    // destructor	
	virtual ~LALinearRatesSwapTradeValue();
	// Check function for this class ID	
	virtual bool                isTypeOf(function_t id) const;
    // Make copy(clone) of this class
	virtual LACoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%
    // Return this class type
	virtual function_t			getType() const;
	// register dataValues that this class uses
	virtual void				registerData(LAPriceDataManager& dm) const;
    // evaluation function
	virtual double              value(const LADate& basedate, 
										LAObject& object,
									const LADataValuation& att) const;


	//Remove warning:C4512
	LALinearRatesSwapTradeValue & operator=( const LALinearRatesSwapTradeValue & ) { return *this; }


	/*!
		@brief cache class for performance up
	*/
	class LALinearRatesSwapTradeValueDataProvider : public LAPriceTradeValueDataProvider
	{
	public:
		virtual ~LALinearRatesSwapTradeValueDataProvider(); 
	
		LAMathPlainVanillaEntity* pVanilla; // plain vanilla object
		LAString recDCurve;  //rec side discount curve name
		LAString payDCurve;  //pay side discount curve name
		LAString reccur;  //rec side currency
		LAString paycur;  //pay side currency
		bool isannuitycalc; //is annuity calc
		unsigned int annuityLegNo; //is annuityLegNo
		std::map<unsigned int, double> annuitymap;
		bool isdifferentiation;  //differential mode for analytic calculation
		bool isirrmodel;
		double fwdswapterm;    // fwdswap term	

		// for NDS
		BoolVector isnondeliverableLeg; // is the leg non-deliverable or not
		LAString originalcur; // non-deliverable currency
	};

    bool hasCashflow(const LAObject& trade) const;
	// calculate fee value
	static double calcFeeValueVanilla(LAObject &tradeEntity, const LAString& baseCurrency);

protected:
    // copy constructor
    LALinearRatesSwapTradeValue(const LALinearRatesSwapTradeValue& v);	

    // evaluation function
	virtual double              value(const LADate& basedate, 
										LAObject& object, LADataProvider* dp,
										unsigned int startpathnum = 0) const;


	//// get reference indexs
	//std::set<LAMathIndexEntity*>
	//							getReferenceIndex(LAObject& trade) const;
	// set up dataProvider
	LADataProvider*					setUpDataProvider(const LADate& basedate, LAObject& object, 
											const LADataValuation& att) const;	
	// value by backward
	virtual double				value_backward(const DoubleMatrix& time,
											const DoubleMatrix& cf,
											std::vector<std::pair<unsigned int, LADate> >& triggerhit,
											const DoubleArray& rebate,
											const DoubleMatrix& explanatory,
											const LADataProvider* dp,
											DoubleVector &callval,
											BoolVector* afterjudge = 0,
											BoolVector* judge = 0,
											DoubleVector* explainedvar = 0) const;
	// get FXEntity
	const LAMathFXEntity &getFXEntity(LAObject &object) const;
	// set up for annuity calc
	void						setUpForAnnuityCalc(LAObject& object, LADataProvider* dp) const;
	// set ir analytic risk into grid
	// we stopped giving the analytic risk function on 2012/11/26 for improving performance of the linepricer
	//void						setAnatlyticIRRiskIntoGrid(LAObject& object, LADataProvider* dp, const std::map<LAString, std::map<double,double> >& dfZero) const;
	
	void setUpStubCoefficient(LAObject& trade, LADataProvider* dp) const;
    
    void doSetUpStubCoefficient(LAObject& coupon, LAObject& cashlet, LAMathPlainVanillaEntity* pvanilla) const;
	// set compounded rate
	void setCompoundedRate(LAObject& trade, const LAPricePayOff& payoff) const;
	// remove notional exposure
	void removeNotionalExposure(LAObject& trade, LADataProvider* dp) const;

private:
	// create new cache class
	virtual	LADataProvider*			createNewDataProvider() const;
	
    double do_value(const LADate& basedate, LAObject& object, const LADataValuation& att) const;
};
#endif
