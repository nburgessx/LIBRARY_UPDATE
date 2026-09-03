/*! @file
    @brief Class declaration to evaluate trade.

*/
#ifndef AQLLinearRatesSwapTradeValue_h
#define AQLLinearRatesSwapTradeValue_h


#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreValuation.h"
#include "AQLDataValuation.h"
#include "AQLCoreAppError.h"
#include "AQLString.h"
#include "AQLCoreTemplateType.h"
#include "AQLPricePayOff.h"
#include "AQLPriceTradeValue.h"


// AQLLinearRatesSwapTradeValue's function id
#define FN_IR_PLAINVANILLASWAPTRADEVALUE				10049
// AQLLinearRatesSwapTradeValue's function name
#define FN_IR_PLAINVANILLASWAPTRADEVALUE_STR			"fn_ir_plainvanillaswaptradevalue"

#ifndef PRICING_DATA_ISSTUBSETCOMPLETE
#define PRICING_DATA_ISSTUBSETCOMPLETE "IsStubSetComplete"
#endif

#ifndef PRICING_DATA_PV
#define PRICING_DATA_PV "PV"
#endif

//#define __ITM_CHECK__ 
//// FROTOTYPE ////
class AQLDate;
class AQLObject;
class AQLPriceDataManager;
class AQLMathFXEntity;
class AQLMathIndexEntity;
class AQLMathPathEntity;
class AQLRatesNumeraireBase;
class AQLPriceAccruedInterest;
class AQLPolynomialBase;
class AQLDataDoubleMatrix;
class AQLMathPlainVanillaEntity;
class AQLMathYieldCurve;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief class that evaluates trade

*/
class AQLLinearRatesSwapTradeValue : public AQLPriceTradeValue
{
public:
    // constructor	
	explicit AQLLinearRatesSwapTradeValue(AQLPriceAccruedInterest* pacc = 0);
    // destructor	
	virtual ~AQLLinearRatesSwapTradeValue();
	// Check function for this class ID	
	virtual bool                isTypeOf(function_t id) const;
    // Make copy(clone) of this class
	virtual AQLCoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%
    // Return this class type
	virtual function_t			getType() const;
	// register dataValues that this class uses
	virtual void				registerData(AQLPriceDataManager& dm) const;
    // evaluation function
	virtual double              value(const AQLDate& basedate, 
										AQLObject& object,
									const AQLDataValuation& att) const;


	//Remove warning:C4512
	AQLLinearRatesSwapTradeValue & operator=( const AQLLinearRatesSwapTradeValue & ) { return *this; }


	/*!
		@brief cache class for performance up
	*/
	class AQLLinearRatesSwapTradeValueDataProvider : public AQLPriceTradeValueDataProvider
	{
	public:
		virtual ~AQLLinearRatesSwapTradeValueDataProvider(); 
	
		AQLMathPlainVanillaEntity* pVanilla; // plain vanilla object
		AQLString recDCurve;  //rec side discount curve name
		AQLString payDCurve;  //pay side discount curve name
		AQLString reccur;  //rec side currency
		AQLString paycur;  //pay side currency
		bool isannuitycalc; //is annuity calc
		unsigned int annuityLegNo; //is annuityLegNo
		std::map<unsigned int, double> annuitymap;
		bool isdifferentiation;  //differential mode for analytic calculation
		bool isirrmodel;
		double fwdswapterm;    // fwdswap term	

		// for NDS
		BoolVector isnondeliverableLeg; // is the leg non-deliverable or not
		AQLString originalcur; // non-deliverable currency
	};

    bool hasCashflow(const AQLObject& trade) const;
	// calculate fee value
	static double calcFeeValueVanilla(AQLObject &tradeEntity, const AQLString& baseCurrency);

protected:
    // copy constructor
    AQLLinearRatesSwapTradeValue(const AQLLinearRatesSwapTradeValue& v);	

    // evaluation function
	virtual double              value(const AQLDate& basedate, 
										AQLObject& object, AQLDataProvider* dp,
										unsigned int startpathnum = 0) const;


	//// get reference indexs
	//std::set<AQLMathIndexEntity*>
	//							getReferenceIndex(AQLObject& trade) const;
	// set up dataProvider
	AQLDataProvider*					setUpDataProvider(const AQLDate& basedate, AQLObject& object, 
											const AQLDataValuation& att) const;	
	// value by backward
	virtual double				value_backward(const DoubleMatrix& time,
											const DoubleMatrix& cf,
											std::vector<std::pair<unsigned int, AQLDate> >& triggerhit,
											const DoubleArray& rebate,
											const DoubleMatrix& explanatory,
											const AQLDataProvider* dp,
											DoubleVector &callval,
											BoolVector* afterjudge = 0,
											BoolVector* judge = 0,
											DoubleVector* explainedvar = 0) const;
	// get FXEntity
	const AQLMathFXEntity &getFXEntity(AQLObject &object) const;
	// set up for annuity calc
	void						setUpForAnnuityCalc(AQLObject& object, AQLDataProvider* dp) const;
	// set ir analytic risk into grid
	// we stopped giving the analytic risk function on 2012/11/26 for improving performance of the linepricer
	//void						setAnatlyticIRRiskIntoGrid(AQLObject& object, AQLDataProvider* dp, const std::map<AQLString, std::map<double,double> >& dfZero) const;
	
	void setUpStubCoefficient(AQLObject& trade, AQLDataProvider* dp) const;
    
    void doSetUpStubCoefficient(AQLObject& coupon, AQLObject& cashlet, AQLMathPlainVanillaEntity* pvanilla) const;
	// set compounded rate
	void setCompoundedRate(AQLObject& trade, const AQLPricePayOff& payoff) const;
	// remove notional exposure
	void removeNotionalExposure(AQLObject& trade, AQLDataProvider* dp) const;

private:
	// create new cache class
	virtual	AQLDataProvider*			createNewDataProvider() const;
	
    double do_value(const AQLDate& basedate, AQLObject& object, const AQLDataValuation& att) const;
};
#endif
