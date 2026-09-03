/*! @file
    @brief Class declaration to evaluate portfolio.

*/
#ifndef AQLPricePortfolioValue_h
#define AQLPricePortfolioValue_h


#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreValuation.h"
#include "AQLCoreAppError.h"
#include "AQLString.h"
#include "AQLCoreTemplateType.h"
#include <float.h>
#include <utility>

class AQLMathYieldCurve;

// AQLPricePortfolioValue's function id
#define FN_IR_PORTFOLIOVALUE				2011
// AQLPricePortfolioValue's function name
#define FN_IR_PORTFOLIOVALUE_STR			"fn_ir_portfoliovalue"

#ifdef ERROR
#undef	ERROR
#endif


#ifndef PRICING_DATA_RISKCALCINFOS		
#define PRICING_DATA_RISKCALCINFOS			"RiskCalcInfos"		//  data name of risk calculation informations
#endif
#ifndef PRICING_DATA_TARGETNAMES		
#define PRICING_DATA_TARGETNAMES			"TargetNames"		//  data name of target names of risk scenario
#endif
#ifndef PRICING_DATA_EXTRATARGETNAMES1		
#define PRICING_DATA_EXTRATARGETNAMES1		"ExtraTargetNames1"	//  data name of target names of risk extra scenario1
#endif
#ifndef PRICING_DATA_EXTRATARGETNAMES2		
#define PRICING_DATA_EXTRATARGETNAMES2		"ExtraTargetNames2"	//  data name of target names of risk extra scenario2
#endif
#ifndef PRICING_DATA_BASETARGETNAMES		
#define PRICING_DATA_BASETARGETNAMES		"BaseTargetNames"		//  data name of target names of risk scenario
#endif
#ifndef PRICING_DATA_BASEEXTRATARGETNAMES		
#define PRICING_DATA_BASEEXTRATARGETNAMES	"BaseExtraTargetNames"	  //  data name of risk target names base scenario
#endif
#ifndef PRICING_DATA_BASESCENARIO		
#define PRICING_DATA_BASESCENARIO			"BaseScenario"	  //  data name of risk base scenario
#endif
#ifndef PRICING_DATA_BASEEXTRASCENARIO		
#define PRICING_DATA_BASEEXTRASCENARIO		"BaseExtraScenario"	  //  data name of risk base extra scenario
#endif
#ifndef PRICING_DATA_SCENARIO1		
#define PRICING_DATA_SCENARIO1				"Scenario1"			//  data name of risk scenario1
#endif
#ifndef PRICING_DATA_EXTRASCENARIO1		
#define PRICING_DATA_EXTRASCENARIO1			"ExtraScenario1"	//  data name of risk extra scenario1
#endif
#ifndef PRICING_DATA_SCENARIO2		
#define PRICING_DATA_SCENARIO2				"Scenario2"			//  data name of risk scenario2
#endif
#ifndef PRICING_DATA_EXTRASCENARIO2		
#define PRICING_DATA_EXTRASCENARIO2			"ExtraScenario2"	//  data name of risk extra scenario2
#endif
#ifndef PRICING_DATA_BSDEOUTPUTNAME		
#define PRICING_DATA_BASEOUTPUTNAME			"BaseOutPutName"	//  data name of output data name of risk index 
#endif
#ifndef PRICING_DATA_OUTPUTNAME		
#define PRICING_DATA_OUTPUTNAME				"OutputName"		//  data name of output data name of risk index 
#endif
#ifndef PRICING_DATA_OUTPUTNAME2		
#define PRICING_DATA_OUTPUTNAME2			"OutputName2"		//  data name of output data name of risk index 
#endif
#ifndef PRICING_DATA_BASEOPERATOR
#define PRICING_DATA_BASEOPERATOR			"BaseOperator"		//  data name of base operator
#endif
#ifndef PRICING_DATA_WAVEOPERATOR
#define PRICING_DATA_WAVEOPERATOR			"WaveOperator"		//  data name of wave operator
#endif
#ifndef PRICING_DATA_OPERATOR
#define PRICING_DATA_OPERATOR				"Operator"			//  data name of operator
#endif
#ifndef PRICING_DATA_OPERATOR2
#define PRICING_DATA_OPERATOR2				"Operator2"			//  data name of operator2
#endif
#ifndef PRICING_DATA_BASECOEFFICIENT
#define PRICING_DATA_BASECOEFFICIENT		"BaseCoefficient"	//  data name of base coefficient
#endif
#ifndef PRICING_DATA_WAVECOEFFICIENT
#define PRICING_DATA_WAVECOEFFICIENT		"WaveCoefficient"	    //  data name of wave coefficient
#endif
#ifndef PRICING_DATA_COEFFICIENT
#define PRICING_DATA_COEFFICIENT			"Coefficient"		//  data name of coefficient
#endif
#ifndef PRICING_DATA_COEFFICIENT2
#define PRICING_DATA_COEFFICIENT2			"Coefficient2"		//  data name of coefficient2
#endif
#ifndef PRICING_DATA_ISGRIDSENSITIVITY
#define PRICING_DATA_ISGRIDSENSITIVITY		"IsGridSensitivity"	//  data name of grid sensitivity or not
#endif
#ifndef PRICING_DATA_ISCALCRISK
#define PRICING_DATA_ISCALCRISK				"IsCalcRisk"		//  data name of calculate risk index or not
#endif
#ifndef PRICING_DATA_ISWAVE
#define PRICING_DATA_ISWAVE				     "IsWave"		    //  data name of wave
#endif
#ifndef PRICING_DATA_VALUETYPE
#define PRICING_DATA_VALUETYPE				"ValueType"		//  data name of value type
#endif
#ifndef PRICING_DATA_CLEANPRICE
#define PRICING_DATA_CLEANPRICE				"CleanPrice"		//  data name of clean price
#endif
#ifndef PRICING_DATA_ERRORSTATUS
#define PRICING_DATA_ERRORSTATUS		    "ErrorStatus"		//  data name of error status
#endif
#ifndef PRICING_DATA_ISSETUPPAYOFF
#define PRICING_DATA_ISSETUPPAYOFF		    "IsSetUpPayOff"		//  data name of issetuppayoff
#endif
#ifndef PRICING_DATA_RISKOUTPUTCURRENCY
#define PRICING_DATA_RISKOUTPUTCURRENCY		"RiskOutputCurrency"	//  data name of RiskOutputCurrency
#endif
#ifndef PRICING_DATA_VALUATIONCURRENCY
#define PRICING_DATA_VALUATIONCURRENCY		"ValuationCurrency"		//  data name of ValuationCurrency
#endif
#ifndef PRICING_DATA_PREMIUMCURRENCY
#define PRICING_DATA_PREMIUMCURRENCY		"PremiumCurrency"				//  data name of PremiumCurrency
#endif
#ifndef PRICING_DATA_RISKCURVETYPENAME
#define PRICING_DATA_RISKCURVETYPENAME		"RiskCurveTypeName"				//  data name of RiskCurveTypeName
#endif
#ifndef PRICING_DATA_RISKBASESHIFTCURVETYPENAME
#define PRICING_DATA_RISKBASESHIFTCURVETYPENAME		"RiskBaseShiftCurveTypeName"	//  data name of RiskBaseShiftCurveTypeName
#endif
#ifndef PRICING_DATA_RISKCURVETYPECURRENCY
#define PRICING_DATA_RISKCURVETYPECURRENCY		"RiskCurveTypeCurrency"				//  data name of RiskCurveTypeCurrency
#endif
#ifndef PRICING_DATA_IMMRISKYIELDCURVENAME
#define PRICING_DATA_IMMRISKYIELDCURVENAME		"IMMRiskYieldCurveName"				//  data name of IMMRiskYieldCurveName
#endif
#ifndef PRICING_DATA_IMMFWDRATETERM
#define PRICING_DATA_IMMFWDRATETERM		"IMMFwdRateTerm"				//  data name of IMMFwdRateTerm
#endif
#ifndef PRICING_DATA_IMMRISKFLOORTERM
#define PRICING_DATA_IMMRISKFLOORTERM		"IMMRiskFloorTerm"					// data name of IMMRiskFloorTerm
#endif
#ifndef PRICING_DATA_IMMACTUALGRIDSHIFTVAL
#define PRICING_DATA_IMMACTUALGRIDSHIFTVAL		"IMMActualGridShiftVal"		// data name of IMMActualGridShiftVal
#endif
#ifndef PRICING_DATA_ISANALYTIC
#define PRICING_DATA_ISANALYTIC		"IsAnalytic"	//  data name of IsAnalytic
#endif
#ifndef PRICING_DATA_ISANALYTICALREADYCALCED
#define PRICING_DATA_ISANALYTICALREADYCALCED		"IsAnalyticAlreadyCalced"	//  data name of IsAnalyticAlreadyCalced
#endif
#ifndef PRICING_DATA_ZEROCALC
#define PRICING_DATA_ZEROCALC              "ZeroCalc"	//  data name of calculation switch flag
#endif
#ifndef PRICING_DATA_ANALYTICCALCTYPE
#define PRICING_DATA_ANALYTICCALCTYPE		"AnalyticCalcType"	//  data name of AnalyticCalcType
#endif
#ifndef PRICING_DATA_ISGRIDSENSITIVITY_ANALYTIC
#define PRICING_DATA_ISGRIDSENSITIVITY_ANALYTIC		"IsGridSensitivityForAnalytic"	//  data name of IsGridSensitivityForAnalytic
#endif
#ifndef PRICING_DATA_DIVUNIT_ANALYTIC
#define PRICING_DATA_DIVUNIT_ANALYTIC				"DivUnitForAnalytic"	//  data name of DivUnitForAnalytic
#endif
#ifndef PRICING_DATA_SHIFTTYPE_ANALYTIC
#define PRICING_DATA_SHIFTTYPE_ANALYTIC				"ShiftTypeForAnalytic"	//  data name of ShiftTypeForAnalytic
#endif
#ifndef PRICING_DATA_SHIFTVALS_ANALYTIC
#define PRICING_DATA_SHIFTVALS_ANALYTIC				"ShiftValsForAnalytic"	//  data name of ShiftValsForAnalytic
#endif
#ifndef PRICING_DATA_PREMIUMVALUE
#define PRICING_DATA_PREMIUMVALUE		"PremiumValue"				//  data name of PremiumValue
#endif
#ifndef PRICING_DATA_OPTIONVALUE
#define PRICING_DATA_OPTIONVALUE		"OptionValue"				//  data name of OptionValue
#endif
#ifndef PRICING_DATA_OMITNOTIONALEXPOSURE
#define PRICING_DATA_OMITNOTIONALEXPOSURE          "OmitNotionalExposure"	// data name of OmitNotionalExposure
#endif
#ifndef PRICING_DATA_ISADDFWDPREMPV
#define PRICING_DATA_ISADDFWDPREMPV	"IsAddFwdPremPV"	 // data name of flag for adding FwdPremium into PV
#endif
#ifndef PRICING_DATA_CASHSETTLEMENTCURRENCY
#define PRICING_DATA_CASHSETTLEMENTCURRENCY		"CashSettlementCurrency"				//  data name of CashSettlementCurrency
#endif
#ifndef AP_CALIBRATION_DATA_RISK_GRID
#define AP_CALIBRATION_DATA_RISK_GRID   "RiskGrid"				//  data name of RiskGrid
#endif
#ifndef PRICING_DATA_PV_FEE
#define PRICING_DATA_PV_FEE	"PV_Fee"
#endif
#ifndef PRICING_DATA_PVVALUE_FEE
#define PRICING_DATA_PVVALUE_FEE	"PVValue_Fee"
#endif
#ifndef PRICING_DATA_DF_FEE
#define PRICING_DATA_DF_FEE	"DF_Fee"
#endif
#ifndef PRICING_DATA_TODAYFX_FEE
#define PRICING_DATA_TODAYFX_FEE	"TodayFX_Fee"
#endif


