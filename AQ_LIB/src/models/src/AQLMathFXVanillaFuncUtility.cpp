#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "AQLFunctionUtilities.h"
#include "AQLMathFXVanillaFuncUtility.h"
#include "AQLMathIRVanillaFuncUtility.h"
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
#include "AQLPriceDataDayCount.h"
#include "AQLPriceCFGenUtility.h"
#include "AQLMathDateCalculations.h"
#include "AQLDataReference.h"
#include "AQLAnalyticFormula.h"
#include "AQLBlackScholesCalc.h"
#include "AQLDataProcedure.h"
#include "AQLCoreComponentManager.h"
#include "AQLPriceDataConvention.h"
#include "AQLDataMatrix.h"
#include "AQLPriceDataInterpolation.h"
#include "AQLDataMultiReference.h"
#include "AQLMatrix.h"
#include <cmath>
#include <map>

using namespace std;


double 
AQLMathFXVanillaFuncUtility::gkOption(AQLString& optiontype, AQLString& buysell, AQLString& callput,
				 double spot, double strike, double vol, 
				 double localrate, double foreignrate, const AQLDate& basedate, 
				 const AQLDate& spotdate, const AQLDate& expirydate, const AQLDate& deliverydate)
{
	//change nospace & upper
	upper(optiontype);
	upper(callput);
	upper(buysell);
	AQLString daycount(AC_365I);
	//hishida vannavolga
	AQLString blackdaycount(AC_365I);

	///////////setup///////////////////////////
	AnalyticGKParam param;
	param.S = spot;
	param.K	= strike;
	param.Vol = vol;
	param.rf = foreignrate;
	param.rd = localrate;
	param.Td = AQLMathDateUtilities::getTerm(spotdate,deliverydate,daycount,false);
	//hishida vannavolga
	//param.Te = AQLMathDateUtilities::getTerm(basedate,expirydate,daycount, true);
	param.Te = AQLMathDateUtilities::getTerm(basedate,expirydate,blackdaycount, true);
	param.ErrorCheck();
	////////////////////////////////////////////
	//main sorce
	AQLString bscomponent = GK + optiontype  + callput ;
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

	AQLMathBaseFuncUtility::adjustunit(ret,optiontype);
	return ret;
}
double 
AQLMathFXVanillaFuncUtility::gkOptionIV(AQLString& buysell,
									 AQLString& callput,
							            double spot,
										double strike,
										double prem,
                                        double localrate, 
										double foreignrate,
										const AQLDate& basedate,
										const AQLDate& spotdate, 
										const AQLDate& expirydate, 
										const AQLDate& deliverydate,
										double high,
                                    	double low)
{
	//Change nospace & upper
	upper(callput);
	upper(buysell);
	AQLString daycount(AC_365I);
	//hishida vannavolga
	AQLString blackdaycount(AC_365I);

	//Setup
	AnalyticGKParam param;
	param.S   = spot;
	param.K	  = strike;
	param.rd  = localrate;
	param.rf  = foreignrate;
	//hishida vannavolga
	//param.Te  = AQLMathDateUtilities::getTerm(basedate,expirydate,   daycount, true);
	param.Te  = AQLMathDateUtilities::getTerm(basedate,expirydate,   blackdaycount, true);
	param.Td  = AQLMathDateUtilities::getTerm(spotdate,deliverydate, daycount, true);
	param.ErrorCheck();	
	
	if (callput != CALL && callput != PUT) throw AQLCoreInvalidData("Choose Call or Put!", __FILE__,__LINE__);
	if (buysell != BUY && buysell != SELL) throw AQLCoreInvalidData("Choose Buy or Sell!", __FILE__,__LINE__);

	//TypeSelect
	AQLString bscomponent = AQLString(GK) + AQLString(PREM)  + AQLString(callput) ;
	std::map<AQLString, AQLBlackScholesBase*> &var = AQLCoreComponentManager::getBlackComponentMap();
	std::map<AQLString, AQLBlackScholesBase*> ::iterator it = var.find(bscomponent);
	AQLBlackScholesBase* p1 = it->second;

	bscomponent = AQLString(GK) + AQLString(VEGA)  + AQLString(callput) ;
	var = AQLCoreComponentManager::getBlackComponentMap();
	it = var.find(bscomponent);
	AQLBlackScholesBase* p2 = it->second;

	//Optimize
	return AQLMathIRVanillaFuncUtility::optimize(low, high, prem, param, p1, p2);//Bisection Method}
}
//================================================================================================

