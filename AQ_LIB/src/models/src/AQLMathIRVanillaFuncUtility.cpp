#include "AQLDate.h"

// (copied by AQLDate.cpp) 
static const unsigned short LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[2][2][12] =
    {{{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
      { 0, 31, 59, 90,120,151,181,212,243,273,304,334}},
     {{31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
      { 0, 31, 60, 91,121,152,182,213,244,274,305,335}}};

static const double INV_24 = 1.0 / 24;
static const double INV_1920 = 1.0 / 1920;
static const double MM_SQRT2 = 1.41421356237309504880168872420969807856967187537694;
static const double MM_1_SQRTPI = 0.564189583547756286948;

#if defined (WIN32) || defined (WIN64)

static const double a_[5] = {
    1.161110663653770e-002, 3.951404679838207e-001, 2.846603853776254e+001,
    1.887426188426510e+002, 3.209377589138469e+003
};
static const double b_[5] = {
    1.767766952966369e-001, 8.344316438579620e+000, 1.725514762600375e+002,
    1.813893686502485e+003, 8.044716608901563e+003
};
static const double c_[9] = {
    2.15311535474403846e-8, 5.64188496988670089e-1, 8.88314979438837594e00,
    6.61191906371416295e01, 2.98635138197400131e02, 8.81952221241769090e02,
    1.71204761263407058e03, 2.05107837782607147e03, 1.23033935479799725E03
};
static const double d_[9] = {
    1.00000000000000000e00, 1.57449261107098347e01, 1.17693950891312499e02,
    5.37181101862009858e02, 1.62138957456669019e03, 3.29079923573345963e03,
    4.36261909014324716e03, 3.43936767414372164e03, 1.23033935480374942e03
};
static const double p_[6] = {
    1.63153871373020978e-2, 3.05326634961232344e-1, 3.60344899949804439e-1,
    1.25781726111229246e-1, 1.60837851487422766e-2, 6.58749161529837803e-4
};
static const double q_[6] = {
    1.00000000000000000e00, 2.56852019228982242e00, 1.87295284992346047e00,
    5.27905102951428412e-1, 6.05183413124413191e-2, 2.33520497626869185e-3
};

#else


static const double a_[5] __attribute__ ((aligned(64))) = {
    1.161110663653770e-002, 3.951404679838207e-001, 2.846603853776254e+001,
    1.887426188426510e+002, 3.209377589138469e+003
};
static const double b_[5] __attribute__ ((aligned(64))) = {
    1.767766952966369e-001, 8.344316438579620e+000, 1.725514762600375e+002,
    1.813893686502485e+003, 8.044716608901563e+003
};
static const double c_[9] __attribute__ ((aligned(64))) = {
    2.15311535474403846e-8, 5.64188496988670089e-1, 8.88314979438837594e00,
    6.61191906371416295e01, 2.98635138197400131e02, 8.81952221241769090e02,
    1.71204761263407058e03, 2.05107837782607147e03, 1.23033935479799725E03
};
static const double d_[9] __attribute__ ((aligned(64))) = {
    1.00000000000000000e00, 1.57449261107098347e01, 1.17693950891312499e02,
    5.37181101862009858e02, 1.62138957456669019e03, 3.29079923573345963e03,
    4.36261909014324716e03, 3.43936767414372164e03, 1.23033935480374942e03
};
static const double p_[6] __attribute__ ((aligned(64))) = {
    1.63153871373020978e-2, 3.05326634961232344e-1, 3.60344899949804439e-1,
    1.25781726111229246e-1, 1.60837851487422766e-2, 6.58749161529837803e-4
};
static const double q_[6] __attribute__ ((aligned(64))) = {
    1.00000000000000000e00, 2.56852019228982242e00, 1.87295284992346047e00,
    5.27905102951428412e-1, 6.05183413124413191e-2, 2.33520497626869185e-3
};
#endif


#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "AQLMathIRVanillaFuncUtility.h"
#include "AQLFunctionUtilities.h"
#include "AQLMathCurveFuncUtility.h"
#include "AQLMathDateUtilities.h"
#include "AQLObject.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLCoreTemplateType.h"
#include "AQLMathDefine.h"
#include "AQLPriceDataCalendar.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLBasic.h"
#include "AQLAlgorithm.h"
#include "AQLSplineInterpolation.h"
#include "AQLStepInterpolation.h"
#include "AQLLinearInterpolation.h"
#include "AQLInterpolationBase.h"
#include "AQLPriceDataDayCount.h"
#include "AQLPriceCFGenUtility.h"
#include "AQLMathDateCalculations.h"
#include "AQLPriceYieldGenerator.h"
#include "AQLDataReference.h"
#include "AQLAnalyticFormula.h"
#include "AQLBlackScholesCalc.h"
#include "AQLMathYieldCurve.h"
#include "AQLMathYieldCurvePro.h"
#include "AQLDataProcedure.h"
#include "AQLPriceDataInterpolation.h"
#include "AQLCoreComponentManager.h"
#include "AQLPriceDataConvention.h"
#include "AQLDataMultiReference.h"
#include "AQLMathValuableEntity.h"
#include "AQLGaussLegendre.h"
#include "AQLMathSwaptionVolUtility.h"
#include <cmath>
#include <map>

using namespace std;

const size_t AQLMathIRVanillaFuncUtility::busDayPerYear = 250;


//////////////////////////Option method//////////////////////////////////////////////////////////
double 
AQLMathIRVanillaFuncUtility::bkOption(AnalyticBKParam& param, 
								   AQLString& optiontype, 
								   AQLString& buysell, 
								   AQLString& callput,
								   bool isUnitAdjust)
{
	upper(optiontype);
	upper(callput);
	upper(buysell);

	//Main sorce
	AQLString bscomponent = BK + optiontype  + callput;
	std::map<AQLString, AQLBlackScholesBase*> &var = AQLCoreComponentManager::getBlackComponentMap();
	std::map<AQLString, AQLBlackScholesBase*> ::iterator it = var.find(bscomponent);
	if(it==var.end())
		throw AQLCoreInvalidData("Option type is not supported",__FILE__,__LINE__);
	
	AQLBlackScholesBase* p = it->second;
	double ret=0.0;
	if(BUY==buysell)
		ret = p->calc(param);
	else if(SELL==buysell)
		ret = -1* p->calc(param);
	else 
		throw AQLCoreInvalidData("Choose Buy or Sell", __FILE__,__LINE__);

	if (isUnitAdjust)
		AQLMathBaseFuncUtility::adjustunit(ret,optiontype);

	return ret;
}

double 
AQLMathIRVanillaFuncUtility::bkOption(AQLString& optiontype, AQLString& buysell, AQLString& callput,
							       double futureprice, double strike, double vol, 
                                   double localrate,const AQLDate& basedate, 
                                   const AQLDate& spotdate, const AQLDate& expirydate, const AQLDate& deliverydate)
{
	AQLString daycount(AC_365I);

	AnalyticBKParam param;
	param.F   = futureprice;
	param.K	  = strike;
	param.Vol = vol;
	param.rd  = localrate;
	param.Te  = AQLMathDateUtilities::getTerm(basedate, expirydate, daycount, true);
	param.Td  = AQLMathDateUtilities::getTerm(spotdate, deliverydate, daycount, true);
	param.Nu  = AQLMath::exp(-param.rd * param.Td );
	param.ErrorCheck();	
	
    return bkOption(param, optiontype, buysell, callput);
}

double 
AQLMathIRVanillaFuncUtility::bkOption(AQLString optiontype,
								   AQLString buysell,
								   AQLString callput,
							       double futureprice,
								   double strike,
								   double vol,
								   double numeraire,
								   double expiryterm,
								   double localrate,
								   bool isUnitAdjust)
{
	AnalyticBKParam param;
	param.F   = futureprice;
	param.K	  = strike;
	param.Vol = vol;
	param.Te  = expiryterm;
	param.Nu  = numeraire;
	param.rd  = localrate;
	param.ErrorCheck();	
	
	return bkOption(param, optiontype, buysell, callput, isUnitAdjust);
}

double 
AQLMathIRVanillaFuncUtility::bkOption(AQLString& optiontype, AQLString& buysell, AQLString& callput,
							       double futureprice, double strike, double vol, 
                                   double localrate,const AQLDate& basedate, 
                                   const AQLDate& spotdate, const AQLDate& expirydate, const AQLDate& deliverydate,
                                   const AQLString& calendar)
{
	AnalyticBKParam param;
	param.F   = futureprice;
	param.K	  = strike;
	param.Vol = vol;
	param.rd  = localrate;
    param.Te  = (double)businessDaysBetween(basedate, expirydate, calendar) / AQLMathIRVanillaFuncUtility::busDayPerYear;
    param.Td  = (double)businessDaysBetween(basedate, deliverydate, calendar) / AQLMathIRVanillaFuncUtility::busDayPerYear;
	param.Nu  = AQLMath::exp(-param.rd * param.Td );
	param.ErrorCheck();	
	
    return bkOption(param, optiontype, buysell, callput);
}

double 
AQLMathIRVanillaFuncUtility::bkOption(AQLString optiontype, 
								   AQLString buysell, 
								   AQLString callput,
								   double futureprice, 
								   double strike, 
								   double vol,
								   double NumeraireRatio,
								   const AQLDate& basedate, 
								   const AQLDate& expirydate)
{
	//Change nospace & upper
	upper(optiontype);
	upper(callput);
	upper(buysell);
	AQLString daycount(AC_365I);

	//Type Select
	AQLString bscomponent;
	if (basedate < expirydate)
	{
		bscomponent = BK + optiontype  + callput;
	}
	else
	{
		bscomponent = BKPAYOFF + optiontype  + callput;
	}
	std::map<AQLString, AQLBlackScholesBase*> &var = AQLCoreComponentManager::getBlackComponentMap();
	std::map<AQLString, AQLBlackScholesBase*> ::iterator it = var.find(bscomponent);

	//Setup
	AnalyticBKParam param;
	param.F   = futureprice;
	param.K	  = strike;
	param.Vol = vol;
	param.Te  = AQLMathDateUtilities::getTerm(basedate,expirydate,daycount,true);
	param.Nu  = NumeraireRatio;
	param.ErrorCheck();	
	if (it==var.end())							throw AQLCoreInvalidData("Option type is not supported",__FILE__,__LINE__);
	if (buysell != BUY && buysell != SELL)		throw AQLCoreInvalidData("Choose Buy or Sell!", __FILE__,__LINE__);
	if (THETA == optiontype)						throw AQLCoreInvalidData("Option type is not supported",__FILE__,__LINE__);
	
	//Calculate
	AQLBlackScholesBase* p = it->second;
	double ret = p->calc(param);
	if(SELL==buysell) ret *= -1.0;
	AQLMathBaseFuncUtility::adjustunit(ret,optiontype);
	return ret;
}
//============================================================================

double 
AQLMathIRVanillaFuncUtility::bkOptionIV(AQLString& buysell, AQLString& callput,
							double futureprice, double strike, double prem, 
							double localrate, const AQLDate& basedate, const AQLDate& spotdate, 
							const AQLDate& expirydate, const AQLDate& deliverydate,
							double high, double low, const bool isBusinessAdjust, const AQLString& cal)
{
	//Change nospace & upper
	upper(callput);
	upper(buysell);
//	AQLString daycount(AC_365I);
	AQLString daycount(AC_365I);
	//Parameter
	AnalyticBKParam param;
	param.F   = futureprice;
	param.K	  = strike;
	param.rd  = localrate;
	param.Te  = AQLMathDateUtilities::getTerm(basedate,expirydate,   daycount, true);
	param.Td  = AQLMathDateUtilities::getTerm(spotdate,deliverydate, daycount, true);
	param.Nu  = AQLMath::exp(-param.rd * param.Td );
	param.ErrorCheck();
	//Ready for p->calc method1
	AQLString bscomponent = AQLString(BK) + AQLString(PREM)  + callput ;
	std::map<AQLString, AQLBlackScholesBase*> &var = AQLCoreComponentManager::getBlackComponentMap();
	std::map<AQLString, AQLBlackScholesBase*> ::iterator it = var.find(bscomponent);
	if(it==var.end())
		throw AQLCoreInvalidData("CallPut Type is not supported",__FILE__,__LINE__);
	AQLBlackScholesBase* p1 = it->second;

	//Ready for p->calc method2
	bscomponent = AQLString(BK) + AQLString(VEGA)  + callput ;
	var = AQLCoreComponentManager::getBlackComponentMap();
	it = var.find(bscomponent);
	AQLBlackScholesBase* p2 = it->second;

	//Optimize
	double ret = AQLMathIRVanillaFuncUtility::optimize(low, high, prem, param, p1, p2);//Bisection Method}

	if(isBusinessAdjust)
	{
		double std_Dev = ret * ret * param.Te;

		AQLPriceDataCalendar aCal;
		aCal.convertFromString( cal );		

		int horidayNum = aCal.getCalendar().countHoliday(basedate, expirydate);
		double Te_b = static_cast<double > ( basedate.intervalDays(expirydate) - horidayNum ) / 250.;
		ret = AQLMath::sqrt( std_Dev / Te_b );
	}

	return ret;
}

double 
AQLMathIRVanillaFuncUtility::bkOptionIV(const AQLString& callput,
									 double futureprice,
									 double strike,
									 double numeraire,
									 double expiryterm, 
									 double prem, 
									 double high, 
									 double low)
{
	AnalyticBKParam param;
	param.F   = futureprice;
	param.K	  = strike;
	param.Te  = expiryterm;
	param.Nu  = numeraire;
	param.ErrorCheck();	
	
	return bkOptionIV(param, callput, prem, high, low);
}

double 
AQLMathIRVanillaFuncUtility::bkOptionIV(const AQLString& callput,
									 double futureprice,
									 double strike,
									 double numeraire,
									 const AQLDate& basedate, 
									 const AQLDate& expirydate, 
									 double prem, 
									 double high, 
									 double low)
{
	AQLString daycount(AC_365I);

	AnalyticBKParam param;
	param.F   = futureprice;
	param.K	  = strike;
	param.Te  = AQLMathDateUtilities::getTerm(basedate,expirydate,daycount,true);
	param.Nu  = numeraire;
	param.ErrorCheck();	
	
	return bkOptionIV(param, callput, prem, high, low);
}

double 
AQLMathIRVanillaFuncUtility::bkOptionIV(AnalyticBKParam& param, const AQLString& callput, double prem, double high, double low)
{
	//Change nospace & upper
	AQLString callput_upper = callput;
	upper(callput_upper);

	//Ready for p->calc method1
	AQLString bscomponent = AQLString(BK) + AQLString(PREM)  + callput_upper ;
	std::map<AQLString, AQLBlackScholesBase*> &var = AQLCoreComponentManager::getBlackComponentMap();
	std::map<AQLString, AQLBlackScholesBase*> ::iterator it = var.find(bscomponent);
	if(it==var.end())
		throw AQLCoreInvalidData("CallPut Type is not supported",__FILE__,__LINE__);
	AQLBlackScholesBase* p1 = it->second;

	//Ready for p->calc method2
	bscomponent = AQLString(BK) + AQLString(VEGA)  + callput_upper ;
	var = AQLCoreComponentManager::getBlackComponentMap();
	it = var.find(bscomponent);
	AQLBlackScholesBase* p2 = it->second;

	//Optimize
	double ret = AQLMathIRVanillaFuncUtility::optimize(low, high, prem, param, p1, p2);//Bisection Method}

	return ret;
}

double 
AQLMathIRVanillaFuncUtility::getAnnuity(const AQLDate& fromdate, AQLString& term_str, const AQLMathYieldCurve& yc)
{
	//change nospace & upper
	upper(term_str);
	
	AQLDate toDate = AQLMathDateCalculations::getDate(fromdate, term_str, true);
	DateVector out;
	AQLMathDateCalculations::generateSchedule(fromdate, toDate, yc.getFrequency().get(),
							true, NULL, NULL, NULL, out, &yc.getSlidingRule(), &yc.getCalendar());
	if(out.front() != fromdate)
		out.insert(out.begin(), fromdate);

	double ret = yc.getAnnuity(out);
	return ret;
}

double 
AQLMathIRVanillaFuncUtility::getCashSettledAnnuity(const AQLDate& fromdate, AQLString& term_str, const AQLMathYieldCurve& yc, const double swaprate)
{
	//change nospace & upper
	upper(term_str);
	
	AQLDate toDate = AQLMathDateCalculations::getDate(fromdate, term_str, true);
	DateVector out;
	AQLString freq = yc.getFrequency().get();
	AQLMathDateCalculations::generateSchedule(fromdate, toDate, freq,
							true, NULL, NULL, NULL, out, &yc.getSlidingRule(), &yc.getCalendar());
	if(out.front() != fromdate)
		out.insert(out.begin(), fromdate);

	//irrmodel is unadjusted convention
	double unadjterm;
	if(freq == ANNUAL)
		unadjterm = 1.0;
	else if(freq == SEMI_ANNUAL)
		unadjterm = 6.0/12.0;
	else if(freq == QUARTERLY)
		unadjterm = 3.0/12.0;
	else if(freq == MONTHLY)
		unadjterm = 1.0/12.0;
	else
		throw AQLCoreInvalidData("Frequency is not supported",__FILE__,__LINE__);

	double ret = 0.0;
	double dfval = 1.0;
	for (unsigned int i = 0; i < out.size(); i++)
	{
		//notionalexchange cfcase
		if (fromdate == out[i])
		continue;

		dfval /= (1.0 + unadjterm * swaprate);
		ret += dfval * unadjterm;
		
	}

	return ret;
}

double 
AQLMathIRVanillaFuncUtility::capfloorOption(AQLDataInstance* dataInstance, AQLString& caporfloor, const AQLString& curveid, 
					   AQLString& buysell,double nominal, double strike, double avevol, 
					   AQLString& frequency, AQLString& daycount, const DateVector& payVec, const AQLDate& fixingdate, const AQLDate& valuedate, 
					   AQLString& paymentslidingrule, AQLString& paymentcalendar, AQLString& spotlag,  
					   AQLString& fixingcalendar, bool iswithoutfirst, double firstfixingrate,
					   double forwardrate, AQLString foreCurveName, AQLString dfCurveName, const bool isFWDInter)
{
	//change nospace & upper
	upper(caporfloor);
	upper(buysell);
	upper(frequency);
	upper(daycount);
	upper(spotlag);
	upper(paymentslidingrule);
	upper(paymentcalendar);
	upper(fixingcalendar);

	//slidingrule 
	AQLPriceDataSlidingRule sr2;
	sr2.convertFromString(PRE);

	//calendar
	AQLPriceDataCalendar cal2;
	cal2.convertFromString(fixingcalendar);

	//fixingVector
	int N=payVec.size();
	DateVector fixVec(N);
	for(int i =0; i<N;i++)
		fixVec[i] = AQLMathDateCalculations::getDate(payVec[i],spotlag,sr2,&cal2,false); 
	if(fixingdate > valuedate)
		fixVec[0] = fixingdate;

    /*AQLDate tradeDate;
    if(fixingdate > valuedate)
    {
        tradeDate = AQLMathDateCalculations::getDate(valuedate,spotlag,sr2,&cal2,false);
    }
    else
    {
        tradeDate = fixingdate;
    }*/

	daycount = AQLCoreComponentManager::getDayCount(daycount);

	//AQLMathYieldCurve
	AQLMathYieldCurve& yc = AQLMathCurveFuncUtility::getYieldCurveForCurveID(dataInstance,curveid);
	yc.setInterpolation(FN_SPLINEINTERPOLATION_STR);
	yc.getDayCount().setDayCount(daycount);
    //we do not slide dates in AQLMathYieldCurve class
	yc.getSlidingRule().convertFromString(NO_CH);
	AQLMathCurveFuncUtility::setCalendarForCurveID(yc,paymentcalendar);
	yc.getFrequency().convertFromString(frequency);
	//// Param
	AnalyticBKParam param;
	param.Vol = avevol;
	param.K = strike;
	//
	
	AQLObject YieldDate = yc.getYieldData().get().get();
	AQLPriceDataConvention conv(ACT_365_ISDA,CONT);
	AQLPriceDataConvention conv2(yc.getDayCount().getDayCount(),CONT);
	yc.getFrequency().set("SIMPLE");//To caluculate forward libor rate	
	
	//find the pos between fixVec[pos-1] < valuedate < fixVec[pos]
	//if valuedate < fixVec[0] pos =0 if valuedate > fixVec[end] return error!!
	unsigned int pos=0;
	AQLAlgorithm::locate<DateVector,AQLDate>(fixVec,valuedate,N-1,pos);
	if(N-1==static_cast<int>(pos))  
	{	
		AQLString msg = "valuedate : " + valuedate.stringWithFormat("YYYYMMDD")+" is not support for this option pricing";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);		
	}
	//if isfirstinclude = true then caluculate first CashFlow which has already 1st firxing rate.

	//Ready for p->calc method
	AQLString bscomponent;
	if(CAP	 == caporfloor)
		bscomponent = AQLString(BK)+AQLString(PREM)+AQLString(CALL);
	else if(FLOOR == caporfloor)
		bscomponent = AQLString(BK)+AQLString(PREM)+AQLString(PUT);
	else if(STRADDLE == caporfloor)
		bscomponent = AQLString(BK)+AQLString(PREM)+AQLString(CALL);
	std::map<AQLString, AQLBlackScholesBase*>& var2 = AQLCoreComponentManager::getBlackComponentMap();
	std::map<AQLString, AQLBlackScholesBase*>::iterator it2= var2.find(bscomponent);
	if(it2==var2.end())
		throw AQLCoreInvalidData("Product Type is not supported",__FILE__,__LINE__);
	AQLBlackScholesBase* p = it2->second;
	
	double ret=0.0;
	if(!iswithoutfirst&& (fixingdate > valuedate) )
		throw AQLCoreInvalidData("WithoutFirst is only when fixingdate = valuedate",__FILE__,__LINE__);
	if(!iswithoutfirst)
	{
		param.F			=	firstfixingrate;
		double del		=	(0==pos) ? conv2.getTerm(payVec[0],payVec[1]) : conv2.getTerm(payVec[pos-1],payVec[pos]);
		yc.setCurveType(dfCurveName);
		param.Nu		=	yc.getDF(valuedate,payVec[pos])*del;
		if(-1 != bscomponent.findString(PUT))
			ret+=	AQLMath::max((param.K-param.F)* param.Nu,0) ;
		else
			ret+=	AQLMath::max((param.F-param.K)*param.Nu,0);
	}
	for(int i=pos; i<N-1;i++)
	{
		if(-10.0==forwardrate)
		{
			yc.setCurveType(foreCurveName);
			param.F		= yc.getZeroRate(payVec[i],payVec[i+1],isFWDInter);
		}
		else
		{
			param.F		= forwardrate;
		}
		double del		= conv2.getTerm(payVec[i],payVec[i+1]);
		yc.setCurveType(dfCurveName);
		param.Nu		= yc.getDF(valuedate,payVec[i+1])*del;
		param.Te		= conv.getTerm(valuedate,fixVec[i]);
        //param.Te		= conv.getTerm(tradeDate,fixVec[i]);
		param.ErrorCheck();
		ret+= p->calc(param);
	}

	if(STRADDLE == caporfloor)
	{
		bscomponent	= AQLString(BK) + AQLString(PREM) + AQLString(PUT);
		p = var2.find(bscomponent)->second;
		if(!iswithoutfirst)
		{
			param.F			=	firstfixingrate;
			double del		=	(0==pos) ? conv2.getTerm(payVec[0],payVec[1]) : conv2.getTerm(payVec[pos-1],payVec[pos]);
			yc.setCurveType(dfCurveName);
			param.Nu		=	yc.getDF(valuedate,payVec[pos])*del;
			if(-1 != bscomponent.findString(PUT))
				ret+=	AQLMath::max((param.K-param.F)* param.Nu,0) ;
			else
				ret+=	AQLMath::max((param.F-param.K)*param.Nu,0);
		}
		for(int i=pos; i<N-1;i++)
		{
			if(-10.0==forwardrate)
			{
				yc.setCurveType(foreCurveName);
				param.F		= yc.getZeroRate(payVec[i],payVec[i+1],isFWDInter);
			}
			else
			{
				param.F		= forwardrate;
			}
			double del		= conv2.getTerm(payVec[i],payVec[i+1]);
			yc.setCurveType(dfCurveName);
			param.Nu		= yc.getDF(valuedate,payVec[i+1])*del;
            param.Te		= conv.getTerm(valuedate,fixVec[i]);
			//param.Te		= conv.getTerm(tradeDate,fixVec[i]);
			param.ErrorCheck();
			ret+= p->calc(param);
		}
	}

	if(BUY==buysell)
		ret *= nominal;
	else if(SELL==buysell)
		ret *= -nominal;
	else 
		throw AQLCoreInvalidData("Choose Buy or Sell", __FILE__,__LINE__); 
	
	return ret;
}
double
AQLMathIRVanillaFuncUtility::capfloorOption_SABRParams(AQLDataInstance* dataInstance, AQLString& caporfloor, const AQLString& curveid,
	AQLString& buysell, double nominal, double strike, const AQLString& alphaid, const AQLString& betaid, 
	const AQLString& nuid, const AQLString& rhoid, AQLString& frequency, AQLString& daycount, const DateVector& payVec, 
	const AQLDate& fixingdate, const AQLDate& valuedate, AQLString& paymentslidingrule, AQLString& paymentcalendar, 
	AQLString& spotlag, AQLString& fixingcalendar, bool iswithoutfirst, double firstfixingrate, double forwardrate, 
	AQLString foreCurveName, AQLString dfCurveName, const bool isFWDInter, double shiftvalue)
{
	//change nospace & upper
	upper(caporfloor);
	upper(buysell);
	upper(frequency);
	upper(daycount);
	upper(spotlag);
	upper(paymentslidingrule);
	upper(paymentcalendar);
	upper(fixingcalendar);

	//slidingrule 
	AQLPriceDataSlidingRule sr2;
	sr2.convertFromString(PRE);

	//calendar
	AQLPriceDataCalendar cal2;
	cal2.convertFromString(fixingcalendar);

	//fixingVector
	int N = payVec.size();
	DateVector fixVec(N);
	for (int i = 0; i<N; i++)
		fixVec[i] = AQLMathDateCalculations::getDate(payVec[i], spotlag, sr2, &cal2, false);
	if (fixingdate > valuedate)
		fixVec[0] = fixingdate;

	daycount = AQLCoreComponentManager::getDayCount(daycount);

	//tenor string
	AQLDataString tenor;
	if (frequency == "MONTHLY")
		tenor = "1M";
	else if (frequency == "QUARTERLY")
		tenor = "3M";
	else 
		tenor = "6M";
	AQLString tenorstr = dynamic_cast<AQLDataString &>(tenor);

	const AQLString approx_method = "Hagan";
	const AQLString conventionid;

	//AQLMathYieldCurve
	AQLMathYieldCurve& yc = AQLMathCurveFuncUtility::getYieldCurveForCurveID(dataInstance, curveid);
	yc.setInterpolation(FN_SPLINEINTERPOLATION_STR);
	yc.getDayCount().setDayCount(daycount);
	//we do not slide dates in AQLMathYieldCurve class
	yc.getSlidingRule().convertFromString(NO_CH);
	AQLMathCurveFuncUtility::setCalendarForCurveID(yc, paymentcalendar);
	yc.getFrequency().convertFromString(frequency);
	//// Param
	AnalyticBKParam param;
	param.K = strike + shiftvalue;
	//

	AQLObject YieldDate = yc.getYieldData().get().get();
	AQLPriceDataConvention conv(ACT_ACT, CONT);
	AQLPriceDataConvention conv2(yc.getDayCount().getDayCount(), CONT);
	yc.getFrequency().set("SIMPLE");

	AQLMathYieldCurve& yc_nu = yc;
	yc_nu.getSlidingRule().convertFromString(paymentslidingrule);
	yc_nu.getDayCount().setDayCount(ACT_ACT);

	unsigned int pos = 0;
	AQLAlgorithm::locate<DateVector, AQLDate>(fixVec, valuedate, N - 1, pos);
	if (N - 1 == static_cast<int>(pos))
	{
		AQLString msg = "valuedate : " + valuedate.stringWithFormat("YYYYMMDD") + " is not support for this option pricing";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	//if isfirstinclude = true then caluculate first CashFlow which has already 1st firxing rate.

	//Ready for p->calc method
	AQLString bscomponent;
	if (CAP == caporfloor)
		bscomponent = AQLString(BK) + AQLString(PREM) + AQLString(CALL);
	else if (FLOOR == caporfloor)
		bscomponent = AQLString(BK) + AQLString(PREM) + AQLString(PUT);
	else if (STRADDLE == caporfloor)
		bscomponent = AQLString(BK) + AQLString(PREM) + AQLString(CALL);
	std::map<AQLString, AQLBlackScholesBase*>& var2 = AQLCoreComponentManager::getBlackComponentMap();
	std::map<AQLString, AQLBlackScholesBase*>::iterator it2 = var2.find(bscomponent);
	if (it2 == var2.end())
		throw AQLCoreInvalidData("Product Type is not supported", __FILE__, __LINE__);
	AQLBlackScholesBase* p = it2->second;

	double ret = 0.0;
	if (!iswithoutfirst && (fixingdate > valuedate))
		throw AQLCoreInvalidData("WithoutFirst is only when fixingdate = valuedate", __FILE__, __LINE__);
	if (!iswithoutfirst)
	{
		param.F = firstfixingrate + shiftvalue;
		double del = (0 == pos) ? conv2.getTerm(payVec[0], payVec[1]) : conv2.getTerm(payVec[pos - 1], payVec[pos]);
		yc_nu.setCurveType(dfCurveName);
		param.Nu = yc_nu.getDF(valuedate, payVec[pos])*del;
		if (-1 != bscomponent.findString(PUT))
			ret += AQLMath::max((param.K - param.F)* param.Nu, 0);
		else
			ret += AQLMath::max((param.F - param.K)*param.Nu, 0);
	}
	for (int i = pos; i<N - 1; i++)
	{
		if (-10.0 == forwardrate)
		{
			yc.setCurveType(foreCurveName);
			param.F = yc.getZeroRate(payVec[i], payVec[i + 1], isFWDInter) + shiftvalue;
		}
		else
		{
			param.F = forwardrate + shiftvalue;
		}
		double del = conv2.getTerm(payVec[i], payVec[i + 1]);
		yc_nu.setCurveType(dfCurveName);
		param.Nu = yc_nu.getDF(valuedate, payVec[i + 1])*del;
		param.Te = conv.getTerm(valuedate, fixVec[i]);
		param.Vol = AQLMathSwaptionVolUtility::getSABRVol4(dataInstance, param.Te, tenorstr, param.F, param.K, alphaid, betaid, nuid, rhoid,
			curveid, conventionid, foreCurveName, dfCurveName, approx_method);
		
		param.ErrorCheck();
		ret += p->calc(param);
	}

	if (STRADDLE == caporfloor)
	{
		bscomponent = AQLString(BK) + AQLString(PREM) + AQLString(PUT);
		p = var2.find(bscomponent)->second;
		if (!iswithoutfirst)
		{
			param.F = firstfixingrate + shiftvalue;
			double del = (0 == pos) ? conv2.getTerm(payVec[0], payVec[1]) : conv2.getTerm(payVec[pos - 1], payVec[pos]);
			yc_nu.setCurveType(dfCurveName);
			param.Nu = yc_nu.getDF(valuedate, payVec[pos])*del;
			if (-1 != bscomponent.findString(PUT))
				ret += AQLMath::max((param.K - param.F)* param.Nu, 0);
			else
				ret += AQLMath::max((param.F - param.K)*param.Nu, 0);
		}
		// get cap/floor premium by summing up each caplet/floorlet premium
		for (int i = pos; i<N - 1; i++)
		{
			if (-10.0 == forwardrate)
			{
				yc.setCurveType(foreCurveName);
				param.F = yc.getZeroRate(payVec[i], payVec[i + 1], isFWDInter) + shiftvalue;
			}
			else
			{
				param.F = forwardrate +shiftvalue;
			}
			double del = conv2.getTerm(payVec[i], payVec[i + 1]);
			yc_nu.setCurveType(dfCurveName);
			param.Nu = yc_nu.getDF(valuedate, payVec[i + 1])*del;
			param.Te = conv.getTerm(valuedate, fixVec[i]);
			param.Vol = AQLMathSwaptionVolUtility::getSABRVol4(dataInstance, param.Te, tenorstr, param.F, param.K, alphaid, betaid, nuid, rhoid,
				curveid, conventionid, foreCurveName, dfCurveName, approx_method);
			
			param.ErrorCheck();
			ret += p->calc(param);
		}
	}

	if (BUY == buysell)
		ret *= nominal;
	else if (SELL == buysell)
		ret *= -nominal;
	else
		throw AQLCoreInvalidData("Choose Buy or Sell", __FILE__, __LINE__);

	return ret;
}
double 
AQLMathIRVanillaFuncUtility::capfloorOption(AQLDataInstance* dataInstance, AQLString& caporfloor, const AQLString& curveid, 
					   AQLString& buysell,double nominal, double strike, double avevol, 
					   AQLString& frequency, AQLString& daycount, const AQLDate& fixingdate, const AQLDate& valuedate,
					   AQLString& capterm, int roll, 
					   AQLString& paymentslidingrule, AQLString& paymentcalendar, AQLString& spotlag,  
					   AQLString& fixingcalendar, bool iswithoutfirst, double firstfixingrate,
					   double forwardrate, AQLString foreCurveName, AQLString dfCurveName, const bool isFWDInter)
				
{
	//change nospace & upper
	upper(caporfloor);
	upper(buysell);
	upper(frequency);
	upper(daycount);
	upper(capterm);
	upper(spotlag);
	upper(paymentslidingrule);
	upper(paymentcalendar);
	upper(fixingcalendar);
	
	//slidingrule 
	AQLPriceDataSlidingRule sr;
	sr.convertFromString(paymentslidingrule);
	//calendar
	AQLPriceDataCalendar cal;
	AQLPriceDataCalendar cal2;
	cal.convertFromString(paymentcalendar);
	cal2.convertFromString(fixingcalendar);

	//lastpaymentdate
	AQLDate tmpstart = AQLMathDateCalculations::getDate(fixingdate,spotlag,sr,&cal2,true);
	AQLDate endpay(tmpstart);
	int y=0,m=0,d=0,w=0;
	AQLMathDateCalculations::termStrtoYMDW(capterm,y,m,d,w);
	endpay.addYears(y);
	endpay.addMonths(m);
	endpay.addDays(d);
	//roll==0 ifonlyif spotlaginput is default
	if(0==roll)
		roll = tmpstart.dayOfMonth();	
	//roll set
	if (roll > LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[IS_LEAP_YEAR(endpay.yearOfEra())][0][endpay.monthOfYear() - 1])		
		endpay.setDay(LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[IS_LEAP_YEAR(endpay.yearOfEra())][0][endpay.monthOfYear() - 1]);
	else endpay.setDay(roll);
	
	//paymentvec;
	DateVector payVec; 
	AQLMathDateCalculations::generateSchedule(tmpstart,endpay,frequency,true,NULL,NULL,&roll,payVec,&sr,&cal);
	if(fixingdate > valuedate)
		payVec.insert(payVec.begin(),tmpstart);
	
    double ret = capfloorOption(dataInstance,caporfloor,curveid, buysell,nominal, strike, avevol, 
					    frequency,daycount,payVec, fixingdate, valuedate, 
					    paymentslidingrule, paymentcalendar, spotlag,  
					    fixingcalendar, iswithoutfirst, firstfixingrate,
					    forwardrate,foreCurveName, dfCurveName, isFWDInter);
	
	return ret;

}
double
AQLMathIRVanillaFuncUtility::capfloorOption_SABRParams(AQLDataInstance* dataInstance, AQLString& caporfloor, const AQLString& curveid,
	AQLString& buysell, double nominal, double strike, const AQLString& alphaid, const AQLString& betaid, const AQLString& nuid,
	const AQLString& rhoid, AQLString& frequency, AQLString& daycount, const AQLDate& fixingdate,
	const AQLDate& valuedate, AQLString& capterm, int roll, AQLString& paymentslidingrule, AQLString& paymentcalendar, 
	AQLString& spotlag, AQLString& fixingcalendar, bool iswithoutfirst, double firstfixingrate,
	double forwardrate, AQLString foreCurveName, AQLString dfCurveName, const bool isFWDInter, double shiftvalue)

{
	//change nospace & upper
	upper(caporfloor);
	upper(buysell);
	upper(frequency);
	upper(daycount);
	upper(capterm);
	upper(spotlag);
	upper(paymentslidingrule);
	upper(paymentcalendar);
	upper(fixingcalendar);

	//slidingrule 
	AQLPriceDataSlidingRule sr;
	sr.convertFromString(paymentslidingrule);
	//calendar
	AQLPriceDataCalendar cal;
	AQLPriceDataCalendar cal2;
	cal.convertFromString(paymentcalendar);
	cal2.convertFromString(fixingcalendar);

	//lastpaymentdate
	AQLString roll_conv = "TRUE";
	AQLDate tmpstart = AQLMathDateCalculations::getDate(fixingdate, spotlag, sr, &cal2, true, &roll_conv);
	AQLDate endpay(tmpstart);
	int y = 0, m = 0, d = 0, w = 0;
	AQLMathDateCalculations::termStrtoYMDW(capterm, y, m, d, w);
	endpay.addYears(y);
	endpay.addMonths(m);
	endpay.addDays(d);
	//roll==0 ifonlyif spotlaginput is default
	if (0 == roll)
		roll = tmpstart.dayOfMonth();
	//roll set
	if (roll > LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[IS_LEAP_YEAR(endpay.yearOfEra())][0][endpay.monthOfYear() - 1])
		endpay.setDay(LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[IS_LEAP_YEAR(endpay.yearOfEra())][0][endpay.monthOfYear() - 1]);
	else endpay.setDay(roll);

	//paymentvec;
	DateVector payVec;
	AQLMathDateCalculations::generateSchedule(tmpstart, endpay, frequency, true, NULL, NULL, &roll, payVec, &sr, &cal);
	if (fixingdate > valuedate)
		payVec.insert(payVec.begin(), tmpstart);

	double ret = capfloorOption_SABRParams(dataInstance, caporfloor, curveid, buysell, nominal, strike, alphaid, betaid, nuid, rhoid,
		frequency, daycount, payVec, fixingdate, valuedate,
		paymentslidingrule, paymentcalendar, spotlag,
		fixingcalendar, iswithoutfirst, firstfixingrate,
		forwardrate, foreCurveName, dfCurveName, isFWDInter, shiftvalue);

	return ret;

}
double 
AQLMathIRVanillaFuncUtility::capFloorLet
(AQLDataInstance* dataInstance,AQLString& caporfloor, const AQLString& curveid, AQLString& buysell,
 double strike, double vol, AQLString& daycount, const AQLDate& tradedate,
 const AQLDate& valuedate, const AQLDate& expirydate, const AQLDate& startdate, const AQLDate& enddate,
 AQLString foreCurveName, AQLString dfCurveName, const bool isFwdInter)
{
	//change nospace & upper
	upper(caporfloor);
	upper(buysell);
	upper(daycount);

	daycount = AQLCoreComponentManager::getDayCount(daycount);

	//AQLMathYieldCurve
	AQLMathYieldCurve& yc = AQLMathCurveFuncUtility::getYieldCurveForCurveID(dataInstance,curveid);
	yc.setInterpolation(FN_SPLINEINTERPOLATION_STR);
	yc.getDayCount().setDayCount(daycount);
	yc.getFrequency().set("SIMPLE");//To caluculate forward libor rate
    //we do not slide dates in AQLMathYieldCurve class
	yc.getSlidingRule().convertFromString(NO_CH);
	//// Param
	AnalyticBKParam param;
	param.Vol = vol;
	param.K = strike;
	
	AQLPriceDataConvention conv(ACT_365_ISDA,CONT);
	AQLPriceDataConvention conv2(yc.getDayCount().getDayCount(),CONT);
	
	if(tradedate>valuedate || expirydate > startdate || startdate>enddate || valuedate>startdate)  
	{	
		AQLString msg = "order of dates is not support for this option pricing";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);		
	}

	//Ready for p->calc method
	AQLString bscomponent;
	if(CAP	 == caporfloor)
		bscomponent = AQLString(BK)+AQLString(PREM)+AQLString(CALL);
	else if(FLOOR == caporfloor)
		bscomponent = AQLString(BK)+AQLString(PREM)+AQLString(PUT);
	else if(STRADDLE == caporfloor)
		bscomponent = AQLString(BK)+AQLString(PREM)+AQLString(CALL);
	std::map<AQLString, AQLBlackScholesBase*>& var2 = AQLCoreComponentManager::getBlackComponentMap();
	std::map<AQLString, AQLBlackScholesBase*>::iterator it2= var2.find(bscomponent);
	if(it2==var2.end())
		throw AQLCoreInvalidData("Product Type is not supported",__FILE__,__LINE__);
	AQLBlackScholesBase* p = it2->second;
	
	double ret=0.0;
	yc.setCurveType(foreCurveName);
	param.F	= yc.getZeroRate(startdate,enddate,isFwdInter);
	double del	= conv2.getTerm(startdate,enddate);
	yc.setCurveType(dfCurveName);
	param.Nu		= yc.getDF(valuedate,enddate)*del;
    param.Te		= conv.getTerm(tradedate,expirydate);
	param.ErrorCheck();
	ret+= p->calc(param);

	if(STRADDLE == caporfloor)
	{
		bscomponent	= AQLString(BK) + AQLString(PREM) + AQLString(PUT);
		p = var2.find(bscomponent)->second;
		yc.setCurveType(foreCurveName);
		param.F		= yc.getZeroRate(startdate,enddate);
		double del		= conv2.getTerm(startdate,enddate);
		yc.setCurveType(dfCurveName);
		param.Nu		= yc.getDF(valuedate,enddate)*del;
        param.Te		= conv.getTerm(tradedate,expirydate);
		param.ErrorCheck();
		ret+= p->calc(param);
	}

	if(BUY==buysell)
		ret *= 1.;
	else if(SELL==buysell)
		ret *= -1.;
	else 
		throw AQLCoreInvalidData("Choose Buy or Sell", __FILE__,__LINE__); 
	
	return ret;
}

double 
AQLMathIRVanillaFuncUtility::capFloorLetVol
(AQLDataInstance* dataInstance,AQLString& caporfloor, const AQLString& curveid, 
 double strike, double prem, AQLString& daycount, const AQLDate& tradedate,
 const AQLDate& valuedate, const AQLDate& expirydate, const AQLDate& startdate, const AQLDate& enddate,
 double low, double high,
 AQLString foreCurveName, AQLString dfCurveName)
{
	//change nospace & upper
	upper(caporfloor);
	upper(daycount);

	daycount = AQLCoreComponentManager::getDayCount(daycount);

	//AQLMathYieldCurve
	AQLMathYieldCurve& yc = AQLMathCurveFuncUtility::getYieldCurveForCurveID(dataInstance,curveid);
	yc.setInterpolation(FN_SPLINEINTERPOLATION_STR);
	yc.getDayCount().setDayCount(daycount);
    //we do not slide dates in AQLMathYieldCurve class
	yc.getSlidingRule().convertFromString(NO_CH);
	yc.getFrequency().set("SIMPLE");//To caluculate forward libor rate	
	
	AQLPriceDataConvention conv(ACT_365_ISDA,CONT);
	AQLPriceDataConvention conv2(yc.getDayCount().getDayCount(),CONT);
	
	if(tradedate>valuedate || expirydate > startdate || startdate>enddate || valuedate>startdate)  
	{	
		AQLString msg = "order of dates is not support for this option pricing";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);		
	}

	//Ready for p->calc method
	std::map<AQLString, AQLBlackScholesBase*> &var = AQLCoreComponentManager::getBlackComponentMap();
	AQLString bscomponent;
	if(CAP	 == caporfloor)
			bscomponent = AQLString(CF) + AQLString(PREM) + AQLString(CALL);
	else if(FLOOR == caporfloor)
			bscomponent = AQLString(CF) + AQLString(PREM) + AQLString(PUT);
	else if(STRADDLE == caporfloor)
			bscomponent = AQLString(CF) + AQLString(PREM) + AQLString(CALL);
	std::map<AQLString, AQLBlackScholesBase*> ::iterator it = var.find(bscomponent);
	if(it==var.end())
		throw AQLCoreInvalidData("Product Type is not supported",__FILE__,__LINE__);
	AQLBlackScholesBase* p1 = it->second;

	//Ready for p->calc method VEGA
	//std::map<AQLString, AQLBlackScholesBase*> &var = AQLCoreComponentManager::getBlackComponentMap();
	//AQLString bscomponent;
	if(CAP	 == caporfloor)
			bscomponent = AQLString(CF) + AQLString(VEGA) + AQLString(CALL);
	else if(FLOOR == caporfloor)
			bscomponent = AQLString(CF) + AQLString(VEGA) + AQLString(PUT);
	else if(STRADDLE == caporfloor)
			bscomponent = AQLString(CF) + AQLString(VEGA) + AQLString(CALL); 
	it = var.find(bscomponent);
	AQLBlackScholesBase* p2 = it->second;

	AQLBlackScholesBase* p3;
	AQLBlackScholesBase* p4;

	if(STRADDLE == caporfloor)
	{
		bscomponent = AQLString(CF) + AQLString(PREM) + AQLString(PUT);
		it = var.find(bscomponent);
		p3 = it->second;
		bscomponent = AQLString(CF) + AQLString(VEGA) + AQLString(PUT);
		it = var.find(bscomponent);
		p4 = it->second;
	}
	
	//Param
	AnalyticCFParam param;
	param.K = strike;
	param.iswithoutfirst = true;
	
	param.F.push_back(0.0);
	param.Nu.push_back(0.0);
	param.Te.push_back(0.0);
	param.ErrorCheck();
	
	yc.setCurveType(foreCurveName);
	param.F.push_back(yc.getZeroRate(startdate,enddate));

	double del = conv2.getTerm(startdate,enddate);
	yc.setCurveType(dfCurveName);
	param.Nu.push_back(yc.getDF(valuedate,enddate)*del);
	param.Te.push_back(conv.getTerm(tradedate,expirydate));
	param.ErrorCheck();
	
	if(STRADDLE==caporfloor)
		return optimize(low, high, prem, param, p1, p2, p3, p4); 
	else
		return optimize(low, high, prem, param, p1, p2);
}

