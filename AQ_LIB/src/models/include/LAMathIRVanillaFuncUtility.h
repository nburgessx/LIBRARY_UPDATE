#ifndef LAMathIRVanillaFuncUtility_h
#define LAMathIRVanillaFuncUtility_h

#ifdef __GNUG__
#pragma interface
#endif

#include "LADate.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include "LADataInstance.h"
#include "LAString.h"
#include "LAMathYieldCurve.h"
#include "ConstantDeclarations.h"
#include "LATime.h"
#include "LAFunction.h"
#include "LAMathSABR.h"
#include "LAMathAnalyticalFormula.h"
#include "LAAnalyticFormula.h"
//////////////////////////

#ifndef CMS_CA_BLACK
#define CMS_CA_BLACK "BLACK"
#endif
#ifndef CMS_CA_SABR
#define CMS_CA_SABR "SABR"
#endif
#ifndef CMS_CA_SABR_HAGAN
#define CMS_CA_SABR_HAGAN "SABR_HAGAN"
#endif
#ifndef CMS_CA_SABR_ANTONOV
#define CMS_CA_SABR_ANTONOV "SABR_ANTONOV"
#endif
#ifndef CMS_CA_ZERO
#define CMS_CA_ZERO "ZERO"
#endif
#ifndef DELAYED_CA
#define DELAYED_CA "DELAYED"
#endif

class AnalyticParam;
class LABlackScholesBase;
class AnalyticBKParam;

class calcBase
{
public:
	calcBase(){};
	virtual ~calcBase(){};

	virtual void calc( const double x, double& pre, double& deri) = 0;
};

class impliedVolFinder: public calcBase
{
public:
	impliedVolFinder(const double x3, AnalyticParam& param, LABlackScholesBase*& p1, LABlackScholesBase*& p2)
	:  _prem(x3), _param(param), _bs(p1), _vega(p2)
	{};
	~impliedVolFinder(){};

	double _prem;
	AnalyticParam& _param;
	LABlackScholesBase*& _bs;
	LABlackScholesBase*& _vega;

	virtual void calc( const double x, double& pre, double& deri);

};

class impliedStraddleVolFinder: public calcBase
{
public:
	impliedStraddleVolFinder(const double x3, AnalyticParam& param, LABlackScholesBase*& p1, LABlackScholesBase*& p2, LABlackScholesBase*& p3, LABlackScholesBase*& p4)
	:  _prem(x3), _param(param), _bscall(p1), _vegacall(p2),_bsput(p3), _vegaput(p4) 
	{};
	~impliedStraddleVolFinder(){};

	double _prem;
	AnalyticParam& _param;
	LABlackScholesBase*& _bscall;
	LABlackScholesBase*& _vegacall;
	LABlackScholesBase*& _bsput;
	LABlackScholesBase*& _vegaput;

	virtual void calc( const double x, double& pre, double& deri);

};

class fwdDeltaFinder: public calcBase
{
public:
	fwdDeltaFinder(const double x3, AnalyticParam& param, LABlackScholesBase*& p1, LABlackScholesBase*& p2)
	:  _target(x3), _param(param), _delta(p1), _gamma(p2)
	{};
	~fwdDeltaFinder(){};

	double _target;
	AnalyticParam& _param;
	LABlackScholesBase*& _delta;
	LABlackScholesBase*& _gamma;

	virtual void calc( const double x, double& pre, double& deri);

};

class LASABRCAIntegral: public LAFunction
{
public:
	LASABRCAIntegral(double S0_,double T_,const vector<double>& sabr_params, double threshold_);
	~LASABRCAIntegral();
	void setIntegralParams(const double start, const double step, size_t num);
	double integralVol(unsigned int num) const;
	double operator()(double x) const;
private:
	static const unsigned int MAX_INTEGRALNUM = 20;
    double mS0;
	double mT;
	double mT_sqr;
    double mAlpha;
    double mBeta;
    double mNu;
    double mRho;
	double mBeta_m;
	double mTmp1;
	double mTmp2;
	double mTmp3;
	double mIntegralStart;
	double mIntegralStep;
	size_t mIntegralNum;
	double mWeights[MAX_INTEGRALNUM];
	double mAbscissas[MAX_INTEGRALNUM];
	double mThreshold;
 