double 
AQLMathFXVanillaFuncUtility::digitalOption(AQLString& optiontype, AQLString& buysell, AQLString& callput,
				 double spot, double strike, double vol, 
				 double localrate, double foreignrate, const AQLDate& basedate, 
				 const AQLDate& spotdate, const AQLDate& expirydate, const AQLDate& deliverydate)
{
	//change nospace & upper
	upper(optiontype);
	upper(callput);
	upper(buysell);
	AQLString daycount(AC_365I);
	//hishida vannavolga
	AQLString blackdaycount(AC_365I);

	/////////////setup////////////////////////////
	AnalyticDGParam param;
	//Dig = 1.0 <==> digital payoff = 1.0; this means digital prem per Yen; 
	param.Dig = 1.0;
	param.S = spot;
	param.K	= strike;
	param.Vol = vol;
	param.rf = foreignrate;
	param.rd = localrate;
	param.Td = AQLMathDateUtilities::getTerm(spotdate,deliverydate, daycount, true);
	//hishida vannavolga
	//param.Te = AQLMathDateUtilities::getTerm(basedate,expirydate, daycount, true);
	param.Te = AQLMathDateUtilities::getTerm(basedate,expirydate, blackdaycount, true);
	param.ErrorCheck();
	////////////////////////////////////////////////
	//main sorce
	AQLString bscomponent = DG + optiontype  + callput;
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

	AQLMathBaseFuncUtility::adjustunit(ret,optiontype);
	return ret;
}
double 
AQLMathFXVanillaFuncUtility::singleBarrierOption(AQLString& optiontype, AQLString& buysell, AQLString& callput,
										AQLString& downup, AQLString& inout, double rebate, double limit,
										double spot, double strike, double vol, 
										double localrate, double foreignrate, const AQLDate& basedate, 
										const AQLDate& spotdate, const AQLDate& expirydate, const AQLDate& deliverydate)
{
	//change nospace & upper
	upper(optiontype);
	upper(inout);
	upper(buysell);
	upper(downup);
	upper(callput);
	AQLString daycount(AC_365I);
	//hishida vannavolga
	AQLString blackdaycount(AC_365I);

	/////////////setup////////////////////////////
	AnalyticSBParam param;
	param.R = rebate;
	param.L = limit;
	param.S = spot;
	param.K	= strike;
	param.Vol = vol;
	param.Td = AQLMathDateUtilities::getTerm(spotdate,deliverydate,daycount, true);
	//hishida vannavolga
	//param.Te = AQLMathDateUtilities::getTerm(basedate,expirydate,daycount, true);
	param.Te = AQLMathDateUtilities::getTerm(basedate,expirydate,blackdaycount, true);
	param.rf = foreignrate;
	param.rd = localrate;
	if(CALL==callput){ param.cp=1;}
	else if(PUT==callput){param.cp=-1;}
	if(SBDOWN==downup){param.du=1;}
	else if(SBUP==downup){param.du=-1;}
	param.ErrorCheck();
	////////////////////////////////////////////////
	//main source
	AQLString bscomponent = SB + optiontype  + callput + downup + inout;
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
	
	AQLMathBaseFuncUtility::adjustunit(ret,optiontype);
	return ret;
}

double 
AQLMathFXVanillaFuncUtility::singleBarrierOptionEasily(AQLString& optiontype, AQLString& buysell, AQLString& callput,
												AQLString& downup, AQLString& inout, double rebate, double limit,
												double spot, double strike, double vol, 
												double localrate, double foreignrate,
												double expiryterms, double deliveryterms, int rebatetime)
{
	//change nospace & upper
	upper(downup);
	upper(callput);	
	upper(optiontype);
	upper(inout);
	upper(buysell);
	/////////////setup////////////////////////////
	AnalyticSBParam param;
	param.R = rebate;
	param.L = limit;
	param.S = spot;
	param.K	= strike;
	param.Vol = vol;
	param.Td = deliveryterms;
	param.Te = expiryterms;
	if(CALL==callput){ param.cp=1;}
	else if(PUT==callput){param.cp=-1;}
	if(SBDOWN==downup){param.du=1;}
	else if(SBUP==downup){param.du=-1;}
	param.rf = foreignrate;
	param.rd = localrate;
	param.ReTime = rebatetime;
	param.ErrorCheck();
	////////////////////////////////////////////////
	//main source
	AQLString bscomponent = SB + optiontype  + callput + downup + inout;
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
	
	AQLMathBaseFuncUtility::adjustunit(ret,optiontype);
	return ret;
}