double 
AQLMathIRVanillaFuncUtility::capfloorOptionVol(AQLDataInstance* dataInstance, AQLString& caporfloor, const AQLString& curveid, 
					    double strike, double prem, AQLString& frequency, AQLString& daycount,
						const DateVector& payVec, const AQLDate& fixingdate, const AQLDate& valuedate, 
					   AQLString& paymentslidingrule, AQLString& paymentcalendar, AQLString& spotlag,  
					   AQLString& fixingcalendar, double low, double high, bool iswithoutfirst, 
					   double firstfixingrate, double forwardrate, AQLString foreCurveName,
					   AQLString dfCurveName, const bool isFWDInter)
{
	//change nospace & upper
	upper(caporfloor);
	upper(frequency);
	upper(daycount);
	upper(spotlag);
	upper(paymentslidingrule);
	upper(paymentcalendar);
	upper(fixingcalendar);

	//slidingrule 
	AQLPriceDataSlidingRule sr2;
	sr2.convertFromString(PRE);

	//calendar
	AQLPriceDataCalendar cal2;
	cal2.convertFromString(fixingcalendar);

	//fixingVector
	int N=payVec.size();
	DateVector fixVec(N);
	for(int i =0; i<N;i++)
		fixVec[i]= AQLMathDateCalculations::getDate(payVec[i],spotlag,sr2,&cal2,false); 
	if(fixingdate > valuedate)
		fixVec[0] = fixingdate;

    /*AQLDate tradeDate;
    if(fixingdate > valuedate)
    {
        tradeDate = AQLMathDateCalculations::getDate(valuedate,spotlag,sr2,&cal2,false);
    }
    else
    {
        tradeDate = fixingdate;
    }*/

	daycount = AQLCoreComponentManager::getDayCount(daycount);

	//AQLMathYieldCurve
	AQLMathYieldCurve& yc = AQLMathCurveFuncUtility::getYieldCurveForCurveID(dataInstance,curveid);
	yc.setInterpolation(FN_SPLINEINTERPOLATION_STR);
	yc.getDayCount().setDayCount(daycount);
    //we do not slide dates in AQLMathYieldCurve class
	yc.getSlidingRule().convertFromString(NO_CH);
	AQLMathCurveFuncUtility::setCalendarForCurveID(yc,paymentcalendar);
	yc.getFrequency().convertFromString(frequency);
	
	AQLObject YieldDate = yc.getYieldData().get().get();
	AQLPriceDataConvention conv(ACT_365_ISDA,CONT);
	AQLPriceDataConvention conv2(yc.getDayCount().getDayCount(),CONT);
	yc.getFrequency().set("SIMPLE");//To caluculate forward libor rate	
	
	//find the pos between fixVec[pos-1] < valuedate < fixVec[pos]
	//if valuedate < fixVec[0] pos =0 if valuedate > fixVec[end] return error!!
	unsigned int pos=0;
	AQLAlgorithm::locate<DateVector,AQLDate>(fixVec,valuedate,N-1,pos);
	if(N-1==static_cast<int>(pos))  
	{	
		AQLString msg = "valuedate : " + valuedate.stringWithFormat("YYYYMMDD")+" is not support for this option pricing";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);		
	}
	//if isfirstinclude = true then caluculate first CashFlow which has already 1st firxing rate.

	//Ready for p->calc method
	std::map<AQLString, AQLBlackScholesBase*> &var = AQLCoreComponentManager::getBlackComponentMap();
	AQLString bscomponent;
	if(CAP	 == caporfloor)
			bscomponent = AQLString(CF) + AQLString(PREM) + AQLString(CALL);
	else if(FLOOR == caporfloor)
			bscomponent = AQLString(CF) + AQLString(PREM) + AQLString(PUT);
	else if(STRADDLE == caporfloor)
			bscomponent = AQLString(CF) + AQLString(PREM) + AQLString(CALL);
	std::map<AQLString, AQLBlackScholesBase*> ::iterator it = var.find(bscomponent);
	if(it==var.end())
		throw AQLCoreInvalidData("Product Type is not supported",__FILE__,__LINE__);
	AQLBlackScholesBase* p1 = it->second;

	//Ready for p->calc method VEGA
	//std::map<AQLString, AQLBlackScholesBase*> &var = AQLCoreComponentManager::getBlackComponentMap();
	//AQLString bscomponent;
	if(CAP	 == caporfloor)
			bscomponent = AQLString(CF) + AQLString(VEGA) + AQLString(CALL);
	else if(FLOOR == caporfloor)
			bscomponent = AQLString(CF) + AQLString(VEGA) + AQLString(PUT);
	else if(STRADDLE == caporfloor)
			bscomponent = AQLString(CF) + AQLString(VEGA) + AQLString(CALL); 
	it = var.find(bscomponent);
	AQLBlackScholesBase* p2 = it->second;

	AQLBlackScholesBase* p3;
	AQLBlackScholesBase* p4;

	if(STRADDLE == caporfloor)
	{
		bscomponent = AQLString(CF) + AQLString(PREM) + AQLString(PUT);
		it = var.find(bscomponent);
		p3 = it->second;
		bscomponent = AQLString(CF) + AQLString(VEGA) + AQLString(PUT);
		it = var.find(bscomponent);
		p4 = it->second;
	}
	
	//Param
	AnalyticCFParam param;
	param.K = strike;
	param.iswithoutfirst = iswithoutfirst;

	if(!iswithoutfirst&& (fixingdate > valuedate) )
		throw AQLCoreInvalidData("WithoutFirst is only when fixingdate = valuedate",__FILE__,__LINE__);
	if(!iswithoutfirst)
	{
		param.F.push_back(firstfixingrate);
		double del = (0==pos) ? conv2.getTerm(payVec[0],payVec[1]) : conv2.getTerm(payVec[pos-1],payVec[pos]);
		yc.setCurveType(dfCurveName);
		param.Nu.push_back(yc.getDF(valuedate,payVec[pos])*del);
		param.Te.push_back(0.0);
		param.ErrorCheck();
		
	}
	else
	{	
		param.F.push_back(0.0);
		param.Nu.push_back(0.0);
		param.Te.push_back(0.0);
		param.ErrorCheck();
	}

	for(int i = pos; i<N-1;i++)
	{	
		if(-10.0==forwardrate)
		{
			yc.setCurveType(foreCurveName);
			param.F.push_back(yc.getZeroRate(payVec[i],payVec[i+1],isFWDInter));
		}
		else 
		{
			param.F.push_back(forwardrate);
		}
			double del = conv2.getTerm(payVec[i],payVec[i+1]);
			yc.setCurveType(dfCurveName);
			param.Nu.push_back(yc.getDF(valuedate,payVec[i+1])*del);
			param.Te.push_back(conv.getTerm(valuedate,fixVec[i]));
            //param.Te.push_back(conv.getTerm(tradeDate,fixVec[i]));
			param.ErrorCheck();
	}	
	
	if(STRADDLE==caporfloor)
		return optimize(low, high, prem, param, p1, p2, p3, p4); 
	else
		return optimize(low, high, prem, param, p1, p2);
}

