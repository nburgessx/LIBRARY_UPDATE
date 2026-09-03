#pragma once

#ifdef __GNUG__
#pragma interface
#endif


#include "AQLCoreValuation.h"
#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLBasic.h"
#include "AQLDist.h"
#include "AQLDataValuation.h"
#include "AQLFindRootBrent.h"
#include "AQLGaussLegendre.h"
#include "LAAnalyticFormula.h"
#include "LABlackScholesCalc.h"



#ifndef PRICING_DATA_TRADEDATE
#define PRICING_DATA_TRADEDATE				"TradeDate"				//  data name of TradeDate
#endif
#ifndef PRICING_DATA_EXPIRYDATE
#define PRICING_DATA_EXPIRYDATE				"ExpiryDate"				//  data name of ExpiryDate
#endif
#ifndef PRICING_DATA_DELIVERYDATE
#define PRICING_DATA_DELIVERYDATE			"DeliveryDate"				//  data name of DeliveryDate
#endif
#ifndef PRICING_DATA_VALUEDATE
#define PRICING_DATA_VALUEDATE				"ValueDate"				//  data name of ValueDate
#endif
#ifndef PRICING_DATA_OPTIONSTATUS
#define PRICING_DATA_OPTIONSTATUS					"OptionStatus"				//  data name of OptionStatus
#endif
#ifndef PRICING_DATA_PREMIUMCURRENCY
#define PRICING_DATA_PREMIUMCURRENCY		"PremiumCurrency"				//  data name of PremiumCurrency
#endif
#ifndef PRICING_DATA_DOMESTICCURRENCY
#define PRICING_DATA_DOMESTICCURRENCY		"DomesticCurrency"				//  data name of DomesticCurrency
#endif
#ifndef PRICING_DATA_FOREIGNCURRENCY
#define PRICING_DATA_FOREIGNCURRENCY		"ForeignCurrency"				//  data name of ForeignCurrency
#endif
#ifndef PRICING_DATA_BUYSELL
#define PRICING_DATA_BUYSELL				"BuySell"				//  data name of BuySell
#endif
#ifndef PRICING_DATA_OPTIONTYPE
#define PRICING_DATA_OPTIONTYPE				"OptionType"				//  data name of OptionType
#endif
#ifndef PRICING_DATA_DOMESTICQUANTITY
#define PRICING_DATA_DOMESTICQUANTITY		"DomesticQuantity"				//  data name of DomesticQuantity
#endif
#ifndef PRICING_DATA_FOREIGNQUANTITY
#define PRICING_DATA_FOREIGNQUANTITY		"ForeignQuantity"				//  data name of ForeignQuantity
#endif
#ifndef PRICING_DATA_STRIKE
#define PRICING_DATA_STRIKE					"Strike"						//  data name of Strike
#endif
#ifndef PRICING_DATA_MARKETPARAM
#define PRICING_DATA_MARKETPARAM			"MarketParam"					//  data name of MarketParam
#endif
#ifndef PRICING_DATA_CACHECURRENCY
#define PRICING_DATA_CACHECURRENCY			"CacheCurrency"					//  data name of CacheCurrency
#endif
#ifndef PRICING_DATA_ANALYTICRISKTYPE
#define PRICING_DATA_ANALYTICRISKTYPE		"AnalyticRiskType"					//  data name of AnalyticRiskType
#endif
#ifndef PRICING_DATA_DIGITALPAYOFF
#define PRICING_DATA_DIGITALPAYOFF		"DigitalPayOff"					//  data name of DigitalPayOff
#endif
#ifndef PRICING_DATA_DIGITALSPREAD
#define PRICING_DATA_DIGITALSPREAD		"DigitalSpread"					//  data name of DigitalSpread
#endif
#ifndef PRICING_DATA_DIGITALCOUPON
#define PRICING_DATA_DIGITALCOUPON		"DigitalCoupon"					//  data name of DigitalCoupon
#endif
#ifndef PRICING_DATA_VOLATILITYDIRECTINPUT
#define PRICING_DATA_VOLATILITYDIRECTINPUT		"VolatilityDirectInput"		//  data name of VolatilityDirectInput
#endif
#ifndef PRICING_DATA_VOLATILITYDIRECTINPUTS
#define PRICING_DATA_VOLATILITYDIRECTINPUTS		"VolatilityDirectInputs"		//  data name of VolatilityDirectInputs
#endif
#ifndef PRICING_DATA_REBATE
#define PRICING_DATA_REBATE						"Rebate"					//  data name of Rebate
#endif
#ifndef PRICING_DATA_LIMITVAL
#define PRICING_DATA_LIMITVAL					"LimitVal"		//  data name of LimitVal
#endif
#ifndef PRICING_DATA_MARGIN
#define PRICING_DATA_MARGIN						"Margin"		//  data name of Margin
#endif
#ifndef PRICING_DATA_UPANDDOWN
#define PRICING_DATA_UPANDDOWN					"UpAndDown"		//  data name of UpAndDown
#endif
#ifndef PRICING_DATA_INANDOUT
#define PRICING_DATA_INANDOUT					"InAndOut"		//  data name of InAndOut
#endif
#ifndef PRICING_DATA_VOLATILITYRESULTOFPREMIUM
#define PRICING_DATA_VOLATILITYRESULTOFPREMIUM	 "VolatilityResultOfPremium"		//  data name of VolatilityResultOfPremium
#endif
#ifndef PRICING_DATA_VOLATILITYRESULTOFRISK
#define PRICING_DATA_VOLATILITYRESULTOFRISK		 "VolatilityResultOfRisk"		//  data name of VolatilityResultOfRisk
#endif
#ifndef PRICING_DATA_ISDIFFFORRISK
#define PRICING_DATA_ISDIFFFORRISK					"IsDiffForRisk"		//  data name of IsDiffForRisk
#endif
#ifndef PRICING_DATA_SHIFTVALFORRISK
#define PRICING_DATA_SHIFTVALFORRISK				"ShiftValForRisk"		//  data name of ShiftValForRisk
#endif
#ifndef PRICING_DATA_TERMFORRISK
#define PRICING_DATA_TERMFORRISK				"TermForRisk"		//  data name of TermForRisk
#endif
#ifndef PRICING_DATA_ISLIBORDISCOUNTFORFXOPTION
#define PRICING_DATA_ISLIBORDISCOUNTFORFXOPTION		"IsLiborDiscountForFXOption"		//  data name of IsLiborDiscountForFXOption
#endif
#ifndef PRICING_DATA_BUYSELLDISTINGUISH
#define PRICING_DATA_BUYSELLDISTINGUISH				"BuySellDistinguish"		//  data name of BuySellDistinguish
#endif
#ifndef PRICING_DATA_REBATEPAYMENTTIME
#define PRICING_DATA_REBATEPAYMENTTIME				"RebatePaymentTime"		//  data name of RebatePaymentTime
#endif
#ifndef PRICING_DATA_REBATEPAYMENTCURRENCY
#define PRICING_DATA_REBATEPAYMENTCURRENCY			"RebatePaymentCurrency"		//  data name of RebatePaymentCurrency
#endif
#ifndef PRICING_DATA_REBATEPAYMENTOFFSET
#define PRICING_DATA_REBATEPAYMENTOFFSET			"RebatePaymentOffSet"		//  data name of RebatePaymentOffSet
#endif
#ifndef PRICING_DATA_REBATEPAYMENTCALENDAR
#define PRICING_DATA_REBATEPAYMENTCALENDAR			"RebatePaymentCalendar"		//  data name of RebatePaymentCalendar
#endif
#ifndef PRICING_DATA_ISCALCPAYOFFAFTERMATURITY
#define PRICING_DATA_ISCALCPAYOFFAFTERMATURITY		"IsCalcPayOffAfterMaturity"		//  data name of IsCalcPayOffAfterMaturity
#endif
#ifndef PRICING_DATA_PAYOUT
#define PRICING_DATA_PAYOUT							"PayOut"		//  data name of PayOut
#endif
#ifndef DATA_M_MV_IR_TENOR
#define DATA_M_MV_IR_TENOR							"Tenor"		//  data name of Tenor
#endif
#ifndef PRICING_DATA_INDEXGENERATOR
#define PRICING_DATA_INDEXGENERATOR					"IndexGenerator"		//  data name of IndexGenerator
#endif
#ifndef PRICING_DATA_FIXEDLEGDAYCOUNT
#define PRICING_DATA_FIXEDLEGDAYCOUNT					"FixedLegDayCount"		//  data name of FixedLegDayCount
#endif
#ifndef PRICING_DATA_FIXEDLEGCALENDAR
#define PRICING_DATA_FIXEDLEGCALENDAR					"FixedLegCalendar"		//  data name of FixedLegCalendar
#endif
#ifndef PRICING_DATA_ANALYTICPARAMNAME
#define PRICING_DATA_ANALYTICPARAMNAME				"AnaliticParamName"						//  data name of Forward
#endif
#ifndef PRICING_DATA_ANALYTICPARAM
#define PRICING_DATA_ANALYTICPARAM					"AnalyticParam"						//  data name of Forward
#endif
#ifndef PRICING_DATA_RESULTTRADENAMES
#define PRICING_DATA_RESULTTRADENAMES					"ResultTradeNames"						//  data name of Forward
#endif
#ifndef PRICING_DATA_RESULTRISKNAMES
#define PRICING_DATA_RESULTRISKNAMES					"ResultRiskNames"						//  data name of Forward
#endif
#ifndef PRICING_DATA_RESULTPVS
#define PRICING_DATA_RESULTPVS							"ResultPVs"						//  data name of Forward
#endif
#ifndef PRICING_DATA_RESULTPVCURRENCIES
#define PRICING_DATA_RESULTPVCURRENCIES					"ResultPVCurrencies"						//  data name of Forward
#endif
#ifndef PRICING_DATA_ISCALCEQUIVALENTSTRIKE
#define PRICING_DATA_ISCALCEQUIVALENTSTRIKE					"IsCalcEquivalentStrike"						//  data name of IsCalcEquivalentStrike
#endif
#ifndef PRICING_DATA_ORIGINALLIBORSPREADNAMES
#define PRICING_DATA_ORIGINALLIBORSPREADNAMES					"OriginalLiborSpreadNames"						//  data name of OriginalLiborSpreadNames
#endif
#ifndef PRICING_DATA_ORIGINALLIBORSPREADS
#define PRICING_DATA_ORIGINALLIBORSPREADS					"OriginalLiborSpreads"						//  data name of OriginalLiborSpreads
#endif
#ifndef PRICING_DATA_ORIGINALNOTIONALNAMES
#define PRICING_DATA_ORIGINALNOTIONALNAMES					"OriginalNotionalNames"						//  data name of OriginalNotionalsNames
#endif
#ifndef PRICING_DATA_ORIGINALNOTIONALS
#define PRICING_DATA_ORIGINALNOTIONALS					"OriginalNotionals"						//  data name of OriginalNotionals
#endif
#ifndef PRICING_DATA_AVERAGENOTIONAL
#define PRICING_DATA_AVERAGENOTIONAL					"AverageNotional"						//  data name of AverageNotional
#endif
#ifndef PRICING_DATA_ZEROCALC
#define PRICING_DATA_ZEROCALC            "ZeroCalc"
#endif
#ifndef PRICING_DATA_ISPVVOLUSE
#define PRICING_DATA_ISPVVOLUSE            "IsPVVolUse"	//  data name of IsPVVolUse
#endif
#ifndef PRICING_DATA_PVVOLMATRIX
#define PRICING_DATA_PVVOLMATRIX            "PVVolMatrix"	//  data name of PVVolMatrix
#endif
#ifndef PRICING_DATA_ISIRRMODEL
#define PRICING_DATA_ISIRRMODEL					"IsIRRModel"						//  data name of IsIRRModel
#endif
#ifndef PRICING_DATA_PREMIUMAMOUT
#define PRICING_DATA_PREMIUMAMOUT					"PremiumAmount"						//  data name of PremiumAmount
#endif
#ifndef PRICING_DATA_PREMIUMPAYMENTDATE
#define PRICING_DATA_PREMIUMPAYMENTDATE					"PremiumPaymentDate"			//  data name of PremiumPaymentDate
#endif
#ifndef PRICING_DATA_PREMIUMPAYCURRENCY
#define PRICING_DATA_PREMIUMPAYCURRENCY					"PremiumPaymentCurrency"		//  data name of PremiumPaymentCurrency
#endif
#ifndef PRICING_DATA_ISCLEAREDPHYSICALSETTLE
#define PRICING_DATA_ISCLEAREDPHYSICALSETTLE			"IsClearedPhysicalSettle"		//  data name of IsClearedPhysicalSettle
#endif
#ifndef PRICING_DATA_ISADDFWDPREMPV
#define PRICING_DATA_ISADDFWDPREMPV	"IsAddFwdPremPV"	 // data name of flag for adding FwdPremium into PV
#endif
#ifndef PRICING_DATA_CASHSETTLEMENTAMOUNT
#define PRICING_DATA_CASHSETTLEMENTAMOUNT	"CashSettlementAmount"	 // data name of CashSettlementAmount
#endif
#ifndef PRICING_DATA_CASHSETTLEMENTPAYMENTDATE
#define PRICING_DATA_CASHSETTLEMENTPAYMENTDATE					"CashSettlementPaymentDate"			//  data name of CashSettlementPaymentDate
#endif
#ifndef PRICING_DATA_CASHSETTLEMENTCURRENCY
#define PRICING_DATA_CASHSETTLEMENTCURRENCY		"CashSettlementCurrency"				//  data name of CashSettlementCurrency
#endif