	LAMathSABR* sabr;
};

class LAMathIRVanillaFuncUtility
{
public:
	static double bkOption(LAString& optiontype, LAString& buysell, LAString& callput,
                           double futureprice, double strike, double vol, 
                           double localrate,const LADate& basedate, 
                           const LADate& spotdate, const LADate& expirydate, const LADate& deliverydate);

	static double bkOption(LAString& optiontype, LAString& buysell, LAString& callput,
                           double futureprice, double strike, double vol, 
                           double localrate,const LADate& basedate, 
                           const LADate& spotdate, const LADate& expirydate, const LADate& deliverydate,
                           const LAString& calendar);

    static double bkOption(AnalyticBKParam& param, LAString& optiontype, LAString& buysell, LAString& callput, bool isUnitAdjust = true);

	static double bkOption(LAString optiontype, LAString buysell, LAString callput,
						   double futureprice, double strike, double vol,
							double NumeraireRatio, const LADate& basedate,const LADate& expirydate);

	static double bkOption(LAString optiontype, LAString buysell, LAString callput, double futureprice, 
						   double strike, double vol, double numeraire, double expiryterm, double localrate, bool isUnitAdjust = true);
	
	static double bkOptionIV(LAString& buysell, LAString& callput,
							double futureprice, double strike, double prem, 
							double localrate, const LADate& basedate, const LADate& spotdate, 
							const LADate& expirydate, const LADate& deliverydate,
							double high, double low, const bool isBusinessAdjust = false, const LAString& cal = "");

	static double bkOptionIV(const LAString& callput, double futureprice, double strike, double numeraire,
							 double expiryterm, double prem, double high, double low);

	static double bkOptionIV(const LAString& callput, double futureprice, double strike, double numeraire,
							 const LADate& basedate,const LADate& expirydate, double prem, double high, double low);

	static double bkOptionIV(AnalyticBKParam& param, const LAString& callput, double prem, double high, double low);

	static double capfloorOption(LADataInstance* dataInstance, LAString& caporfloor, const LAString& curveid, 
					   LAString& buysell,double nominal, double strike, double avevol, 
					   LAString& frequency, LAString& daycount,const DateVector& payVec, const LADate& fixingdate, 
					   const LADate& valuedate, LAString& paymentslidingrule, LAString& paymentcalendar, LAString& spotlag,  
					   LAString& fixingcalendar, bool iswithoutfirst = true, double firstfixingrate = 0.0,
					   double forwardrate = -10.0, LAString foreCurveName = STD, 
					   LAString dfCurveName = STD, const bool isFWDInter=true);

	static double capfloorOption_SABRParams(LADataInstance* dataInstance, LAString& caporfloor, const LAString& curveid,
					   LAString& buysell, double nominal, double strike, const LAString& alphaid, const LAString& betaid, const LAString& nuid, const LAString& rhoid,
					   LAString& frequency, LAString& daycount, const DateVector& payVec, const LADate& fixingdate,
					   const LADate& valuedate, LAString& paymentslidingrule, LAString& paymentcalendar, LAString& spotlag,
					   LAString& fixingcalendar, bool iswithoutfirst = true, double firstfixingrate = 0.0,
					   double forwardrate = -10.0, LAString foreCurveName = STD,
					   LAString dfCurveName = STD, const bool isFWDInter = true, double shiftvalue = 0.0);

	static double capfloorOption(LADataInstance* dataInstance, LAString& caporfloor, const LAString& curveid, 
					   LAString& buysell, double nominal, double strike, double avevol, 
					   LAString& frequency, LAString& daycount, const LADate& tradedate, const LADate& valuedate,
					   LAString& capterm, int roll, 
					   LAString& paymentslidingrule, LAString& paymentcalendar, LAString& spotlag,  
					   LAString& fixingcalendar, bool iswithoutfirst = true, double firstfixingrate = 0.0,
					   double forwardrate=-10.0, LAString foreCurveName = STD, 
					   LAString dfCurveName = STD, const bool isFWDInter=true);