double 
AQLMathIRVanillaFuncUtility::capfloorOptionVol(AQLDataInstance* dataInstance, AQLString& caporfloor, const AQLString& curveid, 
					   double strike, double prem, AQLString& frequency, AQLString& daycount,
					   const AQLDate& fixingdate, const AQLDate& valuedate,AQLString& capterm, int roll, 
					   AQLString& paymentslidingrule, AQLString& paymentcalendar, AQLString& spotlag,  
					   AQLString& fixingcalendar, double low, double high,
					   bool iswithoutfirst, double firstfixingrate, double forwardrate, AQLString foreCurveName, 
					   AQLString dfCurveName, const bool isFWDInter)
{
	//change nospace & upper
	upper(caporfloor);
	upper(frequency);
	upper(daycount);
	upper(capterm);
	upper(spotlag);
	upper(paymentslidingrule);
	upper(paymentcalendar);
	upper(fixingcalendar);
	
	//slidingrule 
	AQLPriceDataSlidingRule sr;
	sr.convertFromString(paymentslidingrule);
	//calendar
	AQLPriceDataCalendar cal;
	AQLPriceDataCalendar cal2;
	cal.convertFromString(paymentcalendar);
	cal2.convertFromString(fixingcalendar);

	//lastpaymentdate
	AQLDate tmpstart = AQLMathDateCalculations::getDate(fixingdate,spotlag,sr,&cal2,true);
	AQLDate endpay(tmpstart);
	int y=0,m=0,d=0,w=0;
	AQLMathDateCalculations::termStrtoYMDW(capterm,y,m,d,w);
	endpay.addYears(y);
	endpay.addMonths(m);
	endpay.addDays(d);
	endpay.addWeeks(w);
	//roll==0 ifonlyif spotlaginput is default
	if(0==roll)
		roll = tmpstart.dayOfMonth();	
	//roll set
	if (roll > LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[IS_LEAP_YEAR(endpay.yearOfEra())][0][endpay.monthOfYear() - 1])		
		endpay.setDay(LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[IS_LEAP_YEAR(endpay.yearOfEra())][0][endpay.monthOfYear() - 1]);
	else endpay.setDay(roll);
	
	//paymentvec;
	DateVector payVec; 
	AQLMathDateCalculations::generateSchedule(tmpstart,endpay,frequency,true,NULL,NULL,&roll,payVec,&sr,&cal);
	if(fixingdate > valuedate)
		payVec.insert(payVec.begin(),tmpstart);
	
    double ret = capfloorOptionVol(dataInstance,caporfloor,curveid,strike, prem, 
					    frequency,daycount,payVec, fixingdate, valuedate, 
					    paymentslidingrule, paymentcalendar, spotlag,  
					    fixingcalendar, low, high, iswithoutfirst, firstfixingrate,forwardrate,foreCurveName,dfCurveName,isFWDInter);
	
	return ret;

}