class AQLObject;
class LARatesPathElementCurve;
class AQLPriceDataManager;
class LABlackScholesBase;
class LAMathYieldCurve;
class LAMathPlainVanillaEntity;
class LALinearRatesOptionValueDataProvider;


class LALinearRatesOptionValue : public AQLCoreValuation
{
public:
	// Default constructor
	LALinearRatesOptionValue();
	// Destructor
	~LALinearRatesOptionValue();
								//======================================
								// Return this class ID
	virtual function_t			getType() const;
								//======================================
								// Check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
								//hishida vannavolga
								//======================================
								// Return option method name 
	virtual AQLString			getOptionPayoffName() const;
								// Make copy(clone) of this class
								//======================================
	virtual AQLCoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%

	// register dataValues that this class uses
	virtual void				registerData(AQLPriceDataManager& dm) const;

	virtual double              value(const AQLDate& basedate, AQLObject& inst, const AQLDataValuation& att) const;
	
	// calc option
	virtual double				calcOption(const AQLDataValuation& att, AQLDataProvider* dp, AQLObject& e) const;
	// calc payoff after maturity
	virtual double				calcPayOffAterMaturity(const AQLDataValuation& att, AQLDataProvider* dp, AQLObject& e) const;
	
	//set up analytic param
	virtual void setUpAnalyticParam(AQLObject& object, AQLDataProvider* dp) const;