double 
AQLMathFXVanillaFuncUtility::doubleBarrierOption(AQLString& optiontype, AQLString& buysell, AQLString& callput, AQLString& inout,
										double limitlow, double limithigh, double rebate, AQLString& rebatetype,  
										double spot, double strike, double vol, 
										double localrate, double foreignrate, const AQLDate& basedate, 
										const AQLDate& spotdate, const AQLDate& expirydate, const AQLDate& deliverydate, 
										int num)
{
	//change no space and upper
	upper(optiontype);
	upper(callput);
	upper(inout);
	upper(buysell);
	upper(rebatetype);
	AQLString daycount(AC_365I);
	//hishida vannavolga
	AQLString blackdaycount(AC_365I);

	/////////////setup////////////////////////////
	AnalyticDBParam param;
	param.S = spot;
	param.K = strike;
	param.Vol = vol;
	param.Td = AQLMathDateUtilities::getTerm(spotdate,deliverydate,daycount, true);
	//hishida vannavolga
	//param.Te = AQLMathDateUtilities::getTerm(basedate,expirydate,daycount, true);
	param.Te = AQLMathDateUtilities::getTerm(basedate,expirydate,blackdaycount, true);
	param.Ll = limitlow;
	param.Lh = limithigh;
	param.Num = num;
	param.rf = foreignrate;
	param.rd = localrate;
	if(RHIGH==rebatetype){param.Rh=rebate;}
	else if(RLOW== rebatetype){param.Rl=rebate;}
	else if(RIN==rebatetype){param.Ri=rebate;}
	param.ErrorCheck();
	////////////////////////////////////////////////
	//main sorce
	double ret=0.0;
	
	if(optiontype!=REBATE)
	{	
		//This means we can only calculate version of inin or outout
		AQLString bscomponent = DB + optiontype  + callput + inout + inout;
		std::map<AQLString, AQLBlackScholesBase*> &var = AQLCoreComponentManager::getBlackComponentMap();
		std::map<AQLString, AQLBlackScholesBase*> ::iterator it = var.find(bscomponent);
		if(it==var.end())
			throw AQLCoreInvalidData("Option type is not supported",__FILE__,__LINE__);
		AQLBlackScholesBase* p = it->second;
		if(BUY==buysell)
			ret = p->calc(param);
		else if(SELL==buysell)
			ret = -1* p->calc(param);
		else 
			throw AQLCoreInvalidData("Choose Buy or Sell", __FILE__,__LINE__); 
	}
	else if(optiontype==REBATE)
	{
		AQLString bscomponent = DB + optiontype  + rebatetype;
		std::map<AQLString, AQLBlackScholesBase*> &var = AQLCoreComponentManager::getBlackComponentMap();
		std::map<AQLString, AQLBlackScholesBase*> ::iterator it = var.find(bscomponent);
		if(it==var.end())
			throw AQLCoreInvalidData("Option type is not supported",__FILE__,__LINE__);
		AQLBlackScholesBase* p = it->second;
		if(BUY==buysell)
			ret = p->calc(param);
		else if(SELL==buysell)
			ret = -1* p->calc(param);
		else 
			throw AQLCoreInvalidData("Choose Buy or Sell", __FILE__,__LINE__); 
	}

	AQLMathBaseFuncUtility::adjustunit(ret,optiontype);
	return ret;
}


double 
AQLMathFXVanillaFuncUtility::calcstrikefromdelta(double target, AQLString& spotfwd, AQLString& callput, double spotfx,
									double fwdfx, double atmvol, double reversal, double strangle, double foreignrate,
								  AQLDate& basedate, AQLDate& spotdate, AQLDate& expirydate, AQLDate& deliverydate)
{
	upper(spotfwd);
	upper(callput);
	double lower;
	double upper;
	
	//make upper and lower limit.
	//CAUTION:you have to change the number if there is no convergence with these limits. 
	if(CALL==callput || HIGH==callput)
	{
        lower = fwdfx;
		upper = fwdfx + 10000;
	}
	else if(PUT==callput||LOW==callput)
	{
		lower = 0.01;
		upper = fwdfx;
	}

	double ret = calcstrikefromdelta(target, spotfwd, callput, spotfx,
									 fwdfx, atmvol, reversal, strangle, foreignrate,
								   basedate,  spotdate,  expirydate,  deliverydate,
								   lower,  upper);
	return ret;
}