double 
AQLMathIRVanillaFuncUtility::swaption(AQLDataInstance* dataInstance, const AQLString& curveid, AQLString& buysell, AQLString& callput, 
								   double nominal, double strike, const AQLDate& tradedate, const AQLDate& valuedate,
								   AQLString& optionmaturity, AQLString& swapterm, double vol, AQLString foreCurveName, 
								   const AQLString dfCurveName, const bool isIRRModel)
{
	//change nospace & upper
	upper(callput);
	upper(buysell);
	upper(optionmaturity);
	upper(swapterm);
	
	//AQLMathYieldCurve
	AQLString mod(MOD_FOL);
	AQLString cal("TKB:LNB");
	AQLString cal2("LNB");
	AQLString fre(SEMI_ANNUAL);
	AQLString offset("2d");
	AQLString daycount(AC_365);
		
	AQLDate tmpMDate = AQLMathDateUtilities::getDate(tradedate,optionmaturity,mod,cal);
	AQLPriceDataConvention conv(ACT_365_ISDA,CONT);
	
	double ret = AQLMathIRVanillaFuncUtility::swaption(dataInstance,curveid,buysell,callput,nominal,strike,vol,
													tradedate,tmpMDate,valuedate,offset,swapterm,
													fre,mod,daycount,cal,cal2,-10.,0.,foreCurveName,dfCurveName,isIRRModel);
	return ret;	
}

double 
AQLMathIRVanillaFuncUtility::swaption(AQLDataInstance* dataInstance, const AQLString& curveid, AQLString& buysell, AQLString& callput, 
									double nominal, double strike, double vol,
									const AQLDate& optiontradedate, const AQLDate& optionmaturitydate, const AQLDate& valuedate,
									AQLString& spotlag, AQLString& swapterm,
									AQLString& frequency, AQLString& slidingrule, AQLString& daycount, AQLString& calendar, 
									AQLString& fixingcalendar, double forwardrate, double forwardShift, AQLString foreCurveName, AQLString dfCurveName,
									const bool isIRRModel)
{
	//change nospace & upper
	upper(callput);
	upper(buysell);
	upper(frequency);
	upper(slidingrule);
	upper(daycount);
	upper(calendar);
	upper(spotlag);
	upper(swapterm);
	upper(fixingcalendar);
	AQLString interpolation(SPLINE);
	AQLString fol(FOL);
	
	//AQLMathYieldCurve
	AQLMathYieldCurve& yc = AQLMathCurveFuncUtility::getYieldCurveForCurveID(dataInstance,curveid);
	yc.setInterpolation(FN_SPLINEINTERPOLATION_STR);
	yc.getDayCount().setDayCount(AQLCoreComponentManager::getDayCount(daycount));
	yc.getSlidingRule().convertFromString(slidingrule);
	AQLMathCurveFuncUtility::setCalendarForCurveID(yc,calendar);
	yc.getFrequency().convertFromString(frequency);
	
	DayCount dc_act(ACT_365_ISDA);
	AQLPriceDataConvention conv(ACT_365_ISDA,CONT);
	double oterm = conv.getTerm(optiontradedate,optionmaturitydate); 
	//slidingRule is "FOLLOWING";
	AQLDate tmpDate = AQLMathDateUtilities::getDate(optionmaturitydate,spotlag,fol,fixingcalendar); 
	yc.setCurveType(dfCurveName);

	//Suppose that optiontradedate is as of date
	const AQLDate& asOfDate = dynamic_cast<const AQLDataDate& >(dataInstance->getObjectPool().getObject(curveid,ENCHKTYPE_ISDEFINED).
                                get().getData(CALIBRATION_DATA_ASOFDATE,ISNOTNULL).get()).get();
	double numeraire;
	if (!isIRRModel)
	{
		numeraire = AQLMathIRVanillaFuncUtility::getAnnuity(tmpDate,swapterm,yc);
	}
	else
	{
		numeraire = AQLMathIRVanillaFuncUtility::getCashSettledAnnuity(tmpDate,swapterm,yc,forwardrate);
		numeraire *= yc.getDF(conv.getTerm(asOfDate, tmpDate), &dc_act);

	}

	//changet the premium as the valuedate value.
	if(valuedate >= optionmaturitydate)
	{	
		AQLString msg = "valuedate : " + valuedate.stringWithFormat("YYYYMMDD") + " is not support for this option pricing";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);		
	}
    //if( asOfDate != optiontradedate ) throw AQLCoreInvalidData("as of date is not tradedate!", __FILE__, __LINE__);		
	oterm -= conv.getTerm(optiontradedate,valuedate);
    yc.getSlidingRule().convertFromString(NO_CH);
	yc.setCurveType(dfCurveName);
	numeraire /= yc.getDF(conv.getTerm(asOfDate, valuedate), &dc_act);
	//Param
	AnalyticBKParam param;
	if(-10.0==forwardrate)
		param.F = AQLMathCurveFuncUtility::getParRate(tmpDate,swapterm,dataInstance,curveid,frequency,daycount,slidingrule,calendar,
		interpolation,foreCurveName,dfCurveName) + forwardShift;
	else 
		param.F	= forwardrate + forwardShift;
	param.K = strike + forwardShift;
	param.F = max(param.F, 1e-7);
	param.K = max(param.K, 1e-7);
	param.Vol = vol;	
	param.Te = oterm;
	param.Nu = numeraire;
	param.ErrorCheck();
	//calc
	
	//Ready for p->calc method
	AQLString bscomponent;
	std::map<AQLString, AQLBlackScholesBase*> &var = AQLCoreComponentManager::getBlackComponentMap();
	if		(PAYERS == callput)
			bscomponent = AQLString(BK) + AQLString(PREM) + AQLString(CALL);
	else if(RECEIVERS==callput)
			bscomponent = AQLString(BK) + AQLString(PREM) + AQLString(PUT);
	else if(STRADDLE == callput)
			bscomponent = AQLString(BK) + AQLString(PREM) + AQLString(CALL);
		
	std::map<AQLString, AQLBlackScholesBase*> ::iterator it = var.find(bscomponent);
	if(it==var.end())
		throw AQLCoreInvalidData("Product Type is not supported",__FILE__,__LINE__);
	AQLBlackScholesBase* p = it->second;
	
	double ret = p->calc(param);
	if(STRADDLE == callput)
	{	
		bscomponent	= AQLString(BK) + AQLString(PREM) + AQLString(PUT);
		p = var.find(bscomponent)->second;
		ret+= p->calc(param);
	}
	if(BUY==buysell)
		ret *= nominal;
	else if(SELL==buysell)
		ret *= -nominal;
	else 
		throw AQLCoreInvalidData("Choose Buy or Sell", __FILE__,__LINE__); 
	return ret;
	
}