	//crealte analytic param
	virtual std::vector< std::vector<AnalyticParam*> > createAnalyticParam(AQLObject& object, AQLDataProvider* dp) const;
	
	//analytic method manager
	virtual std::vector< std::vector<LABlackScholesBase* > > getAnalyticMethod(AQLObject& object, AQLDataProvider* dp) const;

	//payoff method manager
	virtual std::vector< std::vector<LABlackScholesBase* > > getPayoffMethod(AQLObject& object, AQLDataProvider* dp) const;

	//get cashlet size
	virtual unsigned int getCashletSize(const AQLObject& object, AQLDataProvider* dp) const;
	
	
	// set up dataProvider
	virtual AQLDataProvider*					setUpDataProvider(const AQLDate& basedate, AQLObject& object, 
											const AQLDataValuation& att) const;
	
	//get maturity date
	virtual const AQLDate& getMaturityDate(const AQLObject& object, AQLDataProvider* dp) const;
	//get delivery date
	virtual const AQLDate& getDeliveryDate(const AQLObject& object, AQLDataProvider* dp) const;
	//multiple unit
	virtual double multipleUnit(const AQLObject& object, AQLDataProvider* dp) const;
	//multiple fx
	virtual double multipleFX(const AQLObject& object, const AQLDataProvider* dp, const AQLString& from, const AQLString& to, double t) const;
	//get volatility result 
	virtual double getVolatilityResult(const AQLObject& object, AQLDataProvider* dp) const;
	//get volatilitymatrix result 
	virtual DoubleMatrix getVolatilityMatrixResult(const AQLObject& object, AQLDataProvider* dp) const;
	
