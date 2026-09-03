#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLPricePayOffTool.h"


// AQLPricePayOff's function id
#define FN_IR_PAYOFF		2002
// AQLPricePayOff's function name
#define FN_IR_PAYOFF_STR	"fn_ir_payoff"

#ifndef IR_CALIBRATION_DATA_DAYCOUNT
#define IR_CALIBRATION_DATA_DAYCOUNT			"DayCount"		// Data Name of DayCount
#endif
#ifndef PRICING_DATA_CFGENERATOR
#define PRICING_DATA_CFGENERATOR			"CashFlowGenerator"		// Data Name of Cashflow Generator
#endif
#ifndef PRICING_DATA_CALLINFO
#define PRICING_DATA_CALLINFO				"CallInfo"				// Data Name of Call Information 
#endif
#ifndef PRICING_DATA_TRIGGERINFOS
#define PRICING_DATA_TRIGGERINFOS			"TriggerInfos"			// Data Name of Trigger Information 
#endif
#ifndef PRICING_DATA_CASHLETS
#define PRICING_DATA_CASHLETS				"Cashlets"				//  data name of reference to cashlet entities
#endif
#ifndef PRICING_DATA_PAYMENTDATE
#define PRICING_DATA_PAYMENTDATE			"PaymentDate"			//  data name of paymentdate
#endif
#ifndef PRICING_DATA_ENDDATE
#define PRICING_DATA_ENDDATE				"EndDate"				//  data name of end date
#endif
#ifndef PRICING_DATA_ACTIONDATES
#define PRICING_DATA_ACTIONDATES			"ActionDates"			//  data name of action dates
#endif
#ifndef PRICING_DATA_EXPIRYDATES
#define PRICING_DATA_EXPIRYDATES			"ExpiryDates"			//  data name of expiry dates
#endif
#ifndef PRICING_DATA_EXTRACF
#define PRICING_DATA_EXTRACF				"ExtraCF"				//  data name of extra cf
#endif
#ifndef PRICING_DATA_EXTRACFS
#define PRICING_DATA_EXTRACFS				"ExtraCFs"				//  data name of extra cf array
#endif
#ifndef PRICING_DATA_EXTRACFDATES
#define PRICING_DATA_EXTRACFDATES			"ExtraCFDates"			//  data name of extra cf dates
#endif
#ifndef PRICING_DATA_EXTRACFINDEXINFOS
#define PRICING_DATA_EXTRACFINDEXINFOS		"ExtraCFIndexInfos"		//  data name of reference to index extra cf information entities
#endif
#ifndef PRICING_DATA_COUPONINFOS
#define PRICING_DATA_COUPONINFOS			"CouponInfos"			//  data name of reference to coupon information entities
#endif
#ifndef PRICING_DATA_INDEXINFOS
#define PRICING_DATA_INDEXINFOS				"IndexInfos"			//  data name of reference to index information entities
#endif
#ifndef PRICING_DATA_FRAINDEXINFO
#define PRICING_DATA_FRAINDEXINFO			"FRAIndexInfo"			//  data name of reference to fra index information entities
#endif
#ifndef PRICING_DATA_INDEXENTITY
#define PRICING_DATA_INDEXENTITY			"IndexEntity"			//  data name of reference to index  object
#endif
#ifndef PRICING_DATA_TARGETLEG
#define PRICING_DATA_TARGETLEG				"TargetLeg"				//  data name of trigger target leg
#endif
#ifndef PRICING_DATA_TRIGGERTARGETS
#define PRICING_DATA_TRIGGERTARGETS			"TriggerTargets"		//  data name of trigger targets
#endif
#ifndef PRICING_DATA_INDEXTYPE
#define PRICING_DATA_INDEXTYPE				"IndexType"				//  data name of index type
#endif
#ifndef PRICING_DATA_INDEXNAME
#define PRICING_DATA_INDEXNAME				"IndexName"				//  data name of index name
#endif
#ifndef PRICING_DATA_LSMCVARIABLES
#define PRICING_DATA_LSMCVARIABLES			"LSMCVariables"			//  data name of LSMC variables
#endif
#ifndef PRICING_DATA_FIXINGDATE
#define PRICING_DATA_FIXINGDATE				"FixingDate"			//  data name of fixing date
#endif
#ifndef PRICING_DATA_OBSERVATIONSTARTDATE
#define PRICING_DATA_OBSERVATIONSTARTDATE	"ObservationStartDate"	//  data name of observation start date
#endif
#ifndef PRICING_DATA_OBSERVATIONENDDATE
#define PRICING_DATA_OBSERVATIONENDDATE		"ObservationEndDate"	//  data name of observation end date
#endif
#ifndef PRICING_DATA_OBSERVATIONSLIDINGRULE
#define PRICING_DATA_OBSERVATIONSLIDINGRULE	"ObservationSlidingRule"	//  data name of observation slidingrule
#endif
#ifndef PRICING_DATA_OBSERVATIONCALENDAR
#define PRICING_DATA_OBSERVATIONCALENDAR	"ObservationCalendar"	//  data name of observation calendar
#endif
#ifndef PRICING_DATA_OBSERVATIONFREQUENCY
#define PRICING_DATA_OBSERVATIONFREQUENCY	"ObservationFrequency"	//  data name of observation frequency
#endif
#ifndef PRICING_DATA_OBSERVATIONDAY
#define PRICING_DATA_OBSERVATIONDAY			"ObservationDay"		//  data name of observation day
#endif
#ifndef PRICING_DATA_OBSERVATIONSTARTTERM
#define PRICING_DATA_OBSERVATIONSTARTTERM	"ObservationStartTerm"	//  data name of start term from some basedate
#endif
#ifndef PRICING_DATA_OBSERVATIONENDTERM
#define PRICING_DATA_OBSERVATIONENDTERM		"ObservationEndTerm"	//  data name of end term from some basedate
#endif
#ifndef PRICING_DATA_OBSERVATIONSAMEDAYS
#define PRICING_DATA_OBSERVATIONSAMEDAYS	"ObservationSameDays"	//  data name of observation same days
#endif
#ifndef PRICING_DATA_PATHENTITY
#define PRICING_DATA_PATHENTITY				"PathEntity"			//  data name of path object
#endif
#ifndef PRICING_DATA_DAYCOUNT
#define PRICING_DATA_DAYCOUNT				"DayCount"				//  data name of daycount
#endif
#ifndef PRICING_CALIBRATION_DATAOTIONAL
#define PRICING_CALIBRATION_DATAOTIONAL				"Notional"				//  data name of notional
#endif
#ifndef PRICING_DATA_CURRENCY
#define PRICING_DATA_CURRENCY				"Currency"				//  data name of notional currency
#endif
#ifndef PRICING_DATA_EXTRACFCURRENCY
#define PRICING_DATA_EXTRACFCURRENCY		"ExtraCFCurrency"		//  data name of extra cf currency
#endif
#ifndef PRICING_DATA_EXTRACFCURRENCIES
#define PRICING_DATA_EXTRACFCURRENCIES		"ExtraCFCurrencies"		//  data name of extra cf currency
#endif
#ifndef PRICING_DATA_ISMULTIEXTRACF
#define PRICING_DATA_ISMULTIEXTRACF		"IsMultiExtraCF"		//  data name of extra cf currency
#endif
#ifndef PRICING_DATA_FXRATE
#define PRICING_DATA_FXRATE					"FXRate"				//  data name of fx rate for notional 
#endif
#ifndef PRICING_DATA_EXTRACFFXRATE
#define PRICING_DATA_EXTRACFFXRATE			"ExtraCFFXRate"			//  data name of fx rate for extra cf
#endif
#ifndef PRICING_DATA_CFCALCSTARTDATE
#define PRICING_DATA_CFCALCSTARTDATE		"CFCalcStartDate"		//  data name of cf calc startdate
#endif
#ifndef PRICING_DATA_CFCALCENDDATE
#define PRICING_DATA_CFCALCENDDATE			"CFCalcEndDate"			//  data name of cf calc enddate
#endif
#ifndef PRICING_DATA_COUPONSELECTOPERATOR
#define PRICING_DATA_COUPONSELECTOPERATOR	"CouponSelectOperator"	//  data name of coupon select operator
#endif
#ifndef PRICING_DATA_ROUNDFUNCTION
#define PRICING_DATA_ROUNDFUNCTION			"RoundFunction"			//  data name of round function 
#endif
#ifndef PRICING_DATA_ROUNDDIGIT
#define PRICING_DATA_ROUNDDIGIT				"RoundDigit"			//  data name of round digit 
#endif
#ifndef PRICING_DATA_OBSERVATIONOPERATOR
#define PRICING_DATA_OBSERVATIONOPERATOR	"ObservationOperator"	//  data name of observation operator
#endif
#ifndef PRICING_DATA_FIXEDRATE
#define PRICING_DATA_FIXEDRATE				"FixedRate"				//  data name of fixed rate
#endif
#ifndef PRICING_DATA_ACCESSORY
#define PRICING_DATA_ACCESSORY				"Accessory"				//  data name of accessory
#endif
#ifndef PRICING_DATA_TRIGGERTYPE
#define PRICING_DATA_TRIGGERTYPE			"TriggerType"			//  data name of trigger type
#endif
#ifndef PRICING_DATA_OPERATOR
#define PRICING_DATA_OPERATOR				"Operator"				//  data name of operator
#endif
#ifndef PRICING_DATA_COEFFICIENT
#define PRICING_DATA_COEFFICIENT			"Coefficient"			//  data name of coefficient
#endif
#ifndef PRICING_DATA_COEFFICIENTS
#define PRICING_DATA_COEFFICIENTS			"Coefficients"			//  data name of coefficient
#endif
#ifndef PRICING_DATA_OPTION
#define PRICING_DATA_OPTION					"Option"				//  data name of option
#endif
#ifndef PRICING_DATA_ISTERMINATE
#define PRICING_DATA_ISTERMINATE			"IsTerminate"			//  data name of terminate or not
#endif
#ifndef PRICING_DATA_COUPONCHANGEINFO
#define PRICING_DATA_COUPONCHANGEINFO		"CouponChangeInfo"		//  data name of coupon change info
#endif
#ifndef PRICING_CALIBRATION_DATAOTIONALCHANGERATIO
#define PRICING_CALIBRATION_DATAOTIONALCHANGERATIO	"NotionalChangeRatio"	//  data name of notinal change ratio
#endif
#ifndef PRICING_DATA_MAXCOUPON
#define PRICING_DATA_MAXCOUPON				"MaxCoupon"				//  data name of max
#endif
#ifndef PRICING_DATA_MINCOUPON
#define PRICING_DATA_MINCOUPON				"MinCoupon"				//  data name of min
#endif
#ifndef PRICING_DATA_MAXINDEX
#define PRICING_DATA_MAXINDEX				"MaxIndex"				//  data name of max
#endif
#ifndef PRICING_DATA_MININDEX
#define PRICING_DATA_MININDEX				"MinIndex"				//  data name of min
#endif
#ifndef PRICING_DATA_PAYMENTTIMING
#define PRICING_DATA_PAYMENTTIMING			"PaymentTiming"			//  data name of payment timing 
#endif
#ifndef PRICING_DATA_ISAMORTIZE
#define PRICING_DATA_ISAMORTIZE				"IsAmortize"			//  data name of amortize flag
#endif
#ifndef PRICING_DATA_AMORTIZETYPE
#define PRICING_DATA_AMORTIZETYPE			"AmortizeType"			//  data name of amortize type
#endif
#ifndef PRICING_DATA_AMORTIZEAMOUNT
#define PRICING_DATA_AMORTIZEAMOUNT			"AmortizeAmount"		//  data name of amortize amount
#endif
#ifndef PRICING_DATA_AMORTIZEROUNDFUNCTION
#define PRICING_DATA_AMORTIZEROUNDFUNCTION	"AmortizeRoundFunction"	//  data name of amortize round function
#endif
#ifndef PRICING_DATA_AMORTIZEROUNDDIGIT
#define PRICING_DATA_AMORTIZEROUNDDIGIT		"AmortizeRoundDigit"	//  data name of amortize round digit
#endif
#ifndef PRICING_DATA_ISAMORTIZE1STFRACTION
#define PRICING_DATA_ISAMORTIZE1STFRACTION		"IsAmortize1stFraction"	//  data name of amortize is first fraction or last fraction
#endif
#ifndef PRICING_DATA_ISNOTIONALEXCHANGEATEND
#define PRICING_DATA_ISNOTIONALEXCHANGEATEND	"IsNotionalExchangeAtEnd"	//  data name of flag of notional change at end 
#endif
#ifndef PRICING_DATA_OBSERVATIONDATES
#define PRICING_DATA_OBSERVATIONDATES		"ObservationDates"		//  data name of observation dates
#endif
#ifndef PRICING_DATA_OBSERVATIONRATES
#define PRICING_DATA_OBSERVATIONRATES		"ObservationRates"		//  data name of observation rates
#endif
#ifndef PRICING_DATA_OBSERVATIONRATESTYPE
#define PRICING_DATA_OBSERVATIONRATESTYPE		"ObservationRatesType"		//  data name of observation rates type
#endif
#ifndef PRICING_DATA_PAYMENTDATES
#define PRICING_DATA_PAYMENTDATES			"PaymentDates"			//  data name of payment dates
#endif
#ifndef PRICING_DATA_COUPONS
#define PRICING_DATA_COUPONS				"Coupons"				//  data name of past coupon array
#endif
#ifndef PRICING_DATA_ISSPOTACCRUEDINTEREST
#define PRICING_DATA_ISSPOTACCRUEDINTEREST	"IsSpotAccruedInterest" //  data name of is spot accrued interest
#endif
#ifndef PRICING_DATA_COUPONPAYOFFS
#define PRICING_DATA_COUPONPAYOFFS			"CouponPayOffs"			//  data name of past coupon payoff array
#endif
#ifndef PRICING_DATA_ISACCRUAL
#define PRICING_DATA_ISACCRUAL				"IsAccrual"				//  data name of flag of accrual payment occur or not when trade is terminated
#endif
#ifndef PRICING_DATA_STARTDATE
#define PRICING_DATA_STARTDATE				"StartDate"				//  data name of start date
#endif
#ifndef PRICING_DATA_DENOMINATOR
#define PRICING_DATA_DENOMINATOR			"Denominator"			//  data name of Denominator
#endif
#ifndef PRICING_DATA_PIPSIZE
#define PRICING_DATA_PIPSIZE				"PipSize"				//  data name of PipSize
#endif
#ifndef PRICING_DATA_SELECTSIDE
#define PRICING_DATA_SELECTSIDE				"SelectSide"			//  data name of SelectSide
#endif
#ifndef PRICING_DATA_FIXEDDATES
#define PRICING_DATA_FIXEDDATES				"FixedDates"			//  data name of fixed dates
#endif
#ifndef PRICING_DATA_FIXEDRATES
#define PRICING_DATA_FIXEDRATES				"FixedRates"			//  data name of fixed Rates
#endif
#ifndef PRICING_CALIBRATION_DATAOTIONALCF
#define PRICING_CALIBRATION_DATAOTIONALCF				"NotionalCF"			//  data name of notional cf
#endif
#ifndef PRICING_DATA_EXTRACFFUNC
#define PRICING_DATA_EXTRACFFUNC			"ExtraCFFunc"			//  data name of extra cf function
#endif
#ifndef PRICING_DATA_EXTRACFFUNCINPUTS
#define PRICING_DATA_EXTRACFFUNCINPUTS		"ExtraCFFuncInputs"		//  data name of extra cf function inputs
#endif
#ifndef PRICING_DATA_EXTRACFFUNCCOEFFICIENTS
#define PRICING_DATA_EXTRACFFUNCCOEFFICIENTS	"ExtraCFFuncCoefficients"		//  data name of extra cf function coefficients
#endif
#ifndef PRICING_DATA_EXTRACFFIXINGOFFSET
#define PRICING_DATA_EXTRACFFIXINGOFFSET	"ExtraCFFixingOffSet"			//  data name of extra cf fixing offset
#endif
#ifndef PRICING_CALIBRATION_DATAOTIONALEXCHANGECURRENCY
#define PRICING_CALIBRATION_DATAOTIONALEXCHANGECURRENCY	"NotionalExchangeCurrency"		//  data name of notional exchange currency
#endif
#ifndef PRICING_CALIBRATION_DATAOTIONALEXCHANGEFXRATE
#define PRICING_CALIBRATION_DATAOTIONALEXCHANGEFXRATE		"NotionalExchangeFXRate"		//  data name of notional exchange fx rate
#endif
#ifndef PRICING_DATA_FXRATEFIXINGDATE
#define PRICING_DATA_FXRATEFIXINGDATE		"FXRateFixingDate"		//  data name of fx rate fixing date
#endif
#ifndef PRICING_DATA_FXRATEFIXINGTERM
#define PRICING_DATA_FXRATEFIXINGTERM		"FXRateFixingTerm"		//  data name of fx rate fixing term
#endif
#ifndef PRICING_DATA_FXRATEFIXINGSLIDINGRULE
#define PRICING_DATA_FXRATEFIXINGSLIDINGRULE		"FXRateFixingSlidingRule"		//  data name of fx rate fixing sliding rule
#endif
#ifndef PRICING_DATA_FXRATEFIXINGCALENDAR
#define PRICING_DATA_FXRATEFIXINGCALENDAR		"FXRateFixingCalendar"		//  data name of fx rate fixing calendar
#endif
#ifndef PRICING_CALIBRATION_DATAOTIONALCFS
#define PRICING_CALIBRATION_DATAOTIONALCFS				"NotionalCFs"				//  data name of notional cf array
#endif
#ifndef PRICING_CALIBRATION_DATAOTIONALCFS
#define PRICING_CALIBRATION_DATAOTIONALCFS				"NotionalCFs"				//  data name of notional cf array
#endif
#ifndef PRICING_DATA_FXRATESTRUCTUREFUNC
#define PRICING_DATA_FXRATESTRUCTUREFUNC		"FXRateStructureFunc"		//  data name of fx rate structure method
#endif
#ifndef PRICING_DATA_POLYNOMIAL
#define PRICING_DATA_POLYNOMIAL					"Polynomial"					//  data name of polynomial
#endif
#ifndef PRICING_DATA_PRODUCT
#define PRICING_DATA_PRODUCT					"Product"					//  data name of product
#endif
#ifndef PRICING_CALIBRATION_DATAOTIONALCFCOUPONINFOS
#define PRICING_CALIBRATION_DATAOTIONALCFCOUPONINFOS		"NotionalCFCouponInfos"		//  data name of NotionalCFCouponInfos
#endif
#ifndef PRICING_CALIBRATION_DATAOTIONALCFCOUPONSELECTOPERATOR
#define PRICING_CALIBRATION_DATAOTIONALCFCOUPONSELECTOPERATOR	"NotionalCFCouponSelectOperator"	//  data name of NotionalCFCouponSelectOperator
#endif
#ifndef PRICING_DATA_EXTRACFFORINITIALEXCHANGE
#define PRICING_DATA_EXTRACFFORINITIALEXCHANGE	"ExtraCFForInitialExchange"	//  data name of ExtraCFForInitialExchange
#endif
#ifndef PRICING_DATA_FXFORINITIALEXCHANGE
#define PRICING_DATA_FXFORINITIALEXCHANGE	"FXForInitialExchange"	//  data name of FXForInitialExchange
#endif
#ifndef PRICING_DATA_FUNDINGSPREAD
#define PRICING_DATA_FUNDINGSPREAD     "FundingSpread"  //  data name of FundingSpread
#endif
#ifndef PRICING_DATA_ISRANGEACCRUE
#define PRICING_DATA_ISRANGEACCRUE	"IsRangeAccrue"	//  data name of range accrue
#endif
#ifndef PRICING_DATA_RANGEACCRUEINDEXINFOS
#define PRICING_DATA_RANGEACCRUEINDEXINFOS	"RangeAccrueIndexInfos"	//  data name of range accrue index
#endif
#ifndef PRICING_DATA_RANGEACCRUEOPERATOR
#define PRICING_DATA_RANGEACCRUEOPERATOR	"RangeAccrueOperator"	//  data name of range accrue operator
#endif
#ifndef PRICING_DATA_RANGEACCRUECOEFFICIENT
#define PRICING_DATA_RANGEACCRUECOEFFICIENT	"RangeAccrueCoefficient"	//  data name of range accrue coefficient
#endif
#ifndef PRICING_DATA_RANGEACCRUEMAX
#define PRICING_DATA_RANGEACCRUEMAX	"RangeAccrueMax"	//  data name of range accrue max
#endif
#ifndef PRICING_DATA_RANGEACCRUEMIN
#define PRICING_DATA_RANGEACCRUEMIN	"RangeAccrueMin"	//  data name of range accrue min
#endif
#ifndef PRICING_DATA_RANGEACCRUESAMEOBSERVATIONDAYS
#define PRICING_DATA_RANGEACCRUESAMEOBSERVATIONDAYS	"RangeAccrueSameObservationDays"	//  data name of range accrue same observation days
#endif
#ifndef PRICING_DATA_ROLLDAYFORINDEXGENERATE
#define PRICING_DATA_ROLLDAYFORINDEXGENERATE "RollDayForIndexGenerate"  //  data name of RollDayForIndexGenerate
#endif
#ifndef PRICING_DATA_SPOTLAG
#define PRICING_DATA_SPOTLAG "SpotLag"  //  data name of SpotLag
#endif
#ifndef PRICING_DATA_DATESFORINDEXGENERATE
#define PRICING_DATA_DATESFORINDEXGENERATE "DatesForIndexGenerate"  //  data name of DatesForIndexGenerate
#endif
#ifndef PRICING_DATA_ISLEGBASECURRENCY
#define PRICING_DATA_ISLEGBASECURRENCY         "IsLegBaseCurrency"        // data name of IsLegBaseCurrency
#endif
#ifndef	PRICING_DATA_RANGEACCRUESAMEOBSERVATIONDATE	
#define	PRICING_DATA_RANGEACCRUESAMEOBSERVATIONDATE	"RangeAccrueSameObservationDate"
#endif		
#ifndef	PRICING_DATA_RANGEACCRUEBOUNDARYINDEXINFOS	
#define	PRICING_DATA_RANGEACCRUEBOUNDARYINDEXINFOS	"RangeAccrueBoundaryIndexInfos"
#endif		
#ifndef	PRICING_DATA_RANGEACCRUEMAXBOUNDARYOPERATOR	
#define	PRICING_DATA_RANGEACCRUEMAXBOUNDARYOPERATOR	"RangeAccrueMaxBoundaryOperator"
#endif		
#ifndef	PRICING_DATA_RANGEACCRUEMINBOUNDARYOPERATOR	
#define	PRICING_DATA_RANGEACCRUEMINBOUNDARYOPERATOR	"RangeAccrueMinBoundaryOperator"
#endif		
#ifndef	PRICING_DATA_RANGEACCRUEMAXBOUNDARYCOEFFICIENT	
#define	PRICING_DATA_RANGEACCRUEMAXBOUNDARYCOEFFICIENT	"RangeAccrueMaxBoundaryCoefficient"
#endif		
#ifndef	PRICING_DATA_RANGEACCRUEMINBOUNDARYCOEFFICIENT	
#define	PRICING_DATA_RANGEACCRUEMINBOUNDARYCOEFFICIENT	"RangeAccrueMinBoundaryCoefficient"
#endif		
#ifndef	PRICING_DATA_RANGEACCRUEOBSERVATIONSTARTDATE	
#define	PRICING_DATA_RANGEACCRUEOBSERVATIONSTARTDATE	"RangeAccrueObservationStartDate"
#endif		
#ifndef	PRICING_DATA_RANGEACCRUEOBSERVATIONENDDATE	
#define	PRICING_DATA_RANGEACCRUEOBSERVATIONENDDATE	"RangeAccrueObservationEndDate"
#endif
#ifndef	PRICING_DATA_RANGEACCRUEINFOS	
#define	PRICING_DATA_RANGEACCRUEINFOS	"RangeAccrueInfos"
#endif
#ifndef	PRICING_DATA_ISANDCONDITION	
#define	PRICING_DATA_ISANDCONDITION		"IsAndCondition"
#endif
#ifndef	PRICING_DATA_ISNOTCONDITION	
#define	PRICING_DATA_ISNOTCONDITION		"IsNotCondition"
#endif
#ifndef	PRICING_DATA_RANGEACCRUESTRIKE	
#define	PRICING_DATA_RANGEACCRUESTRIKE	"RangeAccrueStrike"
#endif
#ifndef	PRICING_DATA_ISEXCLUDEANDCONDITION	
#define	PRICING_DATA_ISEXCLUDEANDCONDITION		"IsExcludeAndCondition"
#endif
#ifndef	PRICING_DATA_BUSINESSDAYSBASE	
#define	PRICING_DATA_BUSINESSDAYSBASE		"IsBusinessDaysBase"
#endif
#ifndef	PRICING_DATA_OBSERVATIONDATES	
#define	PRICING_DATA_OBSERVATIONDATES	"ObservationDates"
#endif
#ifndef	PRICING_DATA_HITRATEOPERATOR	
#define	PRICING_DATA_HITRATEOPERATOR		"HitRateOperator"
#endif
#ifndef	PRICING_DATA_HITRATECOEFFICIENT	
#define	PRICING_DATA_HITRATECOEFFICIENT		"HitRateCoefficient"
#endif
#ifndef	PRICING_DATA_HITRATEMIN	
#define	PRICING_DATA_HITRATEMIN		"HitRateMin"
#endif
#ifndef	PRICING_DATA_HITRATEMAX	
#define	PRICING_DATA_HITRATEMAX		"HitRateMax"
#endif