double 
AQLMathIRVanillaFuncUtility::swaptionVol(AQLDataInstance* dataInstance, const AQLString& curveid, AQLString& callput, 
									  double strike, double prem,
									  const AQLDate& optiontradedate, 
									  const AQLDate& optionmaturitydate, 
									  const AQLDate& valuedate,
									  AQLString& spotlag, AQLString& swapterm,
									  AQLString& frequency, AQLString& slidingrule, 
									  AQLString& daycount, AQLString& calendar, 
									  AQLString& fixingcalendar,
									  double& low, double& high, bool isBusinessAdjust,
									  AQLString foreCurveName, AQLString dfCurveName,
                                      const double* forwardRate, double forwardShift,
									  const bool isIRRModel)
{
	//change nospace & upper
	upper(callput);
	upper(frequency);
	upper(slidingrule);
	upper(daycount);
	upper(calendar);
	upper(spotlag);
	upper(swapterm);
	upper(fixingcalendar);
	AQLString interpolation(SPLINE);

	daycount = AQLCoreComponentManager::getDayCount(daycount);
	
	//Ready for p->calc method
	std::map<AQLString, AQLBlackScholesBase*> &var = AQLCoreComponentManager::getBlackComponentMap();
	AQLString bscomponent;
	if(PAYERS == callput)
			bscomponent = AQLString(BK) + AQLString(PREM) + AQLString(CALL);
	else if(RECEIVERS==callput)
			bscomponent = AQLString(BK) + AQLString(PREM) + AQLString(PUT);
	else if(STRADDLE==callput)
			bscomponent = AQLString(BK) + AQLString(PREM) + AQLString(CALL);
	std::map<AQLString, AQLBlackScholesBase*> ::iterator it = var.find(bscomponent);
	if(it==var.end())
		throw AQLCoreInvalidData("Product Type is not supported",__FILE__,__LINE__);
	AQLBlackScholesBase* p1 = it->second;

	
	//Ready for p->calc method VEGA
	//std::map<AQLString, AQLBlackScholesBase*> &var = AQLCoreComponentManager::getBlackComponentMap();
	//AQLString bscomponent;
	if(PAYERS == callput)
			bscomponent = AQLString(BK) + AQLString(VEGA) + AQLString(CALL);
	else if(RECEIVERS==callput)
			bscomponent = AQLString(BK) + AQLString(VEGA) + AQLString(PUT);
	else if(STRADDLE==callput)
			bscomponent = AQLString(BK) + AQLString(VEGA) + AQLString(CALL); 
	it = var.find(bscomponent);
	AQLBlackScholesBase* p2 = it->second;
	AQLBlackScholesBase* p3;
	AQLBlackScholesBase* p4;

	if(STRADDLE==callput)
	{
		bscomponent = AQLString(BK) + AQLString(PREM) + AQLString(PUT);
		it = var.find(bscomponent);
		p3 = it->second;
		bscomponent = AQLString(BK) + AQLString(VEGA) + AQLString(PUT);
		it = var.find(bscomponent);
		p4 = it->second;
	}

	//AQLMathYieldCurve
	AQLMathYieldCurve& yc = AQLMathCurveFuncUtility::getYieldCurveForCurveID(dataInstance,curveid);
	yc.setInterpolation(FN_SPLINEINTERPOLATION_STR);
	yc.getDayCount().setDayCount(daycount);
	yc.getSlidingRule().convertFromString(slidingrule);
	AQLMathCurveFuncUtility::setCalendarForCurveID(yc,calendar);
	yc.getFrequency().convertFromString(frequency);
	
	DayCount dc_act(ACT_365_ISDA);
	AQLPriceDataConvention conv(ACT_365_ISDA,CONT);
	double oterm = conv.getTerm(optiontradedate,optionmaturitydate); 
	//slidingRule is "FOLLOWING";
	AQLString fol(FOL);
	AQLDate tmpDate = AQLMathDateUtilities::getDate(optionmaturitydate,spotlag,fol,fixingcalendar);
	yc.setCurveType(dfCurveName);

	//Suppose that optiontradedate is as of date
    const AQLDate& asOfDate = dynamic_cast<const AQLDataDate& >(dataInstance->getObjectPool().getObject(curveid,ENCHKTYPE_ISDEFINED).
                                get().getData(CALIBRATION_DATA_ASOFDATE,ISNOTNULL).get()).get();

	double forward;
	if(forwardRate == NULL)
		forward = AQLMathCurveFuncUtility::getParRate(tmpDate,swapterm,dataInstance,curveid,frequency,daycount,slidingrule,calendar,interpolation,foreCurveName,dfCurveName);
	else
		forward = *forwardRate;

	double numeraire;
	if (!isIRRModel)
	{
		numeraire = AQLMathIRVanillaFuncUtility::getAnnuity(tmpDate,swapterm,yc);
	}
	else
	{
		numeraire = AQLMathIRVanillaFuncUtility::getCashSettledAnnuity(tmpDate,swapterm,yc,forward);
		numeraire *= yc.getDF(conv.getTerm(asOfDate, tmpDate), &dc_act);

	}

	//changet the premium as the valuedate value.
	if(valuedate >= optionmaturitydate)
	{	
		AQLString msg = "valuedate : " + valuedate.stringWithFormat("YYYYMMDD")+" is not support for this option pricing";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);		
	}
    //if( asOfDate != optiontradedate ) throw AQLCoreInvalidData("as of date is not tradedate!", __FILE__, __LINE__);		
	oterm -= conv.getTerm(optiontradedate,valuedate);
    yc.getSlidingRule().convertFromString(NO_CH);
	yc.setCurveType(dfCurveName);
	numeraire /= yc.getDF(conv.getTerm(asOfDate, valuedate), &dc_act);
	//Param
	AnalyticBKParam param;
	param.K = strike + forwardShift;
	//param.Vol = vol;	
	param.Te = oterm;
	param.Nu = numeraire;
	param.F = forward + forwardShift;
	param.ErrorCheck();

	double ret;
	if(STRADDLE==callput)
		ret = optimize(low, high, prem, param, p1, p2, p3, p4); 
	else
		ret = optimize(low, high, prem, param, p1, p2);

	if(isBusinessAdjust)
	{
		double std_Dev = ret * ret * param.Te;
		int horidayNum = yc.getCalendar().getCalendar().countHoliday(valuedate, optionmaturitydate);
		double Te_b = static_cast<double > ( valuedate.intervalDays(optionmaturitydate) - horidayNum ) / 245.;
		ret = AQLMath::sqrt( std_Dev / Te_b );
	}

	return ret;
}

double 
AQLMathIRVanillaFuncUtility::swaption_NormalDist(AQLDataInstance* dataInstance, const AQLString& curveid, AQLString& buysell, AQLString& callput, 
									          double nominal, double strike, double vol,
									          const AQLDate& optiontradedate, const AQLDate& optionmaturitydate, const AQLDate& valuedate,
									          AQLString& spotlag, AQLString& swapterm,
									          AQLString& frequency, AQLString& slidingrule, AQLString& daycount, AQLString& calendar, 
									          AQLString& fixingcalendar, double forwardrate, AQLString foreCurveName, AQLString dfCurveName,
									          const bool isIRRModel)
{
	//change nospace & upper
	upper(callput);
	upper(buysell);
	upper(frequency);
	upper(slidingrule);
	upper(daycount);
	upper(calendar);
	upper(spotlag);
	upper(swapterm);
	upper(fixingcalendar);
	AQLString interpolation(SPLINE);
	AQLString fol(FOL);
	
	//AQLMathYieldCurve
	AQLMathYieldCurve& yc = AQLMathCurveFuncUtility::getYieldCurveForCurveID(dataInstance,curveid);
	yc.setInterpolation(FN_SPLINEINTERPOLATION_STR);
	yc.getDayCount().setDayCount(AQLCoreComponentManager::getDayCount(daycount));
	yc.getSlidingRule().convertFromString(slidingrule);
	AQLMathCurveFuncUtility::setCalendarForCurveID(yc,calendar);
	yc.getFrequency().convertFromString(frequency);
	
	DayCount dc_act(ACT_365_ISDA);
	AQLPriceDataConvention conv(ACT_365_ISDA,CONT);
	double oterm = conv.getTerm(optiontradedate,optionmaturitydate); 
	//slidingRule is "FOLLOWING";
	AQLDate tmpDate = AQLMathDateUtilities::getDate(optionmaturitydate,spotlag,fol,fixingcalendar); 
	yc.setCurveType(dfCurveName);

	//Suppose that optiontradedate is as of date
	const AQLDate& asOfDate = dynamic_cast<const AQLDataDate& >(dataInstance->getObjectPool().getObject(curveid,ENCHKTYPE_ISDEFINED).
                                get().getData(CALIBRATION_DATA_ASOFDATE,ISNOTNULL).get()).get();
	double numeraire;
	if (!isIRRModel)
	{
		numeraire = AQLMathIRVanillaFuncUtility::getAnnuity(tmpDate,swapterm,yc);
	}
	else
	{
		numeraire = AQLMathIRVanillaFuncUtility::getCashSettledAnnuity(tmpDate,swapterm,yc,forwardrate);
		numeraire *= yc.getDF(conv.getTerm(asOfDate, tmpDate), &dc_act);

	}

	//changet the premium as the valuedate value.
	if(valuedate >= optionmaturitydate)
	{	
		AQLString msg = "valuedate : " + valuedate.stringWithFormat("YYYYMMDD") + " is not support for this option pricing";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);		
	}
    //if( asOfDate != optiontradedate ) throw AQLCoreInvalidData("as of date is not tradedate!", __FILE__, __LINE__);		
	oterm -= conv.getTerm(optiontradedate,valuedate);
    yc.getSlidingRule().convertFromString(NO_CH);
	yc.setCurveType(dfCurveName);
	numeraire /= yc.getDF(conv.getTerm(asOfDate, valuedate), &dc_act);

	//Param
	AnalyticBKParam param;
	if(-10.0==forwardrate)
	{
		forwardrate = AQLMathCurveFuncUtility::getParRate(tmpDate,swapterm,dataInstance,curveid,frequency,daycount,slidingrule,calendar,
		interpolation,foreCurveName,dfCurveName);
	}

	double ret(0.);
	double t_vol(sqrt(oterm)*vol);
	if		(PAYERS == callput)
	{
		ret = AQLMathAnalyticalFormula::BlackFormulaDD(forwardrate, t_vol, strike, 1, 0);
	}
	else if(RECEIVERS==callput)
	{
		ret = AQLMathAnalyticalFormula::BlackFormulaDD(forwardrate, t_vol, strike, -1, 0);
	}
	else if(STRADDLE == callput)
	{
		ret = AQLMathAnalyticalFormula::BlackFormulaDD(forwardrate, t_vol, strike, 1, 0);
		ret += AQLMathAnalyticalFormula::BlackFormulaDD(forwardrate, t_vol, strike, -1, 0);
	}
	ret *= numeraire;
	
	if(BUY==buysell)
		ret *= nominal;
	else if(SELL==buysell)
		ret *= -nominal;
	else 
		throw AQLCoreInvalidData("Choose Buy or Sell", __FILE__,__LINE__); 
	return ret;
	
}


//
/*!
	@brief CMS Convexity Adjustment Function

	@param[in] fwd     forward swap ate
	@param[in] tau     year flaction of accrual period
	@param[in] delta   accrual period of CMS
	@param[in] fixTerm fixing term of swap rate
	@param[in] atmVol  ATM Volatility of Swaption
    @param[in] cfNum   number of cash flow of swap rate

    @return Convexity Adjustment of CMS
*/
double 
AQLMathIRVanillaFuncUtility::
getConvexityAdjust(double fwd, 
				   double tau,
				   double delta, 
				   double fixTerm,
				   double atmVol,
				   unsigned int cfNum)
{
	// error check
	if(fwd<=0.)
	{
		throw AQLCoreInvalidData("Forward Swap Rate is negative!",__FILE__,__LINE__);
	}

	if(tau<=0. || delta<0.)
	{
		throw AQLCoreInvalidData("accrual term is negative!",__FILE__,__LINE__);
	}

	if(atmVol<0.)
	{
		throw AQLCoreInvalidData("vol is negative!",__FILE__,__LINE__);
	}

	if(cfNum<=0)
	{
		throw AQLCoreInvalidData("number of cash flow is negative!",__FILE__,__LINE__);
	}

	double theta = 1. - tau * fwd / (1. + tau * fwd) * (delta / tau + cfNum / (AQLMath::pow((1. + tau * fwd),cfNum) - 1.));
	return fwd * theta * (AQLMath::exp(atmVol * atmVol * fixTerm) - 1.);
}