	// Get Forward result
	virtual void getAnalyticParamResult(const AQLObject& object,
										AQLDataProvider* dp,
										AQLStringVector& names,
										DoubleVector& params
									   ) const;

	//get additional premium
	virtual double getAdditionalPremium(const AQLObject& object, AQLDataProvider* dp) const;
	//get additional premium
	virtual double getForwardPremium(const AQLObject& object, AQLDataProvider* dp) const;
	//get cash settlement amount
	virtual double getCashSettlementAmount(const AQLObject& object, AQLDataProvider* dp) const;

	virtual void outputResult( AQLObject& object, AQLDataProvider* dp ) const;

	
private:
	// create new cache class
	virtual	AQLDataProvider*			createNewDataProvider() const;

protected:
    // set pv currency
    virtual void setPVCurrency(const AQLObject& object, AQLDataProvider* dp) const;

    // set premium currency
    virtual void setPremiumCurrency(const AQLObject& object, AQLDataProvider* dp) const;

    virtual void setUpNumeraireCurrency(const AQLObject& trade, LALinearRatesOptionValueDataProvider* dataProvider) const;

	// set cash settlement currency
    virtual void setCashSettlementCurrency(const AQLObject& object, AQLDataProvider* dp) const;

//protected:
//	// Copy constructor
//	LALinearRatesOptionValue(LALinearRatesOptionValue& v);
};