#ifndef PRICING_DATA_ISSAVEPASTFIXING
#define PRICING_DATA_ISSAVEPASTFIXING         "IsSavePastFixing"        // data name of IsSavePastFixing
#endif

#ifndef PRICING_DATA_COMPOUNDINGDAYCOUNT
#define PRICING_DATA_COMPOUNDINGDAYCOUNT         "CompoundingDayCount"        // data name of CompoundingDayCount
#endif
#ifndef PRICING_DATA_COMPOUNDINGMARGIN
#define PRICING_DATA_COMPOUNDINGMARGIN         "CompoundingMargin"        // data name of CompoundingMargin
#endif
#ifndef PRICING_DATA_COMPOUNDINGMARGINS
#define PRICING_DATA_COMPOUNDINGMARGINS         "CompoundingMargins"        // data name of CompoundingMargins
#endif
#ifndef PRICING_DATA_COMPOUNDINGDATES
#define PRICING_DATA_COMPOUNDINGDATES         "CompoundingDates"        // data name of CompoundingDates
#endif
#ifndef PRICING_DATA_OBSERVATIONMARGINS
#define PRICING_DATA_OBSERVATIONMARGINS         "ObservationMargins"        // data name of ObservationMargins
#endif
#ifndef PRICING_DATA_DISCOUNTSTARTDATE
#define PRICING_DATA_DISCOUNTSTARTDATE         "DiscountStartDate"        // data name of DiscountStartDate
#endif
#ifndef PRICING_DATA_DISCOUNTENDDATE
#define PRICING_DATA_DISCOUNTENDDATE         "DiscountEndDate"        // data name of DiscountEndDate
#endif
#ifndef PRICING_DATA_UPFRONTFEES
#define PRICING_DATA_UPFRONTFEES         "UpfrontFees"        // data name of UpfrontFees
#endif
#ifndef PRICING_DATA_UPFRONTPAYMENTDATES
#define PRICING_DATA_UPFRONTPAYMENTDATES         "UpfrontPaymentDates"        // data name of UpfrontPaymentDates
#endif
#ifndef PRICING_DATA_FIXINGDATES
#define PRICING_DATA_FIXINGDATES         "FixingDates"        // data name of FixingDates
#endif
#ifndef PRICING_DATA_CFCALCSTARTDATES
#define PRICING_DATA_CFCALCSTARTDATES	"CFCalcStartDates"        // data name of CFCalcStartDates
#endif
#ifndef PRICING_DATA_CFCALCENDDATES
#define PRICING_DATA_CFCALCENDDATES		"CFCalcEndDates"        // data name of CFCalcEndDates
#endif
#ifndef PRICING_DATA_RENOTIONALFIXINGDATE
#define PRICING_DATA_RENOTIONALFIXINGDATE			"RenotionalFixingDate"
#endif
#ifndef PRICING_DATA_COMPOUND_ON_ALL_DAYS
#define PRICING_DATA_COMPOUND_ON_ALL_DAYS "CompoundOnAllDays"
#endif
#ifndef PRICING_DATA_ISCOMPOUNDINGCOUPON
#define PRICING_DATA_ISCOMPOUNDINGCOUPON "IsCompoundingCoupon"
#endif
#ifndef PRICING_DATA_FIRSTSTUBCOUPON
#define PRICING_DATA_FIRSTSTUBCOUPON "FirstStubCoupon"
#endif
#ifndef PRICING_DATA_LASTSTUBCOUPON
#define PRICING_DATA_LASTSTUBCOUPON "LastStubCoupon"
#endif
#ifndef PRICING_DATA_COMPOUNDING_FUNCTION
#define PRICING_DATA_COMPOUNDING_FUNCTION "CompoundingFunction"
#endif
#ifndef PRICING_DATA_FUNDINGSPREADENTITY
#define PRICING_DATA_FUNDINGSPREADENTITY "FundingSpreadEntity"
#endif
#ifndef PRICING_DATA_COUPONCFCURRENCY
#define PRICING_DATA_COUPONCFCURRENCY "CouponCFCurrency"
#endif
#ifndef PRICING_CALIBRATION_DATAOTIONALCFCURRENCY
#define PRICING_CALIBRATION_DATAOTIONALCFCURRENCY "NotionalCFCurrency"
#endif
#ifndef PRICING_DATA_COUPONCFFXRATEVALUE
#define PRICING_DATA_COUPONCFFXRATEVALUE "CouponCFFXRateValue"
#endif
#ifndef PRICING_CALIBRATION_DATAOTIONALCFFXRATEVALUE
#define PRICING_CALIBRATION_DATAOTIONALCFFXRATEVALUE "NotionalCFFXRateValue"
#endif
#ifndef PRICING_DATA_ISREDEMPTION
#define PRICING_DATA_ISREDEMPTION "IsRedemption"
#endif
#ifndef PRICING_DATA_ISBONDFIXED
#define PRICING_DATA_ISBONDFIXED "IsBondFixed"
#endif
#ifndef PRICING_DATA_LEGNUMBER
#define PRICING_DATA_LEGNUMBER			"LegNumber"			// data name of leg number calculation result assigned to 
#endif
#ifndef PRICING_DATA_SETTLEMENTCURRENCY
#define PRICING_DATA_SETTLEMENTCURRENCY			"SettlementCurrency"			// data name of settlement currency used for exotic products with non-deliverable currencies
#endif
#ifndef PRICING_DATA_SETTLEMENTFIXINGDATE
#define PRICING_DATA_SETTLEMENTFIXINGDATE			"SettlementFixingDate"			// data name of settlement fixing date used for exotic products with non-deliverable currencies
#endif
#ifndef PRICING_DATA_SETTLEMENTADJUSTRATIO
#define PRICING_DATA_SETTLEMENTADJUSTRATIO			"SettlementAdjustRatio"			// data name of the ratio fx(fixing)/fx(payment) used for exotic products with non-deliverable currencies
#endif
#ifndef PRICING_DATA_SETTLEMENTFIXINGCALENDAR
#define PRICING_DATA_SETTLEMENTFIXINGCALENDAR			"SettlementFixingCalendar"
#endif
#ifndef PRICING_DATA_SETTLEMENTFIXINGSLIDINGRULE
#define PRICING_DATA_SETTLEMENTFIXINGSLIDINGRULE			"SettlementFixingSlidingRule"
#endif
#ifndef PRICING_DATA_SETTLEMENTFIXINGOFFSET
#define PRICING_DATA_SETTLEMENTFIXINGOFFSET			"SettlementFixingOffSet"
#endif
#ifndef PRICING_DATA_SETTLEMENTFIXINGDATES
#define PRICING_DATA_SETTLEMENTFIXINGDATES			"SettlementFixingDates"
#endif
#ifndef PRICING_DATA_SETTLEMENTADJUSTRATIOS
#define PRICING_DATA_SETTLEMENTADJUSTRATIOS			"SettlementAdjustRatios"
#endif
#ifndef PRICING_DATA_CONVEXITYADJUSTMENT
#define PRICING_DATA_CONVEXITYADJUSTMENT "ConvexityAdjustment"
#endif
#ifndef PRICING_CALIBRATION_DATAOTIONALCFISFIXED
#define PRICING_CALIBRATION_DATAOTIONALCFISFIXED "NotionalCfIsFixed"
#endif
#ifndef PRICING_DATA_IS_FIXED_CF
#define PRICING_DATA_IS_FIXED_CF				"IsFixedCF"		
#endif