	static double capfloorOption_SABRParams(LADataInstance* dataInstance, LAString& caporfloor, const LAString& curveid,
						LAString& buysell, double nominal, double strike, const LAString& alphaid, const LAString& betaid, const LAString& nuid, const LAString& rhoid,
						LAString& frequency, LAString& daycount, const LADate& tradedate, const LADate& valuedate, LAString& capterm, int roll,
						LAString& paymentslidingrule, LAString& paymentcalendar, LAString& spotlag,
						LAString& fixingcalendar, bool iswithoutfirst = true, double firstfixingrate = 0.0,
						double forwardrate = -10.0, LAString foreCurveName = STD,
						LAString dfCurveName = STD, const bool isFWDInter = true, double shiftvalue = 0.0);

	static double capfloorOptionVol(LADataInstance* dataInstance, LAString& caporfloor, const LAString& curveid, 
					    double strike, double prem, LAString& frequency, LAString& daycount,
						const DateVector& payVec, const LADate& fixingdate, const LADate& valuedate, 
					   LAString& paymentslidingrule, LAString& paymentcalendar, LAString& spotlag,  
					   LAString& fixingcalendar, double low, double high, 
					   bool iswithoutfirst= true, double firstfixingrate=0.0, double forwardrate=-10.0,
					   LAString foreCurveName = STD, LAString dfCurveName = STD, const bool isFWDInter=true);

	static double capfloorOptionVol(LADataInstance* dataInstance, LAString& caporfloor, const LAString& curveid, 
					   double strike, double prem, LAString& frequency, LAString& daycount,
					   const LADate& fixingdate, const LADate& valuedate,LAString& capterm, int roll, 
					   LAString& paymentslidingrule, LAString& paymentcalendar, LAString& spotlag,  
					   LAString& fixingcalendar, double low, double high,
					   bool iswithoutfirst= true, double firstfixingrate=0.0, double forwardrate=-10.0,
					   LAString foreCurveName = STD, LAString dfCurveName = STD, const bool isFWDInter=true);

	static double capFloorLet(LADataInstance* dataInstance,LAString& caporfloor, const LAString& curveid, LAString& buysell,
							  double strike, double vol, LAString& daycount, const LADate& tradedate,
							  const LADate& valuedate, const LADate& expirydate, const LADate& startdate, const LADate& enddate,
							  LAString foreCurveName, LAString dfCurveName, const bool isFwdInter=true);

	static double capFloorLetVol(LADataInstance* dataInstance,LAString& caporfloor, const LAString& curveid, 
								 double strike, double prem, LAString& daycount, const LADate& tradedate,
								 const LADate& valuedate, const LADate& expirydate, const LADate& startdate, const LADate& enddate,
								 double low, double high,
								 LAString foreCurveName, LAString dfCurveName);

	static double getAnnuity(const LADate& fromdate, LAString& term_str, const LAMathYieldCurve& yc);

	static double getCashSettledAnnuity(const LADate& fromdate, LAString& term_str, const LAMathYieldCurve& yc, const double swaprate);

	static double swaption(LADataInstance* dataInstance, const LAString& curveid, LAString& buysell, LAString& callput, 
									double nominal, double strike, double vol,
									const LADate& optiontradedate, const LADate& optionmaturitydate, const LADate& valuedate,
									LAString& spotlag, LAString& swapterm,
									LAString& frequency, LAString& slidingrule, LAString& daycount, LAString& calendar, 
									LAString& fixingcalendar, double forwardrate = -10.0, double forwardShift = 0.,
									LAString foreCurveName = STD, LAString dfCurveName = STD, 
									const bool isIRRModel = false);

	static double swaption(LADataInstance* dataInstance, const LAString& curveid, LAString& buysell, LAString& callput, 
						   double nominal, double strike, const LADate& tradedate, const LADate& valuedate,
						   LAString& optionmaturity, LAString& swapterm, double vol, 
					       LAString foreCurveName = STD, LAString dfCurveName = STD, 
						   const bool isIRRModel = false);

	static double swaptionVol(LADataInstance* dataInstance, const LAString& curveid, LAString& callput, 
							  double strike, double prem,
							  const LADate& optiontradedate, const LADate& optionmaturitydate, const LADate& valuedate,
							  LAString& spotlag, LAString& swapterm,
							  LAString& frequency, LAString& slidingrule, LAString& daycount, LAString& calendar, 
							  LAString& fixingcalendar, double& low, double& high, bool isBusinessAdjust = false, 
							  LAString foreCurveName = STD, LAString dfCurveName = STD,
                              const double* forwardRate=NULL, double forwardShift = 0.,
						      const bool isIRRModel = false);

	static double swaption_NormalDist(LADataInstance* dataInstance, const LAString& curveid, LAString& buysell, LAString& callput, 
									  double nominal, double strike, double vol,
									  const LADate& optiontradedate, const LADate& optionmaturitydate, const LADate& valuedate,
									  LAString& spotlag, LAString& swapterm,
									  LAString& frequency, LAString& slidingrule, LAString& daycount, LAString& calendar, 
									  LAString& fixingcalendar, double forwardrate = -10.0, 
									  LAString foreCurveName = STD, LAString dfCurveName = STD, 
									  const bool isIRRModel = false);


	static double floorletOption(const LAString& curveid, LAString& buysell, 
								 double libor, double strike, double vol,  
								 const LADate& basedate, const LADate& spotdate, const LADate& expirydate, const LADate& deliverydate);

	static double capletOption(	const LAString& curveid, LAString& buysell, 
								double libor, double strike, double vol,  
								const LADate& basedate, const LADate& spotdate, const LADate& expirydate, const LADate& deliverydate);

	static double getConvexityAdjust(double fwd, double tau, double delta, double fixTerm, double atmVol, unsigned int cfNum);

	static double getSABRConvexityAdjust2(double S0, double T, double tau, double delta, double num_swap, const DoubleArray& sabr_params, 
										 double int_regime, const DoubleArray& int_cond, size_t num_int_step = 20);
	static double getConvexityAdjust2(	LADataInstance* dataInstance, const LADate& asof, const LAString& curveID, const double& fwd, const double& fwdVol, const LADate& fixingDate, 
										const double& swapTerm, const unsigned int& paymentsPerYear, const DayCount& indexDaycount, const LAString* pCalendar = NULL,
										const LAString* pCurveType = NULL);

	static double getSABRConvexityAdjust(double S0, double T, double tau, double delta, double num_swap, const DoubleArray& sabr_params, 
										 double threshold, const DoubleArray& int_cond);

	// optimazation function retrived from numerical recipe. 
	// please note that this is just temporal implemantation.
	static double optimize(double x1, double x2,double x3, AnalyticParam& param, LABlackScholesBase*& p1, LABlackScholesBase*& p2);	
	static double optimize2(double x1, double x2, double x3, AnalyticParam& param, LABlackScholesBase*& p1, LABlackScholesBase*& p2);
	static double optimize(const double x1, const double x2);
	static double optimize(double x1, double x2, double x3, AnalyticParam& param, LABlackScholesBase*& p1, LABlackScholesBase*& p2, LABlackScholesBase*& p3, LABlackScholesBase*& p4);

	static int businessDaysBetween(const LADate& d1, const LADate& d2, const LAString& calendar);

private:
	LAMathIRVanillaFuncUtility(void);
	~LAMathIRVanillaFuncUtility(void);
	LAMathIRVanillaFuncUtility(const LAMathIRVanillaFuncUtility &rhs);
	LAMathIRVanillaFuncUtility &operator=(const LAMathIRVanillaFuncUtility &rhs);

    static const size_t busDayPerYear;
};


#endif 
