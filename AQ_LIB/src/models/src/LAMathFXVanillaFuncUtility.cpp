#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LAFunctionUtilities.h"
#include "LAMathFXVanillaFuncUtility.h"
#include "LAMathIRVanillaFuncUtility.h"
#include "LAMathDateUtilities.h"
#include "LAObject.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LACoreTemplateType.h"
#include "LAMathDefine.h"
#include "LAPriceDataCalendar.h"
#include "LAPriceDataSlidingRule.h"
#include "LABasic.h"
#include "LAAlgorithm.h"
#include "LAPriceDataDayCount.h"
#include "LAPriceCFGenUtility.h"
#include "LAMathDateCalculations.h"
#include "LADataReference.h"
#include "LAAnalyticFormula.h"
#include "LABlackScholesCalc.h"
#include "LADataProcedure.h"
#include "LACoreComponentManager.h"
#include "LAPriceDataConvention.h"
#include "LADataMatrix.h"
#include "LAPriceDataInterpolation.h"
#include "LADataMultiReference.h"
#include "LAMatrix.h"
#include <cmath>
#include <map>

using namespace std;


double 
LAMathFXVanillaFuncUtility::gkOption(LAString& optiontype, LAString& buysell, LAString& callput,
				 double spot, double strike, double vol, 
				 double localrate, double foreignrate, const LADate& basedate, 
				 const LADate& spotdate, const LADate& expirydate, const LADate& deliverydate)
{
	//change nospace & upper
	upper(optiontype);
	upper(callput);
	upper(buysell);
	LAString daycount(AC_365I);
	//hishida vannavolga
	LAString blackdaycount(AC_365I);

	///////////setup///////////////////////////
	AnalyticGKParam param;
	param.S = spot;
	param.K	= strike;
	param.Vol = vol;
	param.rf = foreignrate;
	param.rd = localrate;
	param.Td = LAMathDateUtilities::getTerm(spotdate,deliverydate,daycount,false);
	//hishida vannavolga
	//param.Te = LAMathDateUtilities::getTerm(basedate,expirydate,daycount, true);
	param.Te = LAMathDateUtilities::getTerm(basedate,expirydate,blackdaycount, true);
	param.ErrorCheck();
	////////////////////////////////////////////
	//main sorce
	LAString bscomponent = GK + optiontype  + callput ;
	std::map<LAString, LABlackScholesBase*> &var = LACoreComponentManager::getBlackComponentMap();
	std::map<LAString, LABlackScholesBase*> ::iterator it = var.find(bscomponent);
	if(it==var.end())
		throw LACoreInvalidData("Option type is not supported",__FILE__,__LINE__);
	LABlackScholesBase* p = it->second;
	double ret=0.0;
	if(BUY==buysell)
		ret = p->calc(param);
	else if(SELL==buysell)
		ret = -1* p->calc(param);
	else 
		throw LACoreInvalidData("Choose Buy or Sell", __FILE__,__LINE__);

	LAMathBaseFuncUtility::adjustunit(ret,optiontype);
	return ret;
}
double 
LAMathFXVanillaFuncUtility::gkOptionIV(LAString& buysell,
									 LAString& callput,
							            double spot,
										double strike,
										double prem,
                                        double localrate, 
										double foreignrate,
										const LADate& basedate,
										const LADate& spotdate, 
										const LADate& expirydate, 
										const LADate& deliverydate,
										double high,
                                    	double low)
{
	//Change nospace & upper
	upper(callput);
	upper(buysell);
	LAString daycount(AC_365I);
	//hishida vannavolga
	LAString blackdaycount(AC_365I);

	//Setup
	AnalyticGKParam param;
	param.S   = spot;
	param.K	  = strike;
	param.rd  = localrate;
	param.rf  = foreignrate;
	//hishida vannavolga
	//param.Te  = LAMathDateUtilities::getTerm(basedate,expirydate,   daycount, true);
	param.Te  = LAMathDateUtilities::getTerm(basedate,expirydate,   blackdaycount, true);
	param.Td  = LAMathDateUtilities::getTerm(spotdate,deliverydate, daycount, true);
	param.ErrorCheck();	
	
	if (callput != CALL && callput != PUT) throw LACoreInvalidData("Choose Call or Put!", __FILE__,__LINE__);
	if (buysell != BUY && buysell != SELL) throw LACoreInvalidData("Choose Buy or Sell!", __FILE__,__LINE__);

	//TypeSelect
	LAString bscomponent = LAString(GK) + LAString(PREM)  + LAString(callput) ;
	std::map<LAString, LABlackScholesBase*> &var = LACoreComponentManager::getBlackComponentMap();
	std::map<LAString, LABlackScholesBase*> ::iterator it = var.find(bscomponent);
	LABlackScholesBase* p1 = it->second;

	bscomponent = LAString(GK) + LAString(VEGA)  + LAString(callput) ;
	var = LACoreComponentManager::getBlackComponentMap();
	it = var.find(bscomponent);
	LABlackScholesBase* p2 = it->second;

	//Optimize
	return LAMathIRVanillaFuncUtility::optimize(low, high, prem, param, p1, p2);//Bisection Method}
}
//================================================================================================