//// FROTOTYPE ////
class AQLDate;
class AQLObject;
class AQLObjectPool;
class AQLCoreReferencePool;
class AQLDataMultiReference;
class AQLPriceDataManager;
class AQLMathObjectValue;
class AQLFunctionBase;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief class that evaluates portfolio

*/
class AQLPricePortfolioValue : public AQLCoreValuation
{
public:
    // constructor	
	AQLPricePortfolioValue();
    // destructor	
	virtual ~AQLPricePortfolioValue();
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
	AQLPricePortfolioValue & operator=( const AQLPricePortfolioValue & ) { return *this; }
private:
    // calculate pv of scenario cases
	void						calcPV(const AQLDate& basedate,
										std::vector<AQLMathObjectValue*> trades,
										const AQLStringVector& targets,
										AQLDataMultiReference& scenario, 
										bool isgrid,
										AQLObjectPool& objPool, 
										AQLCoreReferencePool& rpool,
										DoubleMatrix& output,
										std::vector<AQLStringVector>* ex_targets = 0,
										std::vector<AQLDataMultiReference*>* ex_scenarios = 0) const;
    // set risk value 
	void		setRiskValue(const AQLDataMultiReference &unders,
					const AQLDataMultiReference &scenario1, 
					const AQLDataMultiReference *pscenario2,
					const AQLFunctionBase &method,
					bool isgrid,
					const AQLString &outputname,
					const DoubleArray &pv_base, 
					const DoubleMatrix &pv_1,
					const DoubleMatrix &pv_2,
					const AQLObject &riskEntity) const;
   // set wave risk value 
	void			setWaveRiskValue(const AQLDataMultiReference &unders,
					const AQLDataMultiReference &scenario1, 
					const AQLDataMultiReference *pscenario2,
					const DoubleMatrix &coeffW,
					AQLFunctionBase &funcW,
					const AQLFunctionBase &method,
					const AQLString &outputname,
					const DoubleArray &pv_base, 
					const DoubleMatrix &pv_1,
					const DoubleMatrix &pv_2) const;

    // set scenario
    void		setScenario(const AQLString& name, 
				    AQLObject& scenario, 
				    AQLCoreReferencePool& rpool) const;
    // back to base market data
    void		backToBase(const AQLString& name, 
				   AQLObjectPool& objPool, 
				   AQLCoreReferencePool& rpool) const;

	static const double FAIL_VALUE;     // FAIL_VALUE
	static const AQLString ERROR;        // ERROR string
	static const AQLString PV_ERROR;     // PV_ERROR string
	static const unsigned int EXTRASCENARIO_MAX; // Extra Scenario max num

};
#endif