double 
AQLMathFXVanillaFuncUtility::calcstrikefromdelta(double target, AQLString& spotfwd, AQLString& callput, double spotfx,
									double fwdfx, double atmvol, double reversal, double strangle, double foreignrate,
								  AQLDate& basedate, AQLDate& spotdate, AQLDate& expirydate, AQLDate& deliverydate,
								  double lower, double upper)                                    
{
	upper(spotfwd);
	upper(callput);

	AQLString daycount(AC_365I);	
	double deliveryterm = AQLMathDateUtilities::getTerm(spotdate,deliverydate,daycount, true);
	double expiryterm = AQLMathDateUtilities::getTerm(basedate,expirydate,daycount, true);
	double foreigndf;
	FORMULAE_BEGIN
	foreigndf = AQLMath::exp(-1.0 * foreignrate * deliveryterm);
	FORMULAE_END
	double ret = calcstrikefromdelta(target, spotfwd, callput, spotfx,
									fwdfx, atmvol, reversal, strangle, foreigndf, expiryterm,
								  lower, upper) ; 
	return ret;
	
}

double
AQLMathFXVanillaFuncUtility::calcstrikefromdelta(double target, AQLString& spotfwd, AQLString& callput, double spotfx, 
											  double fwdfx, double atmvol, double reversal, double strangle,
											  double foreigndf, double expiryterm)
{
	upper(spotfwd);
	upper(callput);
	double lower;
	double upper;
	//make upper and lower limit.
	//CAUTION:you have to change the number if there is no convergence with these limits. 
	if(CALL==callput||HIGH==callput)
	{
        lower = fwdfx;
		//upper = fwdfx + 10000;
		upper = fwdfx * 100;
	}
	else if(PUT==callput||LOW==callput)
	{
		//lower = 0.01;
		lower = fwdfx * 0.0001;
		upper = fwdfx;
	}
	double ret = calcstrikefromdelta(target, spotfwd, callput,spotfx, fwdfx,  atmvol, reversal, strangle,
											  foreigndf, expiryterm, lower, upper);
	return ret;
}

double
AQLMathFXVanillaFuncUtility::calcstrikefromdelta(double target, AQLString& spotfwd, AQLString& callput, double spotfx, 
											  double fwdfx, double atmvol, double reversal, double strangle,
											  double foreigndf, double expiryterm, double lower, double upper)
{
	upper(spotfwd);
	upper(callput);
	if(spotfwd != SPOT && spotfwd != FORWARD)
		throw AQLCoreInvalidData("Input SPOT or FWD",__FILE__,__LINE__);
	if(callput != CALL && callput != PUT && callput != HIGH && callput != LOW)
		throw AQLCoreInvalidData("Input HIGH or LOW",__FILE__,__LINE__);

	if(HIGH==callput)
		callput = AQLString(CALL);
	else if(LOW==callput)
		callput = AQLString(PUT);

	AQLString bscomponent1 = AQLString(FD) + AQLString(DELTA) + callput + spotfwd;
	AQLString bscomponent2 = AQLString(FD) + AQLString(FIRSTDIFF) + callput + spotfwd;
	AQLBlackScholesBase* p1;
	AQLBlackScholesBase* p2;
	std::map<AQLString, AQLBlackScholesBase*> &var = AQLCoreComponentManager::getBlackComponentMap();
	std::map<AQLString, AQLBlackScholesBase*> ::iterator it1;
	std::map<AQLString, AQLBlackScholesBase*> ::iterator it2;

	if(target<0)
		throw AQLCoreInvalidData("Input positive number as target",__FILE__,__LINE__);

	AnalyticGKParam param;
	param.S = spotfx;
	param.F = fwdfx;
	if(CALL==callput)
		param.Vol = atmvol + reversal * 0.5 + strangle;
	else if (PUT==callput)
	{
		param.Vol = atmvol - reversal * 0.5 + strangle;
		target = -target;
	}
	param.Te = expiryterm;
	param.DFf = foreigndf;
	if(param.F!=0.0)
		param.DFd = param.S*param.DFf/param.F;

	else if(param.F==0.0)
		throw AQLCoreInvalidData("Input ForwardFX as positive",__FILE__,__LINE__);
	
	param.ErrorCheck();

	it1 = var.find(bscomponent1);
	it2 = var.find(bscomponent2);	
	if(it1==var.end()||it2==var.end())
		throw AQLCoreInvalidData("Choose Call or Put",__FILE__,__LINE__);
	
	p1 = it1->second;
	p2 = it2->second;

	return AQLMathIRVanillaFuncUtility::optimize2(lower, upper, target, param, p1, p2);
}

