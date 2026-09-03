#ifndef LAMathIRVanillaFuncUtility_h
#define LAMathIRVanillaFuncUtility_h

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLDate.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLDataInstance.h"
#include "AQLString.h"
#include "LAMathYieldCurve.h"
#include "ConstantDeclarations.h"
#include "LATime.h"
#include "AQLFunction.h"
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

class LASABRCAIntegral: public AQLFunction
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
	static double bkOption(AQLString& optiontype, AQLString& buysell, AQLString& callput,
                           double futureprice, double strike, double vol, 
                           double localrate,const AQLDate& basedate, 
                           const AQLDate& spotdate, const AQLDate& expirydate, const AQLDate& deliverydate);

	static double bkOption(AQLString& optiontype, AQLString& buysell, AQLString& callput,
                           double futureprice, double strike, double vol, 
                           double localrate,const AQLDate& basedate, 
                           const AQLDate& spotdate, const AQLDate& expirydate, const AQLDate& deliverydate,
                           const AQLString& calendar);

    static double bkOption(AnalyticBKParam& param, AQLString& optiontype, AQLString& buysell, AQLString& callput, bool isUnitAdjust = true);

	static double bkOption(AQLString optiontype, AQLString buysell, AQLString callput,
						   double futureprice, double strike, double vol,
							double NumeraireRatio, const AQLDate& basedate,const AQLDate& expirydate);

	static double bkOption(AQLString optiontype, AQLString buysell, AQLString callput, double futureprice, 
						   double strike, double vol, double numeraire, double expiryterm, double localrate, bool isUnitAdjust = true);
	
	static double bkOptionIV(AQLString& buysell, AQLString& callput,
							double futureprice, double strike, double prem, 
							double localrate, const AQLDate& basedate, const AQLDate& spotdate, 
							const AQLDate& expirydate, const AQLDate& deliverydate,
							double high, double low, const bool isBusinessAdjust = false, const AQLString& cal = "");

	static double bkOptionIV(const AQLString& callput, double futureprice, double strike, double numeraire,
							 double expiryterm, double prem, double high, double low);

	static double bkOptionIV(const AQLString& callput, double futureprice, double strike, double numeraire,
							 const AQLDate& basedate,const AQLDate& expirydate, double prem, double high, double low);

	static double bkOptionIV(AnalyticBKParam& param, const AQLString& callput, double prem, double high, double low);

	static double capfloorOption(AQLDataInstance* dataInstance, AQLString& caporfloor, const AQLString& curveid, 
					   AQLString& buysell,double nominal, double strike, double avevol, 
					   AQLString& frequency, AQLString& daycount,const DateVector& payVec, const AQLDate& fixingdate, 
					   const AQLDate& valuedate, AQLString& paymentslidingrule, AQLString& paymentcalendar, AQLString& spotlag,  
					   AQLString& fixingcalendar, bool iswithoutfirst = true, double firstfixingrate = 0.0,
					   double forwardrate = -10.0, AQLString foreCurveName = STD, 
					   AQLString dfCurveName = STD, const bool isFWDInter=true);

	static double capfloorOption_SABRParams(AQLDataInstance* dataInstance, AQLString& caporfloor, const AQLString& curveid,
					   AQLString& buysell, double nominal, double strike, const AQLString& alphaid, const AQLString& betaid, const AQLString& nuid, const AQLString& rhoid,
					   AQLString& frequency, AQLString& daycount, const DateVector& payVec, const AQLDate& fixingdate,
					   const AQLDate& valuedate, AQLString& paymentslidingrule, AQLString& paymentcalendar, AQLString& spotlag,
					   AQLString& fixingcalendar, bool iswithoutfirst = true, double firstfixingrate = 0.0,
					   double forwardrate = -10.0, AQLString foreCurveName = STD,
					   AQLString dfCurveName = STD, const bool isFWDInter = true, double shiftvalue = 0.0);

	static double capfloorOption(AQLDataInstance* dataInstance, AQLString& caporfloor, const AQLString& curveid, 
					   AQLString& buysell, double nominal, double strike, double avevol, 
					   AQLString& frequency, AQLString& daycount, const AQLDate& tradedate, const AQLDate& valuedate,
					   AQLString& capterm, int roll, 
					   AQLString& paymentslidingrule, AQLString& paymentcalendar, AQLString& spotlag,  
					   AQLString& fixingcalendar, bool iswithoutfirst = true, double firstfixingrate = 0.0,
					   double forwardrate=-10.0, AQLString foreCurveName = STD, 
					   AQLString dfCurveName = STD, const bool isFWDInter=true);

	static double capfloorOption_SABRParams(AQLDataInstance* dataInstance, AQLString& caporfloor, const AQLString& curveid,
						AQLString& buysell, double nominal, double strike, const AQLString& alphaid, const AQLString& betaid, const AQLString& nuid, const AQLString& rhoid,
						AQLString& frequency, AQLString& daycount, const AQLDate& tradedate, const AQLDate& valuedate, AQLString& capterm, int roll,
						AQLString& paymentslidingrule, AQLString& paymentcalendar, AQLString& spotlag,
						AQLString& fixingcalendar, bool iswithoutfirst = true, double firstfixingrate = 0.0,
						double forwardrate = -10.0, AQLString foreCurveName = STD,
						AQLString dfCurveName = STD, const bool isFWDInter = true, double shiftvalue = 0.0);

	static double capfloorOptionVol(AQLDataInstance* dataInstance, AQLString& caporfloor, const AQLString& curveid, 
					    double strike, double prem, AQLString& frequency, AQLString& daycount,
						const DateVector& payVec, const AQLDate& fixingdate, const AQLDate& valuedate, 
					   AQLString& paymentslidingrule, AQLString& paymentcalendar, AQLString& spotlag,  
					   AQLString& fixingcalendar, double low, double high, 
					   bool iswithoutfirst= true, double firstfixingrate=0.0, double forwardrate=-10.0,
					   AQLString foreCurveName = STD, AQLString dfCurveName = STD, const bool isFWDInter=true);

	static double capfloorOptionVol(AQLDataInstance* dataInstance, AQLString& caporfloor, const AQLString& curveid, 
					   double strike, double prem, AQLString& frequency, AQLString& daycount,
					   const AQLDate& fixingdate, const AQLDate& valuedate,AQLString& capterm, int roll, 
					   AQLString& paymentslidingrule, AQLString& paymentcalendar, AQLString& spotlag,  
					   AQLString& fixingcalendar, double low, double high,
					   bool iswithoutfirst= true, double firstfixingrate=0.0, double forwardrate=-10.0,
					   AQLString foreCurveName = STD, AQLString dfCurveName = STD, const bool isFWDInter=true);

	static double capFloorLet(AQLDataInstance* dataInstance,AQLString& caporfloor, const AQLString& curveid, AQLString& buysell,
							  double strike, double vol, AQLString& daycount, const AQLDate& tradedate,
							  const AQLDate& valuedate, const AQLDate& expirydate, const AQLDate& startdate, const AQLDate& enddate,
							  AQLString foreCurveName, AQLString dfCurveName, const bool isFwdInter=true);

	static double capFloorLetVol(AQLDataInstance* dataInstance,AQLString& caporfloor, const AQLString& curveid, 
								 double strike, double prem, AQLString& daycount, const AQLDate& tradedate,
								 const AQLDate& valuedate, const AQLDate& expirydate, const AQLDate& startdate, const AQLDate& enddate,
								 double low, double high,
								 AQLString foreCurveName, AQLString dfCurveName);

	static double getAnnuity(const AQLDate& fromdate, AQLString& term_str, const LAMathYieldCurve& yc);

	static double getCashSettledAnnuity(const AQLDate& fromdate, AQLString& term_str, const LAMathYieldCurve& yc, const double swaprate);

	static double swaption(AQLDataInstance* dataInstance, const AQLString& curveid, AQLString& buysell, AQLString& callput, 
									double nominal, double strike, double vol,
									const AQLDate& optiontradedate, const AQLDate& optionmaturitydate, const AQLDate& valuedate,
									AQLString& spotlag, AQLString& swapterm,
									AQLString& frequency, AQLString& slidingrule, AQLString& daycount, AQLString& calendar, 
									AQLString& fixingcalendar, double forwardrate = -10.0, double forwardShift = 0.,
									AQLString foreCurveName = STD, AQLString dfCurveName = STD, 
									const bool isIRRModel = false);

	static double swaption(AQLDataInstance* dataInstance, const AQLString& curveid, AQLString& buysell, AQLString& callput, 
						   double nominal, double strike, const AQLDate& tradedate, const AQLDate& valuedate,
						   AQLString& optionmaturity, AQLString& swapterm, double vol, 
					       AQLString foreCurveName = STD, AQLString dfCurveName = STD, 
						   const bool isIRRModel = false);

	static double swaptionVol(AQLDataInstance* dataInstance, const AQLString& curveid, AQLString& callput, 
							  double strike, double prem,
							  const AQLDate& optiontradedate, const AQLDate& optionmaturitydate, const AQLDate& valuedate,
							  AQLString& spotlag, AQLString& swapterm,
							  AQLString& frequency, AQLString& slidingrule, AQLString& daycount, AQLString& calendar, 
							  AQLString& fixingcalendar, double& low, double& high, bool isBusinessAdjust = false, 
							  AQLString foreCurveName = STD, AQLString dfCurveName = STD,
                              const double* forwardRate=NULL, double forwardShift = 0.,
						      const bool isIRRModel = false);

	static double swaption_NormalDist(AQLDataInstance* dataInstance, const AQLString& curveid, AQLString& buysell, AQLString& callput, 
									  double nominal, double strike, double vol,
									  const AQLDate& optiontradedate, const AQLDate& optionmaturitydate, const AQLDate& valuedate,
									  AQLString& spotlag, AQLString& swapterm,
									  AQLString& frequency, AQLString& slidingrule, AQLString& daycount, AQLString& calendar, 
									  AQLString& fixingcalendar, double forwardrate = -10.0, 
									  AQLString foreCurveName = STD, AQLString dfCurveName = STD, 
									  const bool isIRRModel = false);


	static double floorletOption(const AQLString& curveid, AQLString& buysell, 
								 double libor, double strike, double vol,  
								 const AQLDate& basedate, const AQLDate& spotdate, const AQLDate& expirydate, const AQLDate& deliverydate);

	static double capletOption(	const AQLString& curveid, AQLString& buysell, 
								double libor, double strike, double vol,  
								const AQLDate& basedate, const AQLDate& spotdate, const AQLDate& expirydate, const AQLDate& deliverydate);

	static double getConvexityAdjust(double fwd, double tau, double delta, double fixTerm, double atmVol, unsigned int cfNum);

	static double getSABRConvexityAdjust2(double S0, double T, double tau, double delta, double num_swap, const DoubleArray& sabr_params, 
										 double int_regime, const DoubleArray& int_cond, size_t num_int_step = 20);
	static double getConvexityAdjust2(	AQLDataInstance* dataInstance, const AQLDate& asof, const AQLString& curveID, const double& fwd, const double& fwdVol, const AQLDate& fixingDate, 
										const double& swapTerm, const unsigned int& paymentsPerYear, const DayCount& indexDaycount, const AQLString* pCalendar = NULL,
										const AQLString* pCurveType = NULL);

	static double getSABRConvexityAdjust(double S0, double T, double tau, double delta, double num_swap, const DoubleArray& sabr_params, 
										 double threshold, const DoubleArray& int_cond);

	// optimazation function retrived from numerical recipe. 
	// please note that this is just temporal implemantation.
	static double optimize(double x1, double x2,double x3, AnalyticParam& param, LABlackScholesBase*& p1, LABlackScholesBase*& p2);	
	static double optimize2(double x1, double x2, double x3, AnalyticParam& param, LABlackScholesBase*& p1, LABlackScholesBase*& p2);
	static double optimize(const double x1, const double x2);
	static double optimize(double x1, double x2, double x3, AnalyticParam& param, LABlackScholesBase*& p1, LABlackScholesBase*& p2, LABlackScholesBase*& p3, LABlackScholesBase*& p4);

	static int businessDaysBetween(const AQLDate& d1, const AQLDate& d2, const AQLString& calendar);

private:
	LAMathIRVanillaFuncUtility(void);
	~LAMathIRVanillaFuncUtility(void);
	LAMathIRVanillaFuncUtility(const LAMathIRVanillaFuncUtility &rhs);
	LAMathIRVanillaFuncUtility &operator=(const LAMathIRVanillaFuncUtility &rhs);

    static const size_t busDayPerYear;
};


#endif 