double 
LAMathFXVanillaFuncUtility::digitalOption(LAString& optiontype, LAString& buysell, LAString& callput,
				 double spot, double strike, double vol, 
				 double localrate, double foreignrate, const LADate& basedate, 
				 const LADate& spotdate, const LADate& expirydate, const LADate& deliverydate)
{
	//change nospace & upper
	upper(optiontype);
	upper(callput);
	upper(buysell);
	LAString daycount(AC_365I);
	//hishida vannavolga
	LAString blackdaycount(AC_365I);

	/////////////setup////////////////////////////
	AnalyticDGParam param;
	//Dig = 1.0 <==> digital payoff = 1.0; this means digital prem per Yen; 
	param.Dig = 1.0;
	param.S = spot;
	param.K	= strike;
	param.Vol = vol;
	param.rf = foreignrate;
	param.rd = localrate;
	param.Td = LAMathDateUtilities::getTerm(spotdate,deliverydate, daycount, true);
	//hishida vannavolga
	//param.Te = LAMathDateUtilities::getTerm(basedate,expirydate, daycount, true);
	param.Te = LAMathDateUtilities::getTerm(basedate,expirydate, blackdaycount, true);
	param.ErrorCheck();
	////////////////////////////////////////////////
	//main sorce
	LAString bscomponent = DG + optiontype  + callput;
	std::map<LAString, LABlackScholesBase*> &var = LACoreComponentManager::getBlackComponentMap();
	std::map<LAString, LABlackScholesBase*> ::iterator it = var.find(bscomponent);
	if(it==var.end())
		throw LACoreInvalidData("Option type is not supported",__FILE__,__LINE__);
	LABlackScholesBase* p = it->second;
	double ret=0.0;
	if(BUY==buysell)
		ret = p->calc(param);
	else if(SELL==buysell)
		ret = -1* p->calc(param);
	else 
		throw LACoreInvalidData("Choose Buy or Sell", __FILE__,__LINE__); 

	LAMathBaseFuncUtility::adjustunit(ret,optiontype);
	return ret;
}
double 
LAMathFXVanillaFuncUtility::singleBarrierOption(LAString& optiontype, LAString& buysell, LAString& callput,
										LAString& downup, LAString& inout, double rebate, double limit,
										double spot, double strike, double vol, 
										double localrate, double foreignrate, const LADate& basedate, 
										const LADate& spotdate, const LADate& expirydate, const LADate& deliverydate)
{
	//change nospace & upper
	upper(optiontype);
	upper(inout);
	upper(buysell);
	upper(downup);
	upper(callput);
	LAString daycount(AC_365I);
	//hishida vannavolga
	LAString blackdaycount(AC_365I);

	/////////////setup////////////////////////////
	AnalyticSBParam param;
	param.R = rebate;
	param.L = limit;
	param.S = spot;
	param.K	= strike;
	param.Vol = vol;
	param.Td = LAMathDateUtilities::getTerm(spotdate,deliverydate,daycount, true);
	//hishida vannavolga
	//param.Te = LAMathDateUtilities::getTerm(basedate,expirydate,daycount, true);
	param.Te = LAMathDateUtilities::getTerm(basedate,expirydate,blackdaycount, true);
	param.rf = foreignrate;
	param.rd = localrate;
	if(CALL==callput){ param.cp=1;}
	else if(PUT==callput){param.cp=-1;}
	if(SBDOWN==downup){param.du=1;}
	else if(SBUP==downup){param.du=-1;}
	param.ErrorCheck();
	////////////////////////////////////////////////
	//main source
	LAString bscomponent = SB + optiontype  + callput + downup + inout;
	std::map<LAString, LABlackScholesBase*> &var = LACoreComponentManager::getBlackComponentMap();
	std::map<LAString, LABlackScholesBase*> ::iterator it = var.find(bscomponent);
	if(it==var.end())
		throw LACoreInvalidData("Option type is not supported",__FILE__,__LINE__);
	LABlackScholesBase* p = it->second;
	double ret=0.0;
	if(BUY==buysell)
		ret = p->calc(param);
	else if(SELL==buysell)
		ret = -1* p->calc(param);
	else 
		throw LACoreInvalidData("Choose Buy or Sell", __FILE__,__LINE__); 
	
	LAMathBaseFuncUtility::adjustunit(ret,optiontype);
	return ret;
}