class AQLObject;
class AQLPriceDataManager;
class AQLPriceEventTool;
class AQLPriceIndexToolBase;
class AQLPriceEventHolder;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief declaration of payoff calculator class for mc simulation.
*/
class AQLPricePayOff : public AQLCoreFunctionBase
{
public:
//  LIFECYCLE
    // constructor	
	AQLPricePayOff();
    // destructor	
	virtual ~AQLPricePayOff();
	// copy constructor
	AQLPricePayOff(const AQLPricePayOff& v);
								//======================================
								// Check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual AQLCoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%
								//======================================
								// Return this class type
    virtual function_t          getType() const;

    // set up for payoff calculation 
	virtual void	            setUp(const AQLDate& basedate, AQLObject& trade);

	virtual void				setUpAccrued(const AQLDate& basedate, AQLObject& trade, int number);

	// calculate payoff
	virtual void				calcPayOff(DoubleMatrix& time, DoubleMatrix& payoff, bool trigger, 
											std::vector<std::pair<unsigned int, AQLDate> >& triggerhit,
											/*bool includecall = true,*/
											bool islsmc = false, DoubleArray* prebate = NULL, 
											DoubleMatrix* pexplanatory = NULL, std::vector<UintArray>* extracfpos = NULL) const;

	// register dataValues that this class uses
	virtual void				registerData(AQLPriceDataManager& dm) const;
	/*!
		@brief set LSMC mode(regression coefficient calculation mode) or not
		@param[in] flag true:LSMC mode, false:non LSMC mode
	*/
	void						setLSMCMode(bool flag) {mIsLSMC = flag;}
	
	/*!
		@brief get payoff 
		@return payoff
	*/
	const std::vector<PayOffToolHolderVector>& 
								getPayOff(void) const 
								{
									if (mIsTrigger) return mPayOff;
									else return mPayOffMaster;
								}
	/*!
		@brief get payoff 
		@return payoff
	*/
	std::vector<PayOffToolHolderVector>& 
								getPayOff(void) 
								{
									if (mIsTrigger) return mPayOff;
									else return mPayOffMaster;
								}
	// set LSMC regression coefficient 
//	void						setLSMCCoefficient(const DoubleMatrix& coeff);
	const std::vector<PayOffToolHolderVector>& getPayOffMaster(void) const { return mPayOffMaster; }
	// cal derivation of libor
	virtual void				calcDerivationOfLibor(std::map<AQLString, std::map<double, double> >& delivationLiborMap, std::vector<const AQLInterpolationBase*>& pNumeInterpVec, BoolVector& isRecVec) const;

	virtual void getCompoundedRateInfo(const size_t leg, DateVector& start, DateVector& end, DoubleVector& term, DateVector& fixing_date, AQLStringVector& fixing_flag, DoubleVector& rate) const;
protected:

private:
	// clear time grid of index object 
	void						clearIndexEntityGrid (AQLObject& trade) const;
	// setup index object 
	void						setUpIndexEntity(const AQLDate& basedate, AQLObject& trade) const;
	// setup index object for coupon calculation
	void						setUpIndexEntityOfCashlet(const AQLDate& basedate, AQLObject& cashlet, const AQLObject& trade) const;
	// setup index object for call judge
	void						setUpIndexEntityOfCall (const AQLDate& basedate, AQLObject& callinfo, const AQLObject& trade) const;
	// setup index object for trigger judge
	void						setUpIndexEntityOfTrigger (const AQLDate& basedate, AQLObject& triggerinfo, const AQLObject& trade) const;
	// setup index object refered from index information object
	void						setUpIndexEntityOfIndex (const AQLDate& basedate, AQLObject& indexinfo, const AQLObject& trade,
												const DateVector* pfixingdates = NULL, bool isInter = false) const;
	// setup payoff
	void						setUpPayOff(const AQLDate& basedate, AQLObject& trade);

	void						setUpPayOffAccrued(const AQLDate& basedate, AQLObject& trade, int number);

	// setup trigger and call
	void						setUpTrigger(const AQLDate& basedate, AQLObject& trade);
    // check range acccrue cashlet
	bool                         isRangeAccrueCashlet(const AQLObject &object) const;

    bool isCompoundingCashlet(const AQLObject& object) const;

	bool								mIsLSMC;	// LSMC mode(regression coefficient calculation mode) flag
	std::vector<PayOffToolHolderVector> mPayOffMaster;// payoff
	mutable std::vector<PayOffToolHolderVector> mPayOff;// temporary payoff
	DoubleMatrix						mTimes;		// payment time
	bool								mIsTrigger; // their are triggers or not 	
	std::vector<AQLPriceEventTool*>		mTriggers;  // triggerTool
	std::vector<AQLPriceEventHolder*> mActions; // triggerAction, potiner owner is mTriggers

};