double
AQLMathFXVanillaFuncUtility::calcmaxstrike(AQLString& spotfwd, const double spotfx, 
											  const double fwdfx, const double atmvol, const double reversal, const double strangle,
											  const double foreigndf, const double expiryterm, double lower, double upper)
{
	upper(spotfwd);
	if(spotfwd != SPOT && spotfwd != FORWARD)
		throw AQLCoreInvalidData("Input SPOT or FWD",__FILE__,__LINE__);

    if(upper == -10.0)
        upper = spotfx * 2.0;

	AQLString bscomponent1 = AQLString(FD) + AQLString(FIRSTDIFF) + AQLString(CALL) + AQLString(spotfwd);
	AQLString bscomponent2 = AQLString(FD) + AQLString(SECONDDIFF) + AQLString(CALL) + AQLString(spotfwd);
	AQLBlackScholesBase* p1;
	AQLBlackScholesBase* p2;
	std::map<AQLString, AQLBlackScholesBase*> &var = AQLCoreComponentManager::getBlackComponentMap();
	std::map<AQLString, AQLBlackScholesBase*> ::iterator it1;
	std::map<AQLString, AQLBlackScholesBase*> ::iterator it2;

	AnalyticGKParam param;
	param.S = spotfx;
	param.F = fwdfx;
	param.Vol = atmvol + reversal * 0.5 + strangle;

	param.Te = expiryterm;
	param.DFf = foreigndf;
	if(param.F!=0.0)
		param.DFd = param.S*param.DFf/param.F;

	else if(param.F==0.0)
		throw AQLCoreInvalidData("Input ForwardFX as positive",__FILE__,__LINE__);
	
	param.ErrorCheck();

	it1 = var.find(bscomponent1);
	it2 = var.find(bscomponent2);	
	if(it1==var.end()||it2==var.end())
		throw AQLCoreInvalidData("Choose SPOT or FWD",__FILE__,__LINE__);
	
	p1 = it1->second;
	p2 = it2->second;

	const double ret =  AQLMathIRVanillaFuncUtility::optimize2(lower, upper, 0.0, param, p1, p2);
    return ret;
}



double
AQLMathFXVanillaFuncUtility::calcmaxfxdelta(AQLString& spotfwd, const double spotfx, 
											  const double fwdfx, const double atmvol, const double reversal, const double strangle,
											  const double foreigndf, const double expiryterm, double lower, double upper)
{
	upper(spotfwd);
    const double maxstrike =  calcmaxstrike(spotfwd, spotfx, 
											  fwdfx, atmvol, reversal, strangle,
											  foreigndf, expiryterm, lower, upper);

	//FXDelta
	AnalyticGKParam param;

    param.K = maxstrike;
	
    param.S = spotfx;
	param.F = fwdfx;
	param.Vol = atmvol + reversal * 0.5 + strangle;
	param.Te = expiryterm;
	param.DFf = foreigndf;
	if(param.F!=0.0)
		param.DFd = param.S*param.DFf/param.F;

	else if(param.F==0.0)
		throw AQLCoreInvalidData("Input ForwardFX as positive",__FILE__,__LINE__);
	
	param.ErrorCheck();
	std::map<AQLString, AQLBlackScholesBase*> &var = AQLCoreComponentManager::getBlackComponentMap();
	AQLString bscomponent = AQLString(FD) + AQLString(DELTA) + AQLString(CALL) + spotfwd;
	std::map<AQLString, AQLBlackScholesBase*> ::iterator it = var.find(bscomponent);	
	AQLBlackScholesBase* p = it->second;
	double ret= p->calc(param);
    return ret;
}