double 
LAMathFXVanillaFuncUtility::singleBarrierOptionEasily(LAString& optiontype, LAString& buysell, LAString& callput,
												LAString& downup, LAString& inout, double rebate, double limit,
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
	LAString bscomponent = SB + optiontype  + callput + downup + inout;
	std::map<LAString, LABlackScholesBase*> &var = LACoreComponentManager::getBlackComponentMap();
	std::map<LAString, LABlackScholesBase*> ::iterator it = var.find(bscomponent);
	if(it==var.end())
		throw LACoreInvalidData("Option type is not supported",__FILE__,__LINE__);
	LABlackScholesBase* p = it->second;	
	double ret=0.0;
	if(BUY==buysell)
		ret = p->calc(param);
	else if(SELL==buysell)
		ret = -1* p->calc(param);
	else 
		throw LACoreInvalidData("Choose Buy or Sell", __FILE__,__LINE__); 
	
	LAMathBaseFuncUtility::adjustunit(ret,optiontype);
	return ret;
}


double 
LAMathFXVanillaFuncUtility::doubleBarrierOption(LAString& optiontype, LAString& buysell, LAString& callput, LAString& inout,
										double limitlow, double limithigh, double rebate, LAString& rebatetype,  
										double spot, double strike, double vol, 
										double localrate, double foreignrate, const LADate& basedate, 
										const LADate& spotdate, const LADate& expirydate, const LADate& deliverydate, 
										int num)
{
	//change no space and upper
	upper(optiontype);
	upper(callput);
	upper(inout);
	upper(buysell);
	upper(rebatetype);
	LAString daycount(AC_365I);
	//hishida vannavolga
	LAString blackdaycount(AC_365I);

	/////////////setup////////////////////////////
	AnalyticDBParam param;
	param.S = spot;
	param.K = strike;
	param.Vol = vol;
	param.Td = LAMathDateUtilities::getTerm(spotdate,deliverydate,daycount, true);
	//hishida vannavolga
	//param.Te = LAMathDateUtilities::getTerm(basedate,expirydate,daycount, true);
	param.Te = LAMathDateUtilities::getTerm(basedate,expirydate,blackdaycount, true);
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
		LAString bscomponent = DB + optiontype  + callput + inout + inout;
		std::map<LAString, LABlackScholesBase*> &var = LACoreComponentManager::getBlackComponentMap();
		std::map<LAString, LABlackScholesBase*> ::iterator it = var.find(bscomponent);
		if(it==var.end())
			throw LACoreInvalidData("Option type is not supported",__FILE__,__LINE__);
		LABlackScholesBase* p = it->second;
		if(BUY==buysell)
			ret = p->calc(param);
		else if(SELL==buysell)
			ret = -1* p->calc(param);
		else 
			throw LACoreInvalidData("Choose Buy or Sell", __FILE__,__LINE__); 
	}
	else if(optiontype==REBATE)
	{
		LAString bscomponent = DB + optiontype  + rebatetype;
		std::map<LAString, LABlackScholesBase*> &var = LACoreComponentManager::getBlackComponentMap();
		std::map<LAString, LABlackScholesBase*> ::iterator it = var.find(bscomponent);
		if(it==var.end())
			throw LACoreInvalidData("Option type is not supported",__FILE__,__LINE__);
		LABlackScholesBase* p = it->second;
		if(BUY==buysell)
			ret = p->calc(param);
		else if(SELL==buysell)
			ret = -1* p->calc(param);
		else 
			throw LACoreInvalidData("Choose Buy or Sell", __FILE__,__LINE__); 
	}

	LAMathBaseFuncUtility::adjustunit(ret,optiontype);
	return ret;
}


