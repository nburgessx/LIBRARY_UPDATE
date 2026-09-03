/*! @file
    @brief LALinearRatesModel manager class
*/
//  2011, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LALinearRatesModel.cpp
//
//  DESCRIPTION :       LALinearRatesModel 
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLDataBasics.h"
#include "AQLObject.h"
#include "LALinearRatesModel.h"
#include "LALinearRatesOptionValueDataProvider.h"
#include "AQLDataVector.h"
#include "AQLDataMultiReference.h"
#include "AQLMathDefine.h"
#include "LALinearRatesOptionValue.h"
#include "AQLDataReference.h"
#include "LAMathVolFuncFXVannaVolga.h"
#include "AQLCoreComponentManager.h"
#include "LAMathFXVanillaFuncUtility.h"

using namespace std;

//================ LALinearRatesModel ===================================
// constructor
/*!*/
LALinearRatesModel::LALinearRatesModel(void)
{}
// destructor
/*!*/
LALinearRatesModel::~LALinearRatesModel(void)
{}

//================ LAPriceBSValueModel ===================================
// constructor
/*!*/
LAPriceBSValueModel::LAPriceBSValueModel(void)
: LALinearRatesModel()
{}
// destructor
/*!*/
LAPriceBSValueModel::~LAPriceBSValueModel(void)
{}
// ! calcvalue
double 
LAPriceBSValueModel::calcValue(const AQLDataValuation& att, AQLDataProvider* dp, AQLObject& object, AQLString productname)
{
	const LALinearRatesOptionValue& bsval = dynamic_cast<const LALinearRatesOptionValue &>(att.getMethod());
	double ret = bsval.calcOption(att,dp,object);
	return ret;
}


//================ LAPriceVVValueModel ===================================
// constructor
/*!*/
LAPriceVVValueModel::LAPriceVVValueModel(void)
: LALinearRatesModel()
{}
// destructor
/*!*/
LAPriceVVValueModel::~LAPriceVVValueModel(void)
{}
// ! calcvalue
double 
LAPriceVVValueModel::calcValue(const AQLDataValuation& att, AQLDataProvider* dp, AQLObject& object, AQLString productname)
{
	double ret = 0.0;

	const LALinearRatesOptionValue& bsval = dynamic_cast<const LALinearRatesOptionValue &>(att.getMethod());
	double orgval = bsval.calcOption(att,dp,object);

	//get ATM RR BF * Vega Vanna Volga Matrix A
	LAPriceFXOptionValueDataProvider* dataProvider =  dynamic_cast<LAPriceFXOptionValueDataProvider*>(dataProvider);
	dataProvider->mVolfunc = dataProvider->mpvanilla->getFXVolFunc(dataProvider->mfxcur);
	const LAMathVolFuncFXVannaVolga& fxvol = dynamic_cast<const LAMathVolFuncFXVannaVolga&>(*(dataProvider->mVolfunc));
	
	//set gkOrg[i]
	std::vector <AnalyticGKParam *> gkOrg(dataProvider->mParam[0].size());
	for (unsigned int i = 0; i < dataProvider->mParam[0].size(); i++)
	{
		gkOrg[i] = dynamic_cast<AnalyticGKParam* >(dataProvider->mParam[0][i]);
	}

	std::vector <AnalyticGKParam *> gkvolgaVp(dataProvider->mParam[0].size());
	std::vector <AnalyticGKParam *> gkvolgaVm(dataProvider->mParam[0].size());
	std::vector <AnalyticGKParam *> gkvannaVpSp(dataProvider->mParam[0].size());
	std::vector <AnalyticGKParam *> gkvannaVmSm(dataProvider->mParam[0].size());
	std::vector <AnalyticGKParam *> gkvannaVpSm(dataProvider->mParam[0].size());
	std::vector <AnalyticGKParam *> gkvannaVmSp(dataProvider->mParam[0].size());
	double valvolgaVp,valvolgaVm,valvannaVpSp,valvannaVmSm,valvannaVpSm,valvannaVmSp;

	double voltiny = 0.0001;//0.001%
	double spottiny = 0.0001;//0.001 

	

	//calc valvolgaVp
	for (unsigned int i = 0; i < dataProvider->mParam[0].size(); i++)
	{
		gkvolgaVp[i] = dynamic_cast<AnalyticGKParam* >(gkOrg[i]->clone());
		gkvolgaVp[i]->Vol += voltiny; 
		dataProvider->mParam[0][i] = gkvolgaVp[i];
		
		valvolgaVp = bsval.calcOption(att,dataProvider,object);
	}
	
	//calc valvolgaVm
	for (unsigned int i = 0; i < dataProvider->mParam[0].size(); i++)
	{

		gkvolgaVm[i] = dynamic_cast<AnalyticGKParam* >(gkOrg[i]->clone());
		gkvolgaVm[i]->Vol -= voltiny;
		dataProvider->mParam[0][i] = gkvolgaVm[i];

		valvolgaVm = bsval.calcOption(att,dataProvider,object);
	}

	//calc valvannaVpSp
	for (unsigned int i = 0; i < dataProvider->mParam[0].size(); i++)
	{
		gkvannaVpSp[i] = dynamic_cast<AnalyticGKParam* >(gkOrg[i]->clone());
		gkvannaVpSp[i]->Vol += 0.5 * voltiny; 
		gkvannaVpSp[i]->S += 0.5 * spottiny;
		dataProvider->mParam[0][i] = gkvannaVpSp[i];

		valvannaVpSp = bsval.calcOption(att,dataProvider,object);
	}

	//calc valvannaVmSm
	for (unsigned int i = 0; i < dataProvider->mParam[0].size(); i++)
	{
		gkvannaVmSm[i] = dynamic_cast<AnalyticGKParam* >(gkOrg[i]->clone());
		gkvannaVmSm[i]->Vol -= 0.5 * voltiny; 
		gkvannaVmSm[i]->S -= 0.5 * spottiny;
		dataProvider->mParam[0][i] = gkvannaVmSm[i];

		valvannaVmSm = bsval.calcOption(att,dataProvider,object);
	}

	//calc valvannaVpSm
	for (unsigned int i = 0; i < dataProvider->mParam[0].size(); i++)
	{
		gkvannaVpSm[i] = dynamic_cast<AnalyticGKParam* >(gkOrg[i]->clone());
		gkvannaVpSm[i]->Vol += 0.5 * voltiny; 
		gkvannaVpSm[i]->S -= 0.5 * spottiny;
		dataProvider->mParam[0][i] = gkvannaVpSm[i];

		valvannaVpSm = bsval.calcOption(att,dataProvider,object);
	}

	//calc valvannaVmSp
	for (unsigned int i = 0; i < dataProvider->mParam[0].size(); i++)
	{
		gkvannaVmSp[i] = dynamic_cast<AnalyticGKParam* >(gkOrg[i]->clone());
		gkvannaVmSp[i]->Vol -= 0.5 * voltiny; 
		gkvannaVmSp[i]->S += 0.5 * spottiny;
		dataProvider->mParam[0][i] = gkvannaVmSp[i];

		valvannaVmSp = bsval.calcOption(att,dataProvider,object);
	}

	//calc volga
	double valvanna = ( valvannaVpSp + valvannaVmSm - valvannaVpSm - valvannaVmSp ) / voltiny / spottiny;
	double valvolga = ( valvolgaVp + valvolgaVm - 2.0 * orgval) / voltiny / voltiny;

	//////test volga
	//AQLString vannacall = AQLString(GK) + AQLString(VANNA) + AQLString(CALL);
	//AQLString volgacall = AQLString(GK) + AQLString(VOLGA) + AQLString(CALL);
	//std::map<AQLString, LABlackScholesBase*> &var = AQLCoreComponentManager::getBlackComponentMap();
	//std::map<AQLString, LABlackScholesBase*>::iterator itcalc = var.begin();
	//LABlackScholesBase* panalytic;
	//panalytic = var.find(vannacall)->second;
	//double testvanna =  panalytic->calc(*gkOrg[0]);
	//panalytic = var.find(volgacall)->second;
	//double testvolga =  panalytic->calc(*gkOrg[0]);



	for (unsigned int i = 0; i < dataProvider->mParam[0].size(); i++)
	{	
		delete gkvolgaVp[i];
		delete gkvolgaVm[i];
		delete gkvannaVpSp[i];
		delete gkvannaVmSm[i];
		delete gkvannaVpSm[i];
		delete gkvannaVmSp[i];
		
		dataProvider->mParam[0][i] = gkOrg[i];
	}

	//set VannaVolgaMatrix
	fxvol.setUpVannaVolgaMatrix(gkOrg[0], dataProvider);
	//term point (exp(-0.5 * v * v * termpoint) must be act/365;
	double termpoint = gkOrg[0]->Te;
	//get MarketPriceVec OmegaVega,OmegaVanna,OmegaVolga
	const DoubleVector& omegavec = fxvol.getMarketPriceVec(termpoint);

	double ntprob = 1.0;
	if (productname.findString("fn_fxsinglebarrieroptionvalue") != -1)
	{
		AQLString valname;

		AQLDataHolder* dh = &(object.getData(PRICING_DATA_UPANDDOWN, ISNOTNULL));
		AQLString updown = dynamic_cast<AQLDataString &>(dh->get()).get();
		updown.toUpper();
		
		if (updown == "DOWN")
		{
			valname = AQLString(SB) + AQLString(PROB)	+ AQLString(SBDOWN) + AQLString(NOTOUCH);
		}
		else if (updown == "UP")
		{
			valname = AQLString(SB) + AQLString(PROB)	+ AQLString(SBUP) + AQLString(NOTOUCH);
		}
		
		std::map<AQLString, LABlackScholesBase*> &var = AQLCoreComponentManager::getBlackComponentMap();
		std::map<AQLString, LABlackScholesBase*>::iterator itcalc = var.begin();
		itcalc = var.find(valname);
		ntprob = itcalc->second->calc(*dataProvider->mParam[0][0]);

		object.remove("ImplyVolFromVannaVolga");
		object.add("ImplyVolFromVannaVolga", new AQLDataDouble(ntprob));
	}
	ret = orgval + ntprob * valvanna * omegavec[1] + ntprob * valvolga * omegavec[2];
	//double testret = orgval + ntprob * testvanna * omegavec[1] + ntprob * testvolga * omegavec[2];
	ret = ret < 0.0 ? 0.0 : ret;


	if (productname.findString("fn_fxoptionvalue") != -1)
	{
		AQLString tmpType = PREM; AQLString tmpBuySell = BUY;
		AQLString optiontype = dynamic_cast<AQLDataString &>(object.getData(PRICING_DATA_OPTIONTYPE, ISNOTNULL).get()).get();
		optiontype.toUpper();
		const double high= /*0.5*/3.0; const double low = 0.00000001;
		double highprem = LAMathFXVanillaFuncUtility::gkOption
										(tmpType,tmpBuySell,optiontype,gkOrg[0]->S,gkOrg[0]->K,high,
										gkOrg[0]->rd,gkOrg[0]->rf,dataProvider->mAsofDate,dataProvider->mAsofDate,
										dataProvider->mMaturityDate,dataProvider->mDeliveryDate);
		double lowprem = LAMathFXVanillaFuncUtility::gkOption
										(tmpType,tmpBuySell,optiontype,gkOrg[0]->S,gkOrg[0]->K,low,
										gkOrg[0]->rd,gkOrg[0]->rf,dataProvider->mAsofDate,dataProvider->mAsofDate,
										dataProvider->mMaturityDate,dataProvider->mDeliveryDate);
		double implyvol = 0.0;
		if (ret == highprem)
			implyvol = high;
		else if (ret == lowprem)
			implyvol = low;
		else if (ret > highprem || lowprem > ret)
			implyvol = 0.0; // if it cannot solve implied vol, set zero.
		else
		{
			try
			{
				AQLString buysell = (dataProvider->buysell) ? "BUY" : "SELL";
				/*double implyvol = LAMathIRVanillaFuncUtility::bkOptionIV(buysell,
										optiontype,gkOrg[0]->S,gkOrg[0]->K,ret,
										gkOrg[0]->rd,dataProvider->mAsofDate,dataProvider->mAsofDate,
										dataProvider->mMaturityDate,dataProvider->mMaturityDate,high,low);*/
				implyvol = LAMathFXVanillaFuncUtility::gkOptionIV
											(buysell,optiontype,gkOrg[0]->S,gkOrg[0]->K,ret,
											gkOrg[0]->rd,gkOrg[0]->rf,dataProvider->mAsofDate,dataProvider->mAsofDate,
											dataProvider->mMaturityDate,dataProvider->mDeliveryDate,high,low);
			}
			catch(AQLCoreNumericalError e)
			{
				AQLString msg = e.getMsg();
				if (msg.findString("Not Convergence from rtsafe") == -1) throw e;
				implyvol = 0.0; // if it cannot solve implied vol, set zero.
			}
		}
		//test put callparity
		//double discount = AQLMath::exp(-gkOrg[0]->rd * gkOrg[0]->Td);
		//double discountfrn = AQLMath::exp(-gkOrg[0]->rf * gkOrg[0]->Td);

		/*double testimplyvol = LAMathFXVanillaFuncUtility::gkOptionIV
										(buysell,optiontype,gkOrg[0]->S,gkOrg[0]->K,testret,
										gkOrg[0]->rd,gkOrg[0]->rf,dataProvider->mAsofDate,dataProvider->mAsofDate,
										dataProvider->mMaturityDate,dataProvider->mDeliveryDate,high,low);*/


		object.remove("ImplyVolFromVannaVolga");
		object.add("ImplyVolFromVannaVolga", new AQLDataDouble(implyvol));
		//object.add("ImplyVolFromVannaVolga", new AQLDataDouble(discount));
		//object.add("ImplyVolFromVannaVolga", new AQLDataDouble(discountfrn));
	}

	//object.add("ImplyVolFromVannaVolga", new AQLDataDouble(testimplyvol));

	return ret;
}