void
AQLMathFXVanillaFuncUtility::setupfxvolobject(AQLDataInstance* dataInstance, AQLString& interpid, AQLStringMatrix& baseinfomat, const DoubleArray& termvec, 
										   const DoubleMatrix& strikemat, const DoubleMatrix& volmat, double spotfx)
{
	upper(baseinfomat);
	upper(interpid);
	AQLObjectPool& objPool = dataInstance->getObjectPool();
	AQLObject* e = NULL;
	if(!objPool.getObject(interpid).isDefined())
	{
		e = new AQLObject();
		objPool.set(interpid,e);
	}
	else 
	{
		objPool.getObject(interpid,ENCHKTYPE_ISDEFINED).clear();
		e = &objPool.getObject(interpid).get();
	}

	e->add(INTERPINPUT_TERMVECTOR, new AQLDataDoubles(termvec));
	e->add(INTERPINPUT_STRIKEMATRIX, new AQLDataDoubleMatrix(strikemat));
	e->add(INTERPINPUT_VOLMATRIX, new AQLDataDoubleMatrix(volmat));

	//intertplation mapping
	AQLString interpstr = chgrow(baseinfomat, INTERPINPUT_INTERPOLATION, 1);
	upper(interpstr);
	std::map<AQLString, AQLString>& ivar = AQLCoreComponentManager::getInterpolationMap();
	std::map<AQLString, AQLString>::iterator it= ivar.find(interpstr);
	if(it == ivar.end())
	{
		AQLString msg = interpstr + "is not registered in interpolation methods";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__,__LINE__);
	}

	e->add(INTERPINPUT_INTERPOLATION, new AQLPriceDataInterpolation()).convertFromString(it->second);

	e->add(INTERPINPUT_SPOTFX, new AQLDataDouble(spotfx)	);
}

double
AQLMathFXVanillaFuncUtility::getvaluefromfxvolobject(AQLDataInstance* dataInstance, AQLStringMatrix& baseinfomat, const double term,
												  const double strike)
{
	upper(baseinfomat);
	AQLString interpid = chgrow(baseinfomat, INTERPINPUT_INTERPID, 1);
	AQLObjectPool& objPool = dataInstance->getObjectPool();
	AQLObject& e = objPool.getObject(interpid, ENCHKTYPE_ISDEFINED).get();
	
	AQLDataHolder* dh;

	dh = &(e.getData(INTERPINPUT_TERMVECTOR, ISNOTNULL));
	const DoubleVector& termvec = dynamic_cast<const AQLDataDoubles &>(dh->get()).get();
	
	dh = &(e.getData(INTERPINPUT_STRIKEMATRIX, ISNOTNULL));
	const DoubleMatrix& strikemat = dynamic_cast<const AQLDataDoubleMatrix &>(dh->get()).get();

	dh = &(e.getData(INTERPINPUT_VOLMATRIX, ISNOTNULL));
	const DoubleMatrix& volmat = dynamic_cast<const AQLDataDoubleMatrix &>(dh->get()).get();

	dh = &(e.getData(INTERPINPUT_INTERPOLATION, ISNOTNULL));
	AQLPriceDataInterpolation& attr = dynamic_cast<AQLPriceDataInterpolation &>(dh->get());

	unsigned int N = termvec.size();
	DoubleVector mainvec(N,0.0);
	for(unsigned int i=0;i<N;i++)
	{
		attr.set(strikemat[i],volmat[i]);
		mainvec[i] = attr.value(strike);
	}

	attr.set(termvec, mainvec);
	return attr.value(term);
}

double
AQLMathFXVanillaFuncUtility::getspotfxfromfxvolobject(AQLDataInstance* dataInstance, AQLStringMatrix& baseinfomat )
{
	upper(baseinfomat);
	AQLString interpid = chgrow(baseinfomat, INTERPINPUT_INTERPID, 1);
	AQLObjectPool& objPool = dataInstance->getObjectPool();
	AQLObject& e = objPool.getObject(interpid, ENCHKTYPE_ISDEFINED).get();
	
	AQLDataHolder* dh;

	dh = &(e.getData(INTERPINPUT_SPOTFX, ISNOTNULL));
	double spotfx = dynamic_cast<const AQLDataDouble &>(dh->get()).get();
	
	return spotfx;
}