double 
LAMathFXVanillaFuncUtility::calcstrikefromdelta(double target, LAString& spotfwd, LAString& callput, double spotfx,
									double fwdfx, double atmvol, double reversal, double strangle, double foreignrate,
								  LADate& basedate, LADate& spotdate, LADate& expirydate, LADate& deliverydate)
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
LAMathFXVanillaFuncUtility::calcstrikefromdelta(double target, LAString& spotfwd, LAString& callput, double spotfx,
									double fwdfx, double atmvol, double reversal, double strangle, double foreignrate,
								  LADate& basedate, LADate& spotdate, LADate& expirydate, LADate& deliverydate,
								  double lower, double upper)                                    
{
	upper(spotfwd);
	upper(callput);

	LAString daycount(AC_365I);	
	double deliveryterm = LAMathDateUtilities::getTerm(spotdate,deliverydate,daycount, true);
	double expiryterm = LAMathDateUtilities::getTerm(basedate,expirydate,daycount, true);
	double foreigndf;
	FORMULAE_BEGIN
	foreigndf = LAMath::exp(-1.0 * foreignrate * deliveryterm);
	FORMULAE_END
	double ret = calcstrikefromdelta(target, spotfwd, callput, spotfx,
									fwdfx, atmvol, reversal, strangle, foreigndf, expiryterm,
								  lower, upper) ; 
	return ret;
	
}

double
LAMathFXVanillaFuncUtility::calcstrikefromdelta(double target, LAString& spotfwd, LAString& callput, double spotfx, 
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
LAMathFXVanillaFuncUtility::calcstrikefromdelta(double target, LAString& spotfwd, LAString& callput, double spotfx, 
											  double fwdfx, double atmvol, double reversal, double strangle,
											  double foreigndf, double expiryterm, double lower, double upper)
{
	upper(spotfwd);
	upper(callput);
	if(spotfwd != SPOT && spotfwd != FORWARD)
		throw LACoreInvalidData("Input SPOT or FWD",__FILE__,__LINE__);
	if(callput != CALL && callput != PUT && callput != HIGH && callput != LOW)
		throw LACoreInvalidData("Input HIGH or LOW",__FILE__,__LINE__);

	if(HIGH==callput)
		callput = LAString(CALL);
	else if(LOW==callput)
		callput = LAString(PUT);

	LAString bscomponent1 = LAString(FD) + LAString(DELTA) + callput + spotfwd;
	LAString bscomponent2 = LAString(FD) + LAString(FIRSTDIFF) + callput + spotfwd;
	LABlackScholesBase* p1;
	LABlackScholesBase* p2;
	std::map<LAString, LABlackScholesBase*> &var = LACoreComponentManager::getBlackComponentMap();
	std::map<LAString, LABlackScholesBase*> ::iterator it1;
	std::map<LAString, LABlackScholesBase*> ::iterator it2;

	if(target<0)
		throw LACoreInvalidData("Input positive number as target",__FILE__,__LINE__);

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
		throw LACoreInvalidData("Input ForwardFX as positive",__FILE__,__LINE__);
	
	param.ErrorCheck();

	it1 = var.find(bscomponent1);
	it2 = var.find(bscomponent2);	
	if(it1==var.end()||it2==var.end())
		throw LACoreInvalidData("Choose Call or Put",__FILE__,__LINE__);
	
	p1 = it1->second;
	p2 = it2->second;

	return LAMathIRVanillaFuncUtility::optimize2(lower, upper, target, param, p1, p2);
}

double
LAMathFXVanillaFuncUtility::calcmaxstrike(LAString& spotfwd, const double spotfx, 
											  const double fwdfx, const double atmvol, const double reversal, const double strangle,
											  const double foreigndf, const double expiryterm, double lower, double upper)
{
	upper(spotfwd);
	if(spotfwd != SPOT && spotfwd != FORWARD)
		throw LACoreInvalidData("Input SPOT or FWD",__FILE__,__LINE__);

    if(upper == -10.0)
        upper = spotfx * 2.0;

	LAString bscomponent1 = LAString(FD) + LAString(FIRSTDIFF) + LAString(CALL) + LAString(spotfwd);
	LAString bscomponent2 = LAString(FD) + LAString(SECONDDIFF) + LAString(CALL) + LAString(spotfwd);
	LABlackScholesBase* p1;
	LABlackScholesBase* p2;
	std::map<LAString, LABlackScholesBase*> &var = LACoreComponentManager::getBlackComponentMap();
	std::map<LAString, LABlackScholesBase*> ::iterator it1;
	std::map<LAString, LABlackScholesBase*> ::iterator it2;

	AnalyticGKParam param;
	param.S = spotfx;
	param.F = fwdfx;
	param.Vol = atmvol + reversal * 0.5 + strangle;

	param.Te = expiryterm;
	param.DFf = foreigndf;
	if(param.F!=0.0)
		param.DFd = param.S*param.DFf/param.F;

	else if(param.F==0.0)
		throw LACoreInvalidData("Input ForwardFX as positive",__FILE__,__LINE__);
	
	param.ErrorCheck();

	it1 = var.find(bscomponent1);
	it2 = var.find(bscomponent2);	
	if(it1==var.end()||it2==var.end())
		throw LACoreInvalidData("Choose SPOT or FWD",__FILE__,__LINE__);
	
	p1 = it1->second;
	p2 = it2->second;

	const double ret =  LAMathIRVanillaFuncUtility::optimize2(lower, upper, 0.0, param, p1, p2);
    return ret;
}



double
LAMathFXVanillaFuncUtility::calcmaxfxdelta(LAString& spotfwd, const double spotfx, 
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
		throw LACoreInvalidData("Input ForwardFX as positive",__FILE__,__LINE__);
	
	param.ErrorCheck();
	std::map<LAString, LABlackScholesBase*> &var = LACoreComponentManager::getBlackComponentMap();
	LAString bscomponent = LAString(FD) + LAString(DELTA) + LAString(CALL) + spotfwd;
	std::map<LAString, LABlackScholesBase*> ::iterator it = var.find(bscomponent);	
	LABlackScholesBase* p = it->second;
	double ret= p->calc(param);
    return ret;
}

void
LAMathFXVanillaFuncUtility::setupfxvolobject(LADataInstance* dataInstance, LAString& interpid, LAStringMatrix& baseinfomat, const DoubleArray& termvec, 
										   const DoubleMatrix& strikemat, const DoubleMatrix& volmat, double spotfx)
{
	upper(baseinfomat);
	upper(interpid);
	LAObjectPool& objPool = dataInstance->getObjectPool();
	LAObject* e = NULL;
	if(!objPool.getObject(interpid).isDefined())
	{
		e = new LAObject();
		objPool.set(interpid,e);
	}
	else 
	{
		objPool.getObject(interpid,ENCHKTYPE_ISDEFINED).clear();
		e = &objPool.getObject(interpid).get();
	}

	e->add(INTERPINPUT_TERMVECTOR, new LADataDoubles(termvec));
	e->add(INTERPINPUT_STRIKEMATRIX, new LADataDoubleMatrix(strikemat));
	e->add(INTERPINPUT_VOLMATRIX, new LADataDoubleMatrix(volmat));

	//intertplation mapping
	LAString interpstr = chgrow(baseinfomat, INTERPINPUT_INTERPOLATION, 1);
	upper(interpstr);
	std::map<LAString, LAString>& ivar = LACoreComponentManager::getInterpolationMap();
	std::map<LAString, LAString>::iterator it= ivar.find(interpstr);
	if(it == ivar.end())
	{
		LAString msg = interpstr + "is not registered in interpolation methods";
		throw LACoreInvalidData(msg.getCString(), __FILE__,__LINE__);
	}

	e->add(INTERPINPUT_INTERPOLATION, new LAPriceDataInterpolation()).convertFromString(it->second);

	e->add(INTERPINPUT_SPOTFX, new LADataDouble(spotfx)	);
}

double
LAMathFXVanillaFuncUtility::getvaluefromfxvolobject(LADataInstance* dataInstance, LAStringMatrix& baseinfomat, const double term,
												  const double strike)
{
	upper(baseinfomat);
	LAString interpid = chgrow(baseinfomat, INTERPINPUT_INTERPID, 1);
	LAObjectPool& objPool = dataInstance->getObjectPool();
	LAObject& e = objPool.getObject(interpid, ENCHKTYPE_ISDEFINED).get();
	
	LADataHolder* dh;

	dh = &(e.getData(INTERPINPUT_TERMVECTOR, ISNOTNULL));
	const DoubleVector& termvec = dynamic_cast<const LADataDoubles &>(dh->get()).get();
	
	dh = &(e.getData(INTERPINPUT_STRIKEMATRIX, ISNOTNULL));
	const DoubleMatrix& strikemat = dynamic_cast<const LADataDoubleMatrix &>(dh->get()).get();

	dh = &(e.getData(INTERPINPUT_VOLMATRIX, ISNOTNULL));
	const DoubleMatrix& volmat = dynamic_cast<const LADataDoubleMatrix &>(dh->get()).get();

	dh = &(e.getData(INTERPINPUT_INTERPOLATION, ISNOTNULL));
	LAPriceDataInterpolation& attr = dynamic_cast<LAPriceDataInterpolation &>(dh->get());

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
LAMathFXVanillaFuncUtility::getspotfxfromfxvolobject(LADataInstance* dataInstance, LAStringMatrix& baseinfomat )
{
	upper(baseinfomat);
	LAString interpid = chgrow(baseinfomat, INTERPINPUT_INTERPID, 1);
	LAObjectPool& objPool = dataInstance->getObjectPool();
	LAObject& e = objPool.getObject(interpid, ENCHKTYPE_ISDEFINED).get();
	
	LADataHolder* dh;

	dh = &(e.getData(INTERPINPUT_SPOTFX, ISNOTNULL));
	double spotfx = dynamic_cast<const LADataDouble &>(dh->get()).get();
	
	return spotfx;
}