//++++++++ Funahashi ++++++++
//
/*!
	@brief CMS Convexity Adjustment Function by SABR Model 2

	@param[in] S0			CMS rate
	@param[in] T			the start date of S0
	@param[in] tau			the daycount fraction of S0
	@param[in] delta		the daycount fraction of trade
	@param[in] num_swap		the number of cash flow of CMS
	@param[in] SABR_params	SABR_params = {alpha,beta,nu,rho}
	@param[in] threshold	threshold for SABR volatiltiy
	@param[in] int_cond		integral conditons: (lower bound ofintegral range, upper bound ofintegral range, integral steps)

    @return Convexity Adjustment of CMS 2, based on "A. Antonov, M. Konikov and M. Spectro, "SABR spreads its wings", Risk August 2013". 
*/
double 
AQLMathIRVanillaFuncUtility::
getSABRConvexityAdjust2(double S0,
						double T,
						double tau,
						double delta,
						double num_swap,
						const DoubleArray& sabr_params,
						double int_regime,
						const DoubleArray& int_cond,
						size_t num_int_step
						)
{
	if(sabr_params.size() != 4) throw AQLCoreInvalidData("SABR params.size() != 4",__FILE__,__LINE__);
	if(sabr_params[0] < 0) throw AQLCoreInvalidData("alpha must be positive",__FILE__,__LINE__);
    if(sabr_params[1] < 0 || 1 < sabr_params[1]) throw AQLCoreInvalidData("beta must be in (0,1)",__FILE__,__LINE__);
    if(sabr_params[2] < 0 ) throw AQLCoreInvalidData("nu must be possitive.",__FILE__,__LINE__);
    if(fabs(sabr_params[3]) > 1 ) throw AQLCoreInvalidData("rho must be in (-1,1).",__FILE__,__LINE__);

	if(S0 <= 0.0) throw AQLCoreInvalidData("Foward Rate must be positive!",__FILE__,__LINE__);
	if(T < 0.0) throw AQLCoreInvalidData("Time to Maturity must be non negative!",__FILE__,__LINE__);

	double theta = 1. - tau * S0 / (1. + tau * S0) * ( delta / tau + num_swap / (pow(1. + tau * S0,num_swap) - 1.) );

	/*vector<double> int_cond = XllPlusTips::getDoubleVector(int_cond_);*/
	double int_lb = int_cond[0]; double int_ub = int_cond[1]; double int_step = int_cond[2];

	const double integralStep = 1.0 / int_step;
	const double integralStart = int_lb;
	AQLMathConvexityAdust_Antonov swaption_prem(T, S0, sabr_params[0], sabr_params[1], sabr_params[2], sabr_params[3], num_int_step);

	double dt = (int_ub - int_lb) / int_step;
	double int_BS = 0.;
	double K_s = int_lb;
	double K_e = int_lb + dt;
	for(size_t i = 0; i < int_step; ++i)
	{	
		if( !swaption_prem.getFlg() )
		{
			int_BS += swaption_prem.IntegrateGLegendre(K_s, K_e, 20);
		} 
		K_s += dt;
		K_e += dt;
	}

	double CA = S0 * theta * (2. / S0 / S0 * int_BS - 1.);
	
	return CA;
}
//++++++++ Funahashi ++++++++

// uchida
//
/*!
	@brief CMS Convexity Adjustment Function

	@param[in] asof			asof date
	@param[in] curveID		curve id
	@param[in] fwd			forward swap rate
	@param[in] fwdVol		forward Volatility of Swaption
	@param[in] fixingDate	fixing term of swap rate
	@param[in] swapTerm		swap term
    @param[in] paymentsPerYear	number of cash flow of swap rate per year
	@param[in] indexdaycount index daycount

    @return Convexity Adjustment of CMS
*/
double 
AQLMathIRVanillaFuncUtility::
getConvexityAdjust2(AQLDataInstance* dataInstance, const AQLDate& asof, 
				   const AQLString& curveID,
				   const double& fwd,
				   const double& fwdVol,
				   const AQLDate& fixingDate,
				   const double& swapTerm,
				   const unsigned int& paymentsPerYear,
				   const DayCount& indexdaycount,
				   const AQLString* pCalendar,
				   const AQLString* pCurveType)
{
	// error check
	if(fwd<=0.)
	{
		throw AQLCoreInvalidData("Forward Swap Rate is negative!",__FILE__,__LINE__);
	}

	if(asof >= fixingDate)
	{
		throw AQLCoreInvalidData("fixing Date have been past!",__FILE__,__LINE__);
	}

	if(fwdVol<0.)
	{
		throw AQLCoreInvalidData("vol is negative!",__FILE__,__LINE__);
	}
	if(swapTerm<=0)
	{
		throw AQLCoreInvalidData("swapTenor is negative!",__FILE__,__LINE__);
	}

	if(paymentsPerYear<=0)
	{
		throw AQLCoreInvalidData("number of cash flow per year is negative!",__FILE__,__LINE__);
	}

	vector<double> termvec(swapTerm * paymentsPerYear);
	vector<double> gridvec(swapTerm * paymentsPerYear + 1);
	vector<AQLDate> datevec(swapTerm * paymentsPerYear + 1);	
	vector<double> dfvec(swapTerm * paymentsPerYear + 1);

	if (gridvec.size() < 2)
	{
		throw AQLCoreInvalidData("grid size is less than 2!",__FILE__,__LINE__);
	}

	AQLPriceDataConvention conv(ACT_365_ISDA,CONT);
	AQLString fol(MOD_FOL);

	AQLDate date_nonadjust;
	if (pCalendar)
		date_nonadjust = AQLMathDateUtilities::getDate(fixingDate, AQLString("0D"), fol, *pCalendar);
	else
		date_nonadjust = AQLMathDateUtilities::getDate(fixingDate, AQLString("0D"), fol, AQLString("TKB"));

    AQLDate date1 = date_nonadjust;

	gridvec[0] = conv.getTerm(asof, date_nonadjust);
	datevec[0] = date_nonadjust;

    for (unsigned int i = 1; i < gridvec.size();i++)
	{
        AQLDate oldDate = date1;
        AQLString term = AQLString(int(i * 6)) + AQLString("M");

		if (pCalendar)
			date_nonadjust = AQLMathDateUtilities::getDate(fixingDate, term, AQLString("NO_CHANGE"), *pCalendar);
		else
			date_nonadjust = AQLMathDateUtilities::getDate(fixingDate, term, AQLString("NO_CHANGE"), AQLString("TKB"));

        AQLDate tmpDate = date_nonadjust;

		if (pCalendar)
			date1 = AQLMathDateUtilities::getDate(tmpDate, AQLString("0D"), fol, *pCalendar);
		else
			date1 = AQLMathDateUtilities::getDate(tmpDate, AQLString("0D"), fol, AQLString("TKB"));
        
        datevec[i] = date1;
		AQLPriceDataConvention conv1(ACT_365_ISDA,CONT);
        gridvec[i] = conv1.getTerm(asof, date1);
		AQLPriceDataConvention conv2(DayCount(indexdaycount),CONT);
		//E30_360
        termvec[i - 1] = conv2.getTerm(oldDate, date1);
	}

	//AQLMathYieldCurve
	AQLMathYieldCurve& yc = AQLMathCurveFuncUtility::getYieldCurveForCurveID(dataInstance,curveID);
	yc.setInterpolation(FN_SPLINEINTERPOLATION_STR);
	yc.getDayCount().setDayCount(AQLCoreComponentManager::getDayCount(AC_360));
	yc.getSlidingRule().convertFromString(MOD_FOL);

	if (pCalendar)
		AQLMathCurveFuncUtility::setCalendarForCurveID(yc, *pCalendar);
	else
		AQLMathCurveFuncUtility::setCalendarForCurveID(yc, AQLString("TKB"));

	yc.getFrequency().convertFromString(AQLString("SEMI_ANNUAL"));

	if (pCurveType)
		yc.setCurveType(*pCurveType);
	else
		yc.setCurveType(AQLString("STD"));
	
	// get DF
    for (unsigned int i = 0; i < gridvec.size();i++)
	{
		dfvec[i] = yc.getDF(asof, datevec[i]);
	}

	//option Term
	double optionTerm = conv.getTerm(asof, fixingDate);

	//calc index
    double first = 0;
    double second = 0;
	double T0 = gridvec[0];
	for (unsigned i = 1; i < gridvec.size();i++)
	{
		double Ti = gridvec[i];
		first = first + fwd * termvec[i - 1] * dfvec[i - 1] * (Ti - T0) / dfvec[0];
	    second = second + fwd * termvec[i - 1] * dfvec[i] * (Ti - T0) * (Ti - T0 + 1.) / dfvec[0];

	}
	double Tn = gridvec.back();
    first = first + dfvec[dfvec.size() - 2] * (Tn - T0) / dfvec[0];
    second = second + dfvec.back() * (Tn - T0) * (Tn - T0 + 1.) / dfvec[0];

	if (0.0==first)
		throw AQLCoreInvalidData("Convexity Error",__FILE__,__LINE__);

    return fwd + 0.5 * fwd * fwd * fwdVol * fwdVol * optionTerm * second / first;
}
// uchida

//
/*!
	@brief CMS Convexity Adjustment Function by SABR Model

	@param[in] S0			CMS rate
	@param[in] T			the start date of S0
	@param[in] tau			the daycount fraction of S0
	@param[in] delta		the daycount fraction of trade
	@param[in] num_swap		the number of cash flow of CMS
	@param[in] SABR_params	SABR_params = {alpha,beta,nu,rho}
    @param[in] threshold	threshold for SABR volatiltiy
	@param[in] int_cond		integral conditons: (lower bound ofintegral range, upper bound ofintegral range, integral steps)

    @return Convexity Adjustment of CMS
*/
double 
AQLMathIRVanillaFuncUtility::
getSABRConvexityAdjust(	double S0,
						double T,
						double tau,
						double delta,
						double num_swap,
						const DoubleArray& sabr_params,
						double threshold,
						const DoubleArray& int_cond
						)
{
	if(sabr_params.size() != 4) throw AQLCoreInvalidData("SABR params.size() != 4",__FILE__,__LINE__);
	if(sabr_params[0] < 0) throw AQLCoreInvalidData("alpha must be positive",__FILE__,__LINE__);
    if(sabr_params[1] < 0 || 1 < sabr_params[1]) throw AQLCoreInvalidData("beta must be in (0,1)",__FILE__,__LINE__);
    if(sabr_params[2] < 0 ) throw AQLCoreInvalidData("nu must be possitive.",__FILE__,__LINE__);
    if(fabs(sabr_params[3]) > 1 ) throw AQLCoreInvalidData("rho must be in (-1,1).",__FILE__,__LINE__);

	AQLSABRCAIntegral sub_func(S0, T, sabr_params, threshold);

	double theta = 1. - tau * S0 / (1. + tau * S0) * ( delta / tau + num_swap / (pow(1. + tau * S0,num_swap) - 1.) );

	/*vector<double> int_cond = XllPlusTips::getDoubleVector(int_cond_);*/
	double int_lb = int_cond[0]; double int_ub = int_cond[1]; double int_step = int_cond[2];

	const double integralStep = 1.0 / int_step;
	const double integralStart = int_lb;
	sub_func.setIntegralParams(integralStart, integralStep, 10);

	double int_BS = 0.;
	for(size_t i = int_lb*int_step; i < int_ub*int_step-1; ++i)
	{
		int_BS += sub_func.integralVol(i);
	}

	double CA = S0 * theta * (2. / S0 / S0 * int_BS - 1.);
	
	return CA;
}

//////////////////////////AlgoQuantLib-Userfunc

double rtsafe( calcBase& cb, const double x1, const double x2, const double xacc)
{
	const int MAXIT=100;
	int j;
	double df,dx,dxold,f,fh,fl,temp,xh,xl,rts;

	cb.calc(x1, fl, df);
	cb.calc(x2, fh, df);
	if ((fl > 0.0 && fh > 0.0) || (fl < 0.0 && fh < 0.0))
		throw AQLCoreNumericalError("Not Convergence from rtsafe", __FILE__,__LINE__);
	if (fl == 0.0) return x1;
	if (fh == 0.0) return x2;
	if (fl < 0.0) {
		xl=x1;
		xh=x2;
	} else {
		xh=x1;
		xl=x2;
	}
	rts=0.5*(x1+x2);
	dxold=fabs(x2-x1);
	dx=dxold;
	cb.calc(rts,f,df);
	for (j=0;j<MAXIT;j++) {
		if ((((rts-xh)*df-f)*((rts-xl)*df-f) > 0.0)
			|| (fabs(2.0*f) > fabs(dxold*df))) {
			dxold=dx;
			dx=0.5*(xh-xl);
			rts=xl+dx;
			if (xl == rts) return rts;
		} else {
			dxold=dx;
			dx=f/df;
			temp=rts;
			rts -= dx;
			if (temp == rts) return rts;
		}
		if (fabs(dx) < xacc) return rts;
		cb.calc(rts,f,df);
		if (f < 0.0)
			xl=rts;
		else
			xh=rts;
	}
	throw AQLCoreNumericalError("Maximum number of iterations exceeded in rtsafe",__FILE__,__LINE__);
	return 0.0;
}



// These are the testing functions for rtsafe.
// If you do not need, remove.
void cubic(const double x, double& pre, double& deri)
{	
	double a1 = 0.0;
	double a2 = -1.0;
	double a3 = 0.0;

	pre  = x*x*x + a1*x*x + a2*x + a3;
	deri = 3.0*x*x + 2.0*a1*x + a2;
	if(AQLMath::abs(deri)<0.000000001)
		deri = 0.000000001;
}

// ----------------------------------------------
// This function is for the rtsafe().
// When you use rtsafe(), define new class inheritaged calcBase class.
// And implement actual calc function which is defined as virtual function.
// Please note that you HAVE TO implement the check for deri == 0
// to prevent 0 divide.
void
impliedVolFinder::calc( const double x, double& pre, double& deri)
{
	_param.Vol = x;
	pre  = _bs->calc(_param) - _prem;
	//if(abs(pre) < 1.e-7)
	//	pre = 0.0;
	deri = _vega->calc(_param)  ;
	if(AQLMath::abs(deri) < 0.0000000001)
		deri = 0.0000000001;
}

void
impliedStraddleVolFinder::calc( const double x, double& pre, double& deri)
{
	_param.Vol = x;

#ifdef _DEBUG
    double call_premium = _bscall->calc(_param);
    double put_premium = _bsput->calc(_param);
#endif
	pre  = _bscall->calc(_param) + _bsput->calc(_param) - _prem;
	deri = _vegacall->calc(_param) + _vegaput->calc(_param);
	if(AQLMath::abs(deri) < 0.0000000001)
	deri = 0.0000000001;
}

void
fwdDeltaFinder::calc( const double x, double& pre, double& deri)
{
	_param.K = x;
	pre  = _delta->calc(_param) - _target;
	deri = _gamma->calc(_param)  ;
	if(AQLMath::abs(deri) < 0.0000000001)
		deri = 0.0000000001;
}


double
AQLMathIRVanillaFuncUtility::optimize( double x1, double x2, double x3, AnalyticParam& param, AQLBlackScholesBase*& p1, AQLBlackScholesBase*& p2)
{
	impliedVolFinder im(x3, param, p1, p2);
    calcBase& cb = dynamic_cast<calcBase&>(im);
	
    const double eps = 1.0e-9;
	return rtsafe( cb, x1, x2, eps);
}


double
AQLMathIRVanillaFuncUtility::optimize( double x1, double x2, double x3, AnalyticParam& param, AQLBlackScholesBase*& p1, AQLBlackScholesBase*& p2, AQLBlackScholesBase*& p3, AQLBlackScholesBase*& p4)
{
	impliedStraddleVolFinder im(x3, param, p1, p2, p3, p4);
    calcBase& cb = dynamic_cast<calcBase&>(im);
	
    const double eps = 1.0e-9;
	return rtsafe( cb, x1, x2, eps);
}

//optimize for fwddelta
double
AQLMathIRVanillaFuncUtility::optimize2( double x1, double x2, double x3, AnalyticParam& param, AQLBlackScholesBase*& p1, AQLBlackScholesBase*& p2)
{
	fwdDeltaFinder fd(x3, param, p1, p2);
    calcBase& cb = dynamic_cast<calcBase&>(fd);
	
    const double eps = 1.0e-9;
	return rtsafe( cb, x1, x2, eps);
}

int
AQLMathIRVanillaFuncUtility::businessDaysBetween(const AQLDate& d1, const AQLDate& d2, const AQLString& calendar)
{
    const int inc = d1 < d2 ? 1 : -1;
    int ret = 0;
    AQLPriceDataCalendar cal;
    cal.convertFromString(calendar);
    for(AQLDate temp = d1; temp != d2; temp.addDays(inc)){
        if(cal.getBusinessDay(temp, 0)==temp) ret += inc;
    }
    return ret;
}


//const unsigned int AQLSABRCAIntegral::MAX_INTEGRALNUM = 20;
AQLSABRCAIntegral::AQLSABRCAIntegral(double S0_,double T_,const vector<double>& sabr_params, double threshold_)
:mS0(S0_),mT(T_),mT_sqr(AQLMath::sqrt(mT)),mAlpha(sabr_params[0]), mBeta(sabr_params[1]), mNu(sabr_params[2]), mRho(sabr_params[3]), mBeta_m(1.0 - mBeta),
mIntegralStart(0.0), mIntegralStep(0.0), mIntegralNum(0), mThreshold(threshold_), sabr(new AQLMathSABR_Hagan(sabr_params[0],sabr_params[1],sabr_params[2],sabr_params[3]))
{
	mTmp1 = mRho * mBeta * mNu *  AQLMath::pow(mAlpha, 2) * mT * 0.25;
	mTmp2 = AQLMath::pow(mBeta_m, 2) * AQLMath::pow(mAlpha, 3) * mT * INV_24;
	mTmp3 = mAlpha * mNu * mNu * ( 2.0 - 3.0 * mRho * mRho ) * mT * INV_24;
	for (unsigned int i = 0; i < MAX_INTEGRALNUM; ++i)
	{
		mAbscissas[i] = 0.0;
		mWeights[i] = 0.0;
	}
}

AQLSABRCAIntegral::~AQLSABRCAIntegral() 
{ 
	delete sabr; 
	sabr = 0; 
}

void 
AQLSABRCAIntegral::setIntegralParams(const double start, const double step, size_t num)
{
	if (num > MAX_INTEGRALNUM)
	{
		throw AQLCoreInvalidData("wrong integralnumber", __FILE__, __LINE__);
	}
	mIntegralStart = start;
	mIntegralStep = step;
	mIntegralNum = num;
	
	DoubleArray tmpAbs(mIntegralNum);
	DoubleArray tmpWeights(mIntegralNum);

	AQLGaussLegendre GQ(static_cast<int>(mIntegralNum));

	GQ.get(tmpAbs, tmpWeights, start, start + step);
	for (unsigned int i = 0; i < mIntegralNum; ++i)
	{
		mAbscissas[i] = tmpAbs[i];
		mWeights[i] = tmpWeights[i];
	}

}

double
AQLSABRCAIntegral::integralVol(unsigned int num) const
{
	const double addAbs = num * mIntegralStep;

	double ret = 0.0;
	for (unsigned int i = 0; i < mIntegralNum; ++i)
	{
		const double x = mAbscissas[i] + addAbs;
		const double div_S0_x = mS0 / x;
		const double tmpLog = log(div_S0_x);
		const double tmpA = pow(mS0 * x, mBeta_m * 0.5);
		const double tmpB = pow(tmpLog * (mBeta_m), 2.0);

		double tmp1 = mAlpha  + mTmp1 / tmpA + mTmp2 / (tmpA * tmpA) + mTmp3;

		double tmp2 = (1.0 + tmpB * INV_24 + tmpB * tmpB * INV_1920) * tmpA;

		double v = 0.0;
		if (fabs(mS0 - x) > eps_SABR)
		{
			const double z = mNu / mAlpha * tmpA * tmpLog;
			double chi_z = 0.0;
			const double tmp1_ = sqrt(1.0 - 2.0 * mRho * z + z * z);
			if(fabs(z)<eps_SABR)
			{
				chi_z = z;
			}  
			else if(tmp1_ + z - mRho > 0.0)
			{
				const double tmp2_ = (tmp1_ + z - mRho) / (1.0 - mRho);
				chi_z = log(tmp2_);
			}
			else
			{
				const double tmp2_ = tmp1_ - (z - mRho);
				chi_z = log( (1.0 + mRho) / tmp2_ );
			}
			const double tmp3 = tmp1 * z;
			const double tmp4 = tmp2 * chi_z;
			v = tmp3 / tmp4;
		}
		else
		{
			v = tmp1 / (tmpA * tmpA);
		}

		if (v > mThreshold)
		{
			v = mThreshold;
		}

		tmp2 = v * mT_sqr;
		const double tmp3 = tmpLog / tmp2;
		const double tmp4 = 0.5 * tmp2;
		const double val1 = tmp3 + tmp4;
		const double val2 = tmp3 - tmp4;


		double val1_norm = 0.0;
		double val2_norm = 0.0;
		
		double y, z;
		y = fabs(val1);
		

		if (y <= 0.46875 * MM_SQRT2) 
		{
			// evaluate erf() for |x| <= sqrt(2)*0.46875
			z = y * y;

			y = val1 * ((((a_[0]*z+a_[1])*z+a_[2])*z+a_[3])*z+a_[4])
				  / ((((b_[0]*z+b_[1])*z+b_[2])*z+b_[3])*z+b_[4]);
			val1_norm = 0.5 + y;
		}
		else if (y <= 4.0 * MM_SQRT2)
		{
			// evaluate erfc() for sqrt(2)*0.46875 <= |x| <= sqrt(2)*4.0
			z = exp(-y * y * 0.5) * 0.5;
			y = y / MM_SQRT2;
			y = ((((((((c_[0]*y+c_[1])*y+c_[2])*y+c_[3])*y+c_[4])*y+c_[5])*y+c_[6])*y+c_[7])*y+c_[8])
			  / ((((((((d_[0]*y+d_[1])*y+d_[2])*y+d_[3])*y+d_[4])*y+d_[5])*y+d_[6])*y+d_[7])*y+d_[8]);


			y = z * y;
			val1_norm = val1 < 0.0 ? y : 1-y;
		} 
		else 
		{
			// evaluate erfc() for |x| > sqrt(2)*4.0
			z = exp(-y * y * 0.5) * MM_SQRT2 * 0.5 / y;
	 
			y = 2 / (y*y);
			y = y * (((((p_[0]*y+p_[1])*y+p_[2])*y+p_[3])*y+p_[4])*y+p_[5])
				  / (((((q_[0]*y+q_[1])*y+q_[2])*y+q_[3])*y+q_[4])*y+q_[5]);
			y = z * (MM_1_SQRTPI-y);
			val1_norm = val1 < 0.0 ? y : 1-y;
		}


		y = fabs(val2);
		if (y <= 0.46875 * MM_SQRT2) 
		{
			// evaluate erf() for |x| <= sqrt(2)*0.46875
			z = y * y;

			y = val2 * ((((a_[0]*z+a_[1])*z+a_[2])*z+a_[3])*z+a_[4])
				  / ((((b_[0]*z+b_[1])*z+b_[2])*z+b_[3])*z+b_[4]);
			val2_norm = 0.5 + y;
		}
		else if (y <= 4.0 * MM_SQRT2)
		{
			// evaluate erfc() for sqrt(2)*0.46875 <= |x| <= sqrt(2)*4.0
			z = exp(-y * y * 0.5) * 0.5;
			y = y / MM_SQRT2;

			y = ((((((((c_[0]*y+c_[1])*y+c_[2])*y+c_[3])*y+c_[4])*y+c_[5])*y+c_[6])*y+c_[7])*y+c_[8])
			  / ((((((((d_[0]*y+d_[1])*y+d_[2])*y+d_[3])*y+d_[4])*y+d_[5])*y+d_[6])*y+d_[7])*y+d_[8]);

			y = z * y;
			val2_norm = val2 < 0.0 ? y : 1-y;
		} 
		else 
		{
			// evaluate erfc() for |x| > sqrt(2)*4.0
			z = exp(-y * y * 0.5) * MM_SQRT2 * 0.5 / y;
			y = 2 / (y*y);

			y = y * (((((p_[0]*y+p_[1])*y+p_[2])*y+p_[3])*y+p_[4])*y+p_[5])
				  / (((((q_[0]*y+q_[1])*y+q_[2])*y+q_[3])*y+q_[4])*y+q_[5]);
			y = z * (MM_1_SQRTPI-y);
			val2_norm = val2 < 0.0 ? y : 1-y;
		}

		ret += (mS0 * val1_norm - x * val2_norm) * mWeights[i];		
	}
	
	return ret;
}


double 
AQLSABRCAIntegral::operator()(double x) const
{
	double v = sabr->getSABRVol(mT,mS0,x); v = v > mThreshold ? mThreshold : v;

	AnalyticBKParam param;
	param.F   = mS0;
	param.K	  = x;
	param.Vol = v;
	//param.rd  = 0.;
	param.Te  = mT;
	param.Nu  = 1.;
	param.ErrorCheck();

	return AnalyticFormulae::BKpremCall(param);
}
