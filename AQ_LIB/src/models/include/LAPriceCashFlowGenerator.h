#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreProcedure.h"
#include "AQLDate.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"


// Function ID for LAPriceCashFlowGenerator
#define FN_IR_CASHFLOWGENERATOR		2001
// Function name for LAPriceCashFlowGenerator
#define FN_IR_CASHFLOWGENERATOR_STR	"fn_ir_cashflowgenerator"


#ifndef PRICING_DATA_STARTDATE
#define PRICING_DATA_STARTDATE				"StartDate"				//  data name of start date
#endif
#ifndef PRICING_DATA_ENDDATE
#define PRICING_DATA_ENDDATE				"EndDate"				//  data name of end date
#endif
#ifndef PRICING_DATA_ENDTERM
#define PRICING_DATA_ENDTERM				"EndTerm"				//  data name of term string
#endif
#ifndef PRICING_DATA_FREQUENCY
#define PRICING_DATA_FREQUENCY				"Frequency"				//  data name of frenquency of payment
#endif
#ifndef PRICING_DATA_PAYMENTTIMING
#define PRICING_DATA_PAYMENTTIMING			"PaymentTiming"			//  data name of payment timing 
#endif
#ifndef PRICING_DATA_FIXINGTIMING
#define PRICING_DATA_FIXINGTIMING			"FixingTiming"			//  data name of fixing timing 
#endif
#ifndef PRICING_DATA_FIRSTODDDATE
#define PRICING_DATA_FIRSTODDDATE			"FirstOddDate"			//  data name of first odd date
#endif
#ifndef PRICING_DATA_LASTODDDATE
#define PRICING_DATA_LASTODDDATE			"LastOddDate"			//  data name of last odd date
#endif
#ifndef PRICING_DATA_FIRSTODDINDEXTYPE
#define PRICING_DATA_FIRSTODDINDEXTYPE		"FirstOddIndexType"		//  data name of first odd index type
#endif
#ifndef PRICING_DATA_LASTODDINDEXTYPE
#define PRICING_DATA_LASTODDINDEXTYPE		"LastOddIndexType"		//  data name of last odd index type
#endif
#ifndef PRICING_DATA_COUPONDAY
#define PRICING_DATA_COUPONDAY				"CouponDay"				//  data name of coupon payment day 
#endif
#ifndef PRICING_DATA_ROLLCONVENTION
#define PRICING_DATA_ROLLCONVENTION         "RollConvention"		//  data name of roll convention 
#endif
#ifndef PRICING_DATA_COUPONINFOS
#define PRICING_DATA_COUPONINFOS			"CouponInfos"			//  data name of reference to coupon information entities
#endif
#ifndef PRICING_DATA_CASHLETS
#define PRICING_DATA_CASHLETS				"Cashlets"				//  data name of reference to cashlet entities
#endif
#ifndef PRICING_DATA_ISSPOTACCRUEDINTEREST
#define PRICING_DATA_ISSPOTACCRUEDINTEREST	"IsSpotAccruedInterest"	//  data name of is spot accruedinterest
#endif 
#ifndef PRICING_DATA_PAYMENTDATE
#define PRICING_DATA_PAYMENTDATE			"PaymentDate"			//  data name of paymentdate
#endif
#ifndef PRICING_DATA_CFCALCSTARTDATE
#define PRICING_DATA_CFCALCSTARTDATE		"CFCalcStartDate"		//  data name of cf calc startdate
#endif
#ifndef PRICING_DATA_CFCALCENDDATE
#define PRICING_DATA_CFCALCENDDATE			"CFCalcEndDate"			//  data name of cf calc enddate
#endif
#ifndef PRICING_CALIBRATION_DATAOTIONAL
#define PRICING_CALIBRATION_DATAOTIONAL				"Notional"				//  data name of cf notional
#endif
#ifndef PRICING_DATA_CURRENCY
#define PRICING_DATA_CURRENCY				"Currency"				//  data name of notional currency
#endif
#ifndef PRICING_DATA_FXRATE
#define PRICING_DATA_FXRATE					"FXRate"				//  data name of fx rate for notional 
#endif
#ifndef PRICING_DATA_ISNOTIONALEXCHANGEATSTART
#define PRICING_DATA_ISNOTIONALEXCHANGEATSTART	"IsNotionalExchangeAtStart"	//  data name of flag of notional change at start 
#endif
#ifndef PRICING_DATA_ISNOTIONALEXCHANGEATEND
#define PRICING_DATA_ISNOTIONALEXCHANGEATEND	"IsNotionalExchangeAtEnd"	//  data name of flag of notional change at end 
#endif
#ifndef PRICING_DATA_DAYCOUNT
#define PRICING_DATA_DAYCOUNT				"DayCount"				//  data name of day cont convention
#endif
#ifndef PRICING_DATA_EXTRACF
#define PRICING_DATA_EXTRACF				"ExtraCF"				//  data name of extra cf
#endif
#ifndef PRICING_DATA_COUPONSELECTOPERATOR
#define PRICING_DATA_COUPONSELECTOPERATOR	"CouponSelectOperator"	//  data name of coupon select operator
#endif
#ifndef PRICING_DATA_INDEXINFOS
#define PRICING_DATA_INDEXINFOS				"IndexInfos"			//  data name of reference to index information entities
#endif
#ifndef PRICING_DATA_ODDINDEXINFOS
#define PRICING_DATA_ODDINDEXINFOS				"OddIndexInfos"			//  data name of reference to OddIndex information entities
#endif
#ifndef PRICING_DATA_OPERATOR
#define PRICING_DATA_OPERATOR				"Operator"				//  data name of operator
#endif
#ifndef PRICING_DATA_ODDINDEXTYPE
#define PRICING_DATA_ODDINDEXTYPE		"OddIndexType"
#endif
#ifndef PRICING_DATA_ISODDTRADE
#define PRICING_DATA_ISODDTRADE		"IsOddTrade"
#endif
#ifndef PRICING_DATA_OBSERVATIONSTARTTERM
#define PRICING_DATA_OBSERVATIONSTARTTERM	"ObservationStartTerm"	//  data name of observation start date (relative span from basedate)
#endif
#ifndef PRICING_DATA_OBSERVATIONSTARTDATE
#define PRICING_DATA_OBSERVATIONSTARTDATE	"ObservationStartDate"	//  data name of observation start date
#endif
#ifndef PRICING_DATA_OBSERVATIONENDTERM
#define PRICING_DATA_OBSERVATIONENDTERM		"ObservationEndTerm"	//  data name of observation end date (relative span from basedate)
#endif
#ifndef PRICING_DATA_OBSERVATIONENDDATE
#define PRICING_DATA_OBSERVATIONENDDATE		"ObservationEndDate"	//  data name of observation end date
#endif
#ifndef PRICING_DATA_OBSERVATIONSLIDINGRULE
#define PRICING_DATA_OBSERVATIONSLIDINGRULE	"ObservationSlidingRule"//  data name of observation slidingrule
#endif
#ifndef PRICING_DATA_OBSERVATIONCALENDAR
#define PRICING_DATA_OBSERVATIONCALENDAR	"ObservationCalendar"	//  data name of observation calendar
#endif
#ifndef PRICING_DATA_OBSERVATIONSAMEDAYS
#define PRICING_DATA_OBSERVATIONSAMEDAYS	"ObservationSameDays"	//  data name of observation same days
#endif
#ifndef PRICING_DATA_OFFSET
#define PRICING_DATA_OFFSET					"OffSet"				//  data name of offset days
#endif
#ifndef PRICING_DATA_FIXINGCALENDAR
#define PRICING_DATA_FIXINGCALENDAR			"FixingCalendar"		//  data name of calendar for fixing date
#endif
#ifndef PRICING_DATA_FIXINGSLIDINGRULE
#define PRICING_DATA_FIXINGSLIDINGRULE		"FixingSlidingRule"		//  data name of slidingrule for fixing date
#endif
#ifndef PRICING_DATA_FIXINGSPECIALOFFSET
#define PRICING_DATA_FIXINGSPECIALOFFSET	"FixingSpecialOffSet"	//  data name of offset term for fixing
#endif
#ifndef PRICING_DATA_FIXINGSPECIALDAY
#define PRICING_DATA_FIXINGSPECIALDAY		"FixingSpecialDay"		//  data name of fixing special day
#endif
#ifndef PRICING_DATA_FIXINGDATE
#define PRICING_DATA_FIXINGDATE				"FixingDate"			//  data name of fixing date
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
#define PRICING_DATA_ISAMORTIZE1STFRACTION	"IsAmortize1stFraction"	//  data name of amortize is first fraction or last fraction
#endif
#ifndef PRICING_DATA_ISFIXINGPAYMENTDATEBASE
#define PRICING_DATA_ISFIXINGPAYMENTDATEBASE	"IsFixingPaymentDateBase"	//  data name of fixing date basedate is paymentdate or not
#endif
#ifndef PRICING_DATA_CASHFLOWSLIDINGRULE
#define PRICING_DATA_CASHFLOWSLIDINGRULE	"CashFlowSlidingRule"	//  data name of slidingrule for cashflow calculation
#endif
#ifndef PRICING_DATA_CASHFLOWCALENDAR
#define PRICING_DATA_CASHFLOWCALENDAR		"CashFlowCalendar"		//  data name of calendar for cashflow calculation
#endif
#ifndef PRICING_DATA_CALLINFO
#define PRICING_DATA_CALLINFO				"CallInfo"				// Data Name of Call Information 
#endif
#ifndef PRICING_DATA_TRIGGERINFOS
#define PRICING_DATA_TRIGGERINFOS			"TriggerInfos"			// Data Name of Trigger Information 
#endif
#ifndef PRICING_DATA_INPUTTYPE
#define PRICING_DATA_INPUTTYPE				"InputType"				// Data Name of Input Type
#endif
#ifndef PRICING_DATA_ACTIONSLIDINGRULE
#define PRICING_DATA_ACTIONSLIDINGRULE		"ActionSlidingRule"		//  data name of action slidingrule
#endif
#ifndef PRICING_DATA_ACTIONCALENDAR
#define PRICING_DATA_ACTIONCALENDAR			"ActionCalendar"		//  data name of action calendar
#endif
#ifndef PRICING_DATA_ACTIONOFFSET
#define PRICING_DATA_ACTIONOFFSET			"ActionOffSet"			//  data name of action offset days
#endif
#ifndef PRICING_DATA_ACTIONFIXINGCALENDAR
#define PRICING_DATA_ACTIONFIXINGCALENDAR	"ActionFixingCalendar"	//  data name of calendar for action fixing date
#endif
#ifndef PRICING_DATA_ACTIONFIXINGSLIDINGRULE
#define PRICING_DATA_ACTIONFIXINGSLIDINGRULE	"ActionFixingSlidingRule"	//  data name of  slidingrule for action fixing date
#endif
#ifndef PRICING_DATA_ACTIONFIXINGSPECIALOFFSET
#define PRICING_DATA_ACTIONFIXINGSPECIALOFFSET	"ActionFixingSpecialOffSet"	//  data name of offset term for action fixing
#endif
#ifndef PRICING_DATA_ACTIONFIXINGSPECIALDAY
#define PRICING_DATA_ACTIONFIXINGSPECIALDAY		"ActionFixingSpecialDay"	//  data name of action fixing special day
#endif
#ifndef PRICING_DATA_ACTIONDATES
#define PRICING_DATA_ACTIONDATES			"ActionDates"			//  data name of action dates
#endif
#ifndef PRICING_DATA_EXPIRYDATES
#define PRICING_DATA_EXPIRYDATES			"ExpiryDates"			//  data name of expiry dates
#endif
#ifndef PRICING_DATA_EXTRACFOFFSET
#define PRICING_DATA_EXTRACFOFFSET			"ExtraCFOffSet"			//  data name of extra cf off set days
#endif
/*#ifndef PRICING_DATA_EXTRACFS
#define PRICING_DATA_EXTRACFS				"ExtraCFs"				//  data name of extra cf array
#endif*/
#ifndef PRICING_DATA_EXTRACFDATES
#define PRICING_DATA_EXTRACFDATES			"ExtraCFDates"			//  data name of extra cf dates
#endif
#ifndef PRICING_DATA_INDEXENTITY
#define PRICING_DATA_INDEXENTITY			"IndexEntity"			//  data name of reference to index  object
#endif
#ifndef PRICING_DATA_STARTTERM
#define PRICING_DATA_STARTTERM				"StartTerm"				//  data name of start term from some basedate
#endif
#ifndef PRICING_DATA_ENDTERM
#define PRICING_DATA_ENDTERM				"EndTerm"				//  data name of end term from some basedate
#endif
#ifndef PRICING_DATA_TARGETLEG
#define PRICING_DATA_TARGETLEG				"TargetLeg"				//  data name of trigger target leg
#endif
#ifndef PRICING_DATA_ACTIONTIMING
#define PRICING_DATA_ACTIONTIMING			"ActionTiming"			//  data name of action timing is arrear or not
#endif
#ifndef PRICING_DATA_ISACTIONDETAIL
#define PRICING_DATA_ISACTIONDETAIL			"IsActionDetail"		//  data name of action schedule is detail or not
#endif
#ifndef PRICING_DATA_ACTIONSTARTDATE
#define PRICING_DATA_ACTIONSTARTDATE		"ActionStartDate"		//  data name of start date
#endif
#ifndef PRICING_DATA_ACTIONENDDATE
#define PRICING_DATA_ACTIONENDDATE			"ActionEndDate"			//  data name of end date
#endif
#ifndef PRICING_DATA_ACTIONFREQUENCY
#define PRICING_DATA_ACTIONFREQUENCY		"ActionFrequency"		//  data name of frenquency of payment
#endif
#ifndef PRICING_DATA_ACTIONFIRSTODDDATE
#define PRICING_DATA_ACTIONFIRSTODDDATE		"ActionFirstOddDate"	//  data name of first odd date
#endif
#ifndef PRICING_DATA_ACTIONLASTODDDATE
#define PRICING_DATA_ACTIONLASTODDDATE		"ActionLastOddDate"		//  data name of last odd date
#endif
#ifndef PRICING_DATA_ACTIONDAY
#define PRICING_DATA_ACTIONDAY				"ActionDay"				//  data name of action day 
#endif
#ifndef PRICING_DATA_ACTIONSTARTTERM
#define PRICING_DATA_ACTIONSTARTTERM		"ActionStartTerm"		//  data name of start term from some basedate
#endif
#ifndef PRICING_DATA_ACTIONENDTERM
#define PRICING_DATA_ACTIONENDTERM			"ActionEndTerm"			//  data name of end term from some basedate
#endif
#ifndef PRICING_DATA_FIXEDTERMS
#define PRICING_DATA_FIXEDTERMS				"FixedTerms"			//  data name of fixed dates
#endif
#ifndef PRICING_DATA_FIXEDDATES
#define PRICING_DATA_FIXEDDATES				"FixedDates"			//  data name of fixed dates
#endif
#ifndef PRICING_DATA_FIXEDRATES
#define PRICING_DATA_FIXEDRATES				"FixedRates"			//  data name of fixed Rates
#endif
#ifndef PRICING_DATA_FIXEDRATE
#define PRICING_DATA_FIXEDRATE				"FixedRate"				//  data name of fixed rate
#endif
#ifndef PRICING_DATA_COUPONCHANGEINFO
#define PRICING_DATA_COUPONCHANGEINFO		"CouponChangeInfo"		//  data name of coupon change info
#endif
#ifndef PRICING_DATA_DENOMINATOR
#define PRICING_DATA_DENOMINATOR			"Denominator"			//  data name of Denominator
#endif
#ifndef PRICING_DATA_PIPSIZE
#define PRICING_DATA_PIPSIZE				"PipSize"				//  data name of PipSize
#endif
#ifndef PRICING_DATA_ROUNDFUNCTION
#define PRICING_DATA_ROUNDFUNCTION			"RoundFunction"			//  data name of round function 
#endif
#ifndef PRICING_DATA_ROUNDDIGIT
#define PRICING_DATA_ROUNDDIGIT				"RoundDigit"			//  data name of round digit 
#endif
#ifndef PRICING_DATA_OBSERVATIONFREQUENCY
#define PRICING_DATA_OBSERVATIONFREQUENCY	"ObservationFrequency"	//  data name of frenquency of observation
#endif
#ifndef PRICING_DATA_OBSERVATIONDAY
#define PRICING_DATA_OBSERVATIONDAY			"ObservationDay"		//  data name of observation day
#endif
#ifndef PRICING_DATA_INDEXTYPE
#define PRICING_DATA_INDEXTYPE				"IndexType"				//  data name of index type
#endif
#ifndef PRICING_CALIBRATION_DATAOTIONALCF
#define PRICING_CALIBRATION_DATAOTIONALCF				"NotionalCF"			//  data name of notional cf
#endif
#ifndef PRICING_DATA_EXTRACFFUNC
#define PRICING_DATA_EXTRACFFUNC			"ExtraCFFunc"			//  data name of extra cf function
#endif
#ifndef PRICING_DATA_COEFFICIENT
#define PRICING_DATA_COEFFICIENT			"Coefficient"		//  data name of coefficient
#endif
#ifndef PRICING_DATA_ISFUNDINGCHANGE
#define PRICING_DATA_ISFUNDINGCHANGE			"IsFundingChange"			//  data name of funding change flag
#endif
#ifndef PRICING_DATA_FUNDINGTARGETLEG
#define PRICING_DATA_FUNDINGTARGETLEG			"FundingTargetLeg"			//  data name of funding target leg number
#endif
#ifndef PRICING_DATA_BASEFUNDINGCURRENCY
#define PRICING_DATA_BASEFUNDINGCURRENCY		"BaseFundingCurrency"			//  data name of base funding currency
#endif
#ifndef PRICING_DATA_BASEFUNDINGSPREAD
#define PRICING_DATA_BASEFUNDINGSPREAD		"BaseFundingSpread"			//  data name of base funding spread
#endif
#ifndef PRICING_DATA_BASEFUNDINGSPREADS
#define PRICING_DATA_BASEFUNDINGSPREADS		"BaseFundingSpreads"			//  data name of base funding spreads
#endif
#ifndef PRICING_DATA_BASEFUNDINGNOTIONAL
#define PRICING_DATA_BASEFUNDINGNOTIONAL		"BaseFundingNotional"			//  data name of base funding notional
#endif
#ifndef PRICING_DATA_BASEFUNDINGNOTIONALS
#define PRICING_DATA_BASEFUNDINGNOTIONALS		"BaseFundingNotionals"			//  data name of base funding notional
#endif
#ifndef PRICING_DATA_BASEFUNDINGDAYCOUNT
#define PRICING_DATA_BASEFUNDINGDAYCOUNT		"BaseFundingDayCount"			//  data name of base funding daycount
#endif
#ifndef PRICING_DATA_BASEFUNDINGSLIDINGRULE
#define PRICING_DATA_BASEFUNDINGSLIDINGRULE		"BaseFundingSlidingRule"			//  data name of base funding slidingrule
#endif
#ifndef PRICING_DATA_BASEFUNDINGCALENDAR
#define PRICING_DATA_BASEFUNDINGCALENDAR		"BaseFundingCalendar"			//  data name of base funding calendar
#endif
#ifndef PRICING_DATA_BASEFUNDINGRATES
#define PRICING_DATA_BASEFUNDINGRATES		"BaseFundingRates"			//  data name of base funding rates
#endif
#ifndef PRICING_DATA_BASEFUNDINGINDEXDAYCOUNT
#define PRICING_DATA_BASEFUNDINGINDEXDAYCOUNT		"BaseFundingIndexDayCount"			//  data name of base funding index daycount
#endif
#ifndef PRICING_DATA_BASEFUNDINGINDEXDAYCOUNTS
#define PRICING_DATA_BASEFUNDINGINDEXDAYCOUNTS		"BaseFundingIndexDayCounts"			//  data name of base funding index daycount
#endif
#ifndef PRICING_DATA_BASEFUNDINGSETTLEMENTADJUSTRATIOS
#define PRICING_DATA_BASEFUNDINGSETTLEMENTADJUSTRATIOS		"BaseFundingSettlementAdjustRatios"			//  data name of base funding index daycount
#endif
#ifndef PRICING_DATA_MODIFIEDFUNDINGCURRENCY
#define PRICING_DATA_MODIFIEDFUNDINGCURRENCY		"ModifiedFundingCurrency"			//  data name of modified funding currency
#endif
#ifndef PRICING_DATA_MODIFIEDFUNDINGSPREADS
#define PRICING_DATA_MODIFIEDFUNDINGSPREADS		"ModifiedFundingSpreads"			//  data name of base modified spreads
#endif
#ifndef PRICING_DATA_MODIFIEDFUNDINGNOTIONAL
#define PRICING_DATA_MODIFIEDFUNDINGNOTIONAL		"ModifiedFundingNotional"			//  data name of modified funding notional
#endif
#ifndef PRICING_DATA_MODIFIEDFUNDINGNOTIONALS
#define PRICING_DATA_MODIFIEDFUNDINGNOTIONALS		"ModifiedFundingNotionals"			//  data name of modified funding notional
#endif
#ifndef PRICING_DATA_MODIFIEDFUNDINGDAYCOUNT
#define PRICING_DATA_MODIFIEDFUNDINGDAYCOUNT		"ModifiedFundingDayCount"			//  data name of modified funding daycount
#endif
#ifndef PRICING_DATA_MODIFIEDFUNDINGSLIDINGRULE
#define PRICING_DATA_MODIFIEDFUNDINGSLIDINGRULE		"ModifiedFundingSlidingRule"			//  data name of modified funding slidingrule
#endif
#ifndef PRICING_DATA_MODIFIEDFUNDINGCALENDAR
#define PRICING_DATA_MODIFIEDFUNDINGCALENDAR		"ModifiedFundingCalendar"			//  data name of modified funding calendar
#endif
#ifndef PRICING_DATA_MODIFIEDFUNDINGINDEXDAYCOUNT
#define PRICING_DATA_MODIFIEDFUNDINGINDEXDAYCOUNT		"ModifiedFundingIndexDayCount"			//  data name of base funding index daycount
#endif
#ifndef PRICING_DATA_MODIFIEDFUNDINGINDEXDAYCOUNTS
#define PRICING_DATA_MODIFIEDFUNDINGINDEXDAYCOUNTS		"ModifiedFundingIndexDayCounts"			//  data name of base funding index daycount
#endif
#ifndef PRICING_DATA_MODIFIEDFUNDINGSETTLEMENTADJUSTRATIOS
#define PRICING_DATA_MODIFIEDFUNDINGSETTLEMENTADJUSTRATIOS		"ModifiedFundingSettlementAdjustRatios"			//  data name of base funding index daycount
#endif
#ifndef PRICING_DATA_ISCHANGEDBYFUNDING
#define PRICING_DATA_ISCHANGEDBYFUNDING		"IsChangedByFunding"			//  data name of a flag that is true for already changed funding leg
#endif
#ifndef PRICING_DATA_ISMODIFIEDBYFUNDING
#define PRICING_DATA_ISMODIFIEDBYFUNDING		"IsModifiedByFunding"			//  data name of a flag that is true for already changed funding leg
#endif
#ifndef PRICING_DATA_EXTRACFFORINITIALEXCHANGE
#define PRICING_DATA_EXTRACFFORINITIALEXCHANGE	"ExtraCFForInitialExchange"	//  data name of ExtraCFForInitialExchange
#endif
#ifndef PRICING_DATA_FXFORINITIALEXCHANGE
#define PRICING_DATA_FXFORINITIALEXCHANGE	"FXForInitialExchange"	//  data name of FXForInitialExchange
#endif
#ifndef PRICING_DATA_FUNDINGSPREAD
#define PRICING_DATA_FUNDINGSPREAD	"FundingSpread"	//  data name of FundingSpread
#endif
#ifndef PRICING_DATA_OBSERVATIONDATES
#define PRICING_DATA_OBSERVATIONDATES		"ObservationDates"		//  data name of observation dates
#endif
#ifndef PRICING_DATA_OBSERVATIONRATES
#define PRICING_DATA_OBSERVATIONRATES		"ObservationRates"		//  data name of observation rates
#endif
#ifndef PRICING_DATA_OBSERVATIONOPERATOR
#define PRICING_DATA_OBSERVATIONOPERATOR	"ObservationOperator"	//  data name of observation operator
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
#define PRICING_DATA_RANGEACCRUESAMEOBSERVATIONDAYS	"RangeAccrueSameObservationDays"	//  data name of range accrue same observation
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
#ifndef PRICING_DATA_RANGEACCRUEOBSERVATIONOFFSET
#define PRICING_DATA_RANGEACCRUEOBSERVATIONOFFSET	"RangeAccrueObservationOffset"	//  data name of range accrue observation offset
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
#define	PRICING_DATA_RANGEACCRUESTRIKE		"RangeAccrueStrike"
#endif
#ifndef	PRICING_DATA_ISEXCLUDEANDCONDITION	
#define	PRICING_DATA_ISEXCLUDEANDCONDITION		"IsExcludeAndCondition"
#endif
#ifndef PRICING_DATA_ISDOUBLERANGE
#define PRICING_DATA_ISDOUBLERANGE     "IsDoubleRange"
#endif
#ifndef PRICING_DATA_RANGEACCRUEFIXEDRATE1
#define PRICING_DATA_RANGEACCRUEFIXEDRATE1     "DoubleRangeFixedRate1"
#endif
#ifndef PRICING_DATA_RANGEACCRUEFIXEDRATE2
#define PRICING_DATA_RANGEACCRUEFIXEDRATE2     "DoubleRangeFixedRate2"
#endif
#ifndef PRICING_DATA_RANGEACCRUEFIXEDRATE3
#define PRICING_DATA_RANGEACCRUEFIXEDRATE3     "DoubleRangeFixedRate3"
#endif
#ifndef	PRICING_DATA_BUSINESSDAYSBASE	
#define	PRICING_DATA_BUSINESSDAYSBASE		"IsBusinessDaysBase"
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
#ifndef PRICING_DATA_BASEFUNDINGFORECASTCURVE
#define PRICING_DATA_BASEFUNDINGFORECASTCURVE		"BaseFundingForecastCurve"	//  data name of BaseFundingForecastCurve
#endif
#ifndef PRICING_DATA_MODIFIEDFUNDINGFORECASTCURVE
#define PRICING_DATA_MODIFIEDFUNDINGFORECASTCURVE		"ModifiedFundingForecastCurve"	//  data name of ModifiedFundingForecastCurve
#endif
#ifndef PRICING_DATA_FUNDINGCHANGEINFO
#define PRICING_DATA_FUNDINGCHANGEINFO				"FundingChangeInfo"		//  data name of FundingChangeInfo
#endif
#ifndef PRICING_DATA_ISRENOTIONAL
#define PRICING_DATA_ISRENOTIONAL					"IsRenotional"
#endif
#ifndef PRICING_DATA_RENOTIONALFIXINGDATE
#define PRICING_DATA_RENOTIONALFIXINGDATE			"RenotionalFixingDate"
#endif
#ifndef PRICING_DATA_PATHENTITY
#define PRICING_DATA_PATHENTITY						"PathEntity"
#endif
#ifndef PRICING_DATA_COMPOUNDINGDAYCOUNT
#define PRICING_DATA_COMPOUNDINGDAYCOUNT         "CompoundingDayCount"        // data name of CompoundingDayCount
#endif
#ifndef PRICING_DATA_ISCALCEQUIVALENTSTRIKE
#define PRICING_DATA_ISCALCEQUIVALENTSTRIKE					"IsCalcEquivalentStrike"						//  data name of IsCalcEquivalentStrike
#endif
#ifndef PRICING_DATA_ISFORWARDROLL
#define PRICING_DATA_ISFORWARDROLL		"IsForwardRoll"						//  data name of IsForwardRoll
#endif
#ifndef PRICING_DATA_BASISCURVE
#define PRICING_DATA_BASISCURVE             "BasisCurve"                 //  data name of Basis
#endif
#ifndef PRICING_DATA_DATESFORINDEXGENERATE
#define PRICING_DATA_DATESFORINDEXGENERATE "DatesForIndexGenerate"  //  data name of DatesForIndexGenerate
#endif
#ifndef PRICING_DATA_INDEXGENERATEMETHOD
#define PRICING_DATA_INDEXGENERATEMETHOD		"IndexGenerateMethod"        // data name of IndexGenerateMethod
#endif
#ifndef PRICING_DATA_ISIRRMODEL
#define PRICING_DATA_ISIRRMODEL					"IsIRRModel"						//  data name of IsIRRModel
#endif
#ifndef PRICING_DATA_RENOTIONALCALENDAR
#define PRICING_DATA_RENOTIONALCALENDAR			"RenotionalCalendar"		//  data name of RenotionalCalendar
#endif
#ifndef PRICING_DATA_RENOTIONALOFFSET
#define PRICING_DATA_RENOTIONALOFFSET			"RenotionalOffSet"		//  data name of RenotionalOffSet
#endif
#ifndef PRICING_DATA_RENOTIONALTIMING
#define PRICING_DATA_RENOTIONALTIMING			"RenotionalTiming"		//  data name of RenotionalTiming
#endif
#ifndef PRICING_DATA_RENOTIONALSLIDINGRULE
#define PRICING_DATA_RENOTIONALSLIDINGRULE		"RenotionalSlidingRule"		//  data name of RenotionalSlidingRule
#endif
#ifndef	IR_CALIBRATION_DATA_ISRENOTIONALADJUST	
#define	IR_CALIBRATION_DATA_ISRENOTIONALADJUST	"IsRenotionalAdjust"         //  data name of IsRenotionalAdjust
#endif		
#ifndef	PRICING_DATA_FIRSTFIXINGRATE	
#define	PRICING_DATA_FIRSTFIXINGRATE	"FirstFixingRate"
#endif
#ifndef	PRICING_DATA_FIRSTFIXINGBASEDATE	
#define	PRICING_DATA_FIRSTFIXINGBASEDATE	"FirstFixingBaseDate"
#endif
#ifndef	PRICING_DATA_ROLLFREQUENCY	
#define	PRICING_DATA_ROLLFREQUENCY	"RollFrequency"         //  data name of RollFrequency
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
#ifndef PRICING_CALIBRATION_DATAOTIONALCFISFIXED
#define PRICING_CALIBRATION_DATAOTIONALCFISFIXED "NotionalCfIsFixed"
#endif
#ifndef PRICING_DATA_ISFIRSTNOTIONALRESET
#define PRICING_DATA_ISFIRSTNOTIONALRESET "IsFirstNotionalReset"
#endif
#ifndef PRICING_DATA_ISBONDFIXED
#define PRICING_DATA_ISBONDFIXED "IsBondFixed"
#endif
#ifndef CALIBRATION_DATA_COUPONEXRATE
#define CALIBRATION_DATA_COUPONEXRATE "CouponExRate"
#endif
#ifndef CALIBRATION_DATA_NOTIONALUNIT
#define CALIBRATION_DATA_NOTIONALUNIT "NotionalUnit"
#endif
#ifndef CALIBRATION_DATA_ISRENOTIONALSETUPFINISHED
#define CALIBRATION_DATA_ISRENOTIONALSETUPFINISHED "IsRenotionalSetupFinished"
#endif
#ifndef PRICING_DATA_ISBONDFUNDINGLEG
#define PRICING_DATA_ISBONDFUNDINGLEG		"IsBondFundingLeg"	//  data name of IsBondFundingLeg
#endif
#ifndef PRICING_DATA_BONDFUNDINGCURRENCY
#define PRICING_DATA_BONDFUNDINGCURRENCY		"BondFundingCurrency"	//  data name of BondFundingCurrency
#endif
#ifndef PRICING_DATA_DONOTMODIFYTRADE
#define PRICING_DATA_DONOTMODIFYTRADE		"DoNotModifyTrade"	//  data name of 
#endif
#ifndef PRICING_DATA_ISNONDELIVERABLELEG
#define PRICING_DATA_ISNONDELIVERABLELEG		"IsNonDeliverableLeg"	//  data name of IsNonDeliverableLeg used for NDS
#endif
#ifndef PRICING_DATA_ORIGINALCURRENCY
#define PRICING_DATA_ORIGINALCURRENCY		"OriginalCurrency"	//  data name of OriginalCurrency used for NDS
#endif

// mark for extra libor
#ifndef EXTRA_FIXEDRATE
#define EXTRA_FIXEDRATE	-9999
#endif

// Default velue for fixed rate
#ifndef DEFAULT_SMALL_FIXEDRATE
#define DEFAULT_SMALL_FIXEDRATE -9999
#endif


class AQLObject;
class AQLDataProcedure;
class AQLPriceDataManager;
class AQLDataMultiReference;
class AQLObjectPool;
class AQLDataHolder;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief Generate Cashlets and trigger/call schedule

*/
class LAPriceCashFlowGenerator : public AQLCoreProcedure
{
public:
    class CashletCreator;

//  LIFECYCLE
    // constructor
	LAPriceCashFlowGenerator();
    // destructor	
	virtual ~LAPriceCashFlowGenerator();
    // Check this class ID is the same or not	
	virtual bool                isTypeOf(function_t id) const;
    // Copy this class
	virtual AQLCoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%
    // Return class type
	virtual function_t			getType() const;
    // Generate cashlets and trigger/call schedule
	virtual void	            calibrateModel(const AQLDate& basedate, 
										AQLObject& object, 
										const AQLDataProcedure& att) const;
	// register dataValues that this class uses
	virtual void				registerData(AQLPriceDataManager& dm) const;
	// create funding change data
	void						createFundingChangeAttribute(AQLObject& trade, AQLObject& fundingleg, AQLObject& strleg, AQLObject& fginfo) const;

	// clear cashlets
	void						clearCashlets(AQLObject& leg) const;

    // calc payment dates
	void						calcPaymentDates(AQLObject& leg, 
												 DateVector& paymentdates,
												 DateVector& paymentdates_unadjust,
												 DateVector& startdates,
												 DateVector& startdates_unadjust,
												 DateVector& enddates,
												 DateVector& enddates_unadjust,
                                                 const AQLDate& start_date,
                                                 const AQLDate& end_date,
												 const AQLString& freq,
                                                 const AQLDate* first_odd_date,
                                                 const AQLDate* last_odd_date) const;

protected:
    // copy constructor
	LAPriceCashFlowGenerator(const LAPriceCashFlowGenerator& p);


	// Calculete payment dates and interest calculation dates
    void                        calcPaymentDates(AQLObject& leg, 
												 DateVector& payment_dates,
												 DateVector& payment_dates_unadjust,
												 DateVector& payment_start_dates,
												 DateVector& payment_end_dates,
                                                 DateVector& roll_dates,
                                                 DateVector& roll_dates_unadjust,
                                                 DateVector& roll_start_dates,
                                                 DateVector& roll_end_dates,
												 bool& is_first_stub,
												 bool& is_last_stub) const;

private:
    void                        modifyLegInfo(AQLObject& leg) const;
	// Create cashlets
	void						createCashlets(AQLObject& leg) const;  

	
	
	// Calculete notional correspond to each payment date
	void						calcNotionalArray(const AQLObject& leg,
												const unsigned int cfsize,
												DoubleArray& notional_array) const;

	// create coupon info object
	AQLObject*					createCouponInfo(const AQLString name,
												const AQLDate& advancepaymentdate,
												const AQLDate& paymentdate,
												const AQLDate& paymentdate_unadjust,
												const AQLDate& startdate,
												const AQLDate& enddate,
												const AQLObject& couponinfo,
												const AQLObject& leg,
												const DateVector& rolldates,
												const DateVector& rollstartdates,
												const DateVector& rollenddates,
												AQLString stubindextype = "",
												const double fixedrate = static_cast<double>(DEFAULT_SMALL_FIXEDRATE),
												const bool is_digital = false
												) const;
	AQLObject*					createCouponInfo(const AQLString name,
												const AQLDate& advancepaymentdate,
												const AQLDate& paymentdate,
												const AQLDate& paymentdate_unadjust,
												const AQLDate& startdate,
												const AQLDate& enddate,
												const AQLObject& couponinfo,
												const AQLObject& leg,
												AQLString stubindextype = "",
												const double fixedrate = static_cast<double>(DEFAULT_SMALL_FIXEDRATE),
												const bool is_digital = false
												) const;

	// create index info object
	AQLObject*					createIndexInfo(const AQLString name,
												const AQLDate& advancepaymentdate,
												const AQLDate& paymentdate,
												const AQLDate& paymentdate_unadjust,
												const AQLDate& startdate,
												const AQLDate& enddate,
												const AQLObject& indexinfo,
                                                const AQLObject& leg,
												const DateVector& rolldates,
												const DateVector& rollstartdates,
												const DateVector& rollenddates,
												const double fixedrate = static_cast<double>(DEFAULT_SMALL_FIXEDRATE)
												) const;

	// create range accrue index info object
	AQLObject*					createRangeAccrueIndexInfo(const AQLString name,
												const AQLDate& startdate,
												const AQLDate& enddate,
												const AQLObject& indexinfo) const;
	// create range accrue info object
	AQLObject*					createRangeAccrueInfo(const AQLString name, const AQLObject& rangeinfo, const AQLDate& startdate, 
											 const AQLDate& enddate, const bool is_arrear, const int cf_pos, 
											 const DateVector& payment_dates, const DateVector& payment_dates_unadj) const;

	// setup leg maturity
	void						setUpLegMaturity(AQLObject& trade) const;
	// setup call schedule
	void						setUpCallSchedule(AQLObject& trade) const;
	// setup trigger schedule
	void						setUpTriggerSchedule(AQLObject& trade) const;
	// setup trigger coupon change type
	void						setUpTriggerCpnChange(AQLObject& trade, AQLObject& trigger) const;
	// check start dates are same between both legs
	bool						isSameStartDates(const AQLDataMultiReference& legs) const;
	// check end dates are same between both legs
	bool						isSameEndDates(const AQLDataMultiReference& legs) const;
	// setup funding change
	void						setUpFundingChange(AQLObject& trade) const;
	// create funding spread
	void						createFundingSpread(DoubleMatrix &spreadmat, DoubleVector &fixedrates, 
														AQLObject& trade, AQLObject& fundingleg, AQLObject& strleg, AQLObject& fginfo) const;
	// ! setup renotional
	void						setUpRenotional( AQLObject& trade ) const;
	// ! create renotional
	void						createRenotional( AQLObject& trade, AQLObject& leg ) const;

	// ! create oddtrade object
	void						createOddTradeEntity(AQLObject& trade) const;

	// ! setup equivalent strike
	void						setUpEquivalentStrike(AQLObject& trade) const;

	// ! setup renotional info
	void						createRenotionalInfo(const AQLDate& advancepaymentdate,
										const AQLDate& paymentdate,
										const AQLDate& paymentdate_unadjust, 
										const AQLDate& startdate,
										const AQLDate& enddate,
										AQLObject& cashlet, AQLObject& leg) const;

	// consistency check for frquency
	bool						isConsistentFrequencyPair(const AQLString& rollfreq, const AQLString& payfreq) const;

	// get fixed dates from leg
	DateVector					getFixedDates(const AQLObject& leg) const;

	// detect existance of stub coupon
	void						detectStubCoupon(bool& is_first_stub,
                                        bool& is_last_stub,
	                                    const AQLString& freq,
										const AQLDate& start_date,
										const AQLDate& end_date,
										const AQLDate* first_odd_date,
										const AQLDate* last_odd_date,
										const bool isforwardroll,
										const DateVector& payment_dates_unadjust) const;

	// ! setup funding leg of strctured bonds
	void						setUpBondFundingLeg(AQLObject& trade) const;

	// setup for products with non-deliverable currencies
	void						setUpNonDeliverable(AQLObject& trade) const;
};


class LAPriceCashFlowGenerator::CashletCreator
{
public:
    CashletCreator(const LAPriceCashFlowGenerator* cf_gen_,
                   AQLObject* leg_);
    void setCfSize(const unsigned int s) { cf_size = s; }
    void setPyamentDates(const DateVector& p, const DateVector& pu){ payment_dates = p; payment_dates_unadj = pu; }
    void setStubType(const bool is_fstub, const bool is_lstub){ is_first_stub = is_fstub; is_last_stub = is_lstub; }
    void setup();
    bool getIsNotionalExchangeStart() const { return is_notional_exchange_start; }
    bool getIsNotionalExchangeEnd() const { return is_notional_exchange_end; }
    bool getIsCompRoll() const { return is_comp_roll; }
    bool getIsArrear() const { return is_arrear; }
    AQLObject* createCashlet(const size_t cf_pos,
                            const AQLString& name,
                            const AQLDate& start_date,
                            const AQLDate& end_date,
                            const DateVector& roll_payment_dates,
                            const DateVector& roll_start_dates,
                            const DateVector& roll_end_dates,
							const double fixed_rate = static_cast<double>(DEFAULT_SMALL_FIXEDRATE),
							const bool is_digital = false,
							const size_t cf_pos2 = 0,
							const AQLString* p_basename = 0);
    AQLObject* createNotionalCashlet(const size_t cf_pos,
                                    const AQLString& name);

protected:
    const LAPriceCashFlowGenerator* cf_gen;
    AQLObject* leg;
    AQLObjectPool* objectPool;

    bool is_comp_roll;
    size_t cf_size;
    DoubleArray notional_array;
    AQLString timing;
    bool is_arrear;
    bool is_notional_exchange_start, is_notional_exchange_end;
	bool is_first_stub, is_last_stub;
    AQLString name;
    const AQLDataMultiReference* coupon_infos;
    bool is_renotional;
    AQLString first_odd_idx_type, last_odd_idx_type;
    AQLDataHolder* currency, *fx_rate;
    DateVector payment_dates, payment_dates_unadj;
	bool is_notionalcfcoupon;
    const AQLDataMultiReference* notionalcfcoupon_infos;
};

