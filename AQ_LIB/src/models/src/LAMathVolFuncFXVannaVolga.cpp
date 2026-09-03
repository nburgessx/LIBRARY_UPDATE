/*! @file
    @brief Source code of class to represent FX volatility function

	This class derives from LAFunctionBase

*/
//  2011, Mizuho International London.

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAMathVolFuncFXVannaVolga.h
//
//  SYNOPSIS    :       LAMathVolFuncFXVannaVolga
//  DESCRIPTION :       Source code of class  to represent volatility of FX
//						This class derives from LAFunctionBase
//                      
//  VERSION		:
////X///////////////////X///////////////////////////////X///////////////////

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LAMathVolFuncFXVannaVolga.h"
#include "LAAlgorithm.h"
#include "LAMathFXVolatilitySurfaceGenerate.h"
#include "LAAnalyticFormula.h"
#include "LAMathYieldCurve.h"
#include "LADataReference.h"
#include "LABlackScholesCalc.h"
#include "LACoreComponentManager.h"
#include "LAMatrix.h"
#include "LALinearRatesOptionValueDataProvider.h"
#include "LAMathFXEntity.h"

using namespace std;

//================ LAMathVolFuncFXVannaVolga ===================================
/*!
	@brief constructor

*/
LAMathVolFuncFXVannaVolga::LAMathVolFuncFXVannaVolga(LADataInstance* dataInstance, const ATMInterpolationMethod &atmMethod,
							const std::vector<FXOptionData> &opdata, const std::vector<SmileData> &smiledata,
							const DoubleVector& matuterms365, const LAString& dYieldDataName, const LAString& fYieldDataName,
							double spotrate)
:LAFunctionBase(),mpDataInstance(dataInstance), mAtmMethod(atmMethod), mOpData(opdata),mSmileData(smiledata), 
mMatuTerms365(matuterms365), mdYieldDataName(dYieldDataName),
mfYieldDataName(fYieldDataName),mSpotRate(spotrate)
{
}
/*!
	@brief destructor
*/
LAMathVolFuncFXVannaVolga::~LAMathVolFuncFXVannaVolga(void)
{
}


/*!
	@brief copy constructor
*/
LAMathVolFuncFXVannaVolga::LAMathVolFuncFXVannaVolga(const LAMathVolFuncFXVannaVolga &rhs)
: mpDataInstance(rhs.mpDataInstance), mAtmMethod(rhs.mAtmMethod), mOpData(rhs.mOpData), mSmileData(rhs.mSmileData),
mMatuTerms365(rhs.mMatuTerms365), mdYieldDataName(rhs.mdYieldDataName), 
mfYieldDataName(rhs.mfYieldDataName), mSpotRate(rhs.mSpotRate), mOmegaMap(rhs.mOmegaMap),mStrikeMap(rhs.mStrikeMap)
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LAMathVolFuncFXVannaVolga::clone() const
{
    try 
	{
		//return const_cast<LAMathVolFuncFXVannaVolga *>(this);
		return new LAMathVolFuncFXVannaVolga(*this);
	}
    catch (bad_alloc &e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief check function for this class ID
    @param[in] id ID to check
    @return true or false
*/
bool
LAMathVolFuncFXVannaVolga::isTypeOf(function_t id) const
{
	return (id == FN_VOLFUNCFXVANNAVOLGA ? true : LAFunctionBase::isTypeOf(id));
}

/*!
    @brief return this function type
    @return function type
*/
function_t
LAMathVolFuncFXVannaVolga::getType() const
{
	return FN_VOLFUNCFXVANNAVOLGA;
}


/*!
    @brief return function value
	@param[in] x point
    @return function value

	x[1] is fx value
*/
double
LAMathVolFuncFXVannaVolga::operator()(const DoubleArray& x) const
{
	return LAMathFXVolatilitySurfaceGenerate::GetATMVolatility(x[0],mAtmMethod,mOpData,mSmileData);
}

void
LAMathVolFuncFXVannaVolga::setUpVannaVolgaMatrix(AnalyticGKParam* gkParam, LADataProvider* dp) const
{
	LAPriceFXOptionValueDataProvider* dataProvider =  dynamic_cast<LAPriceFXOptionValueDataProvider*>(dp);

	//term point (exp(-0.5 * v * v * termpoint) must be act/365;
	double termpoint = gkParam->Te;
	std::map<double, DoubleVector>::iterator itMarketPrice = mOmegaMap.find(termpoint);

	if (itMarketPrice != mOmegaMap.end())
		return;
	

	
	//search nearest pos
	unsigned int nearestpos = 0;
	LAAlgorithm::locate<DoubleArray,double>(mMatuTerms365, termpoint, mMatuTerms365.size(), nearestpos);
	FXOptionData tmpData(mOpData[nearestpos]);
	
	//set df
	LAString daycount(AC_365I);
    //DOMESTICCURVEID
	LAMathYieldCurve dcurve(mpDataInstance);
	dcurve.getYieldData().convertFromString(mdYieldDataName);	
	dcurve.setInterpolation("fn_splineinterpolation");
    dcurve.getDayCount().setDayCount(daycount);
   	
	//FOREIGNCURVEID
	LAMathYieldCurve fcurve(mpDataInstance);
	fcurve.getYieldData().convertFromString(mfYieldDataName);	
	fcurve.setInterpolation("fn_splineinterpolation");
    fcurve.getDayCount().setDayCount(daycount);

	//reset tmpData
	tmpData.T = termpoint;
	tmpData.spotFX = mSpotRate;
	//this Td is temporary, we have to modify it
	LAMathFXEntity& fxe = dynamic_cast<LAMathFXEntity &>(dataProvider->mpvanilla->getFXEntity().get().get());
	unsigned int spotlag = fxe.getSpotLag(dataProvider->mfxcur);
	LAString spotlagday = LAString(static_cast<int>(spotlag)) + "D";
	//fxe.getCalendarNames().get();
	LADate deliverydate = fxe.getSpotDate(dataProvider->mdomcur,dataProvider->mforcur,dataProvider->mMaturityDate);
	LAPriceDataDayCount act365ISDA(ACT_365_ISDA);
	double Td = act365ISDA.getTerm(dataProvider->mAsofDate,deliverydate,true); 
	if (Td <= 0.0)
		throw LACoreInvalidData("DeliveryTerm Error for VannaVolga",__FILE__,__LINE__);
	tmpData.Pd = dcurve.getBasisDF(Td);
    tmpData.Pf = fcurve.getBasisDF(Td);
    tmpData.F = mSpotRate * tmpData.Pf / tmpData.Pd;
    

	//get BF and RR cost
	double lowvol = LAMathFXVolatilitySurfaceGenerate::GetVolatilityFromMaturityInterp(termpoint,mAtmMethod,mOpData,mSmileData, 0);
	double atmvol = LAMathFXVolatilitySurfaceGenerate::GetVolatilityFromMaturityInterp(termpoint,mAtmMethod,mOpData,mSmileData, 1);
	double highvol = LAMathFXVolatilitySurfaceGenerate::GetVolatilityFromMaturityInterp(termpoint,mAtmMethod,mOpData,mSmileData, 2);
	
	double lowstrike = LAMath::exp(LAMathFXVolatilitySurfaceGenerate::FindLogStrikeFromDelta(-0.25, lowvol, -1, tmpData ) ) * tmpData.F;
	//hishida 
	//double atmstrikeTemporary = LAMath::exp(LAMathFXVolatilitySurfaceGenerate::FindLogStrikeFromDelta(0.50, atmvol, 1, tmpData ) ) * tmpData.F;
	//double atmstrikeWatchDiff = LAMathFXVolatilitySurfaceGenerate::GetATMStrike(atmvol,tmpData);
	double atmstrike = tmpData.F * LAMath::exp(-0.5*atmvol*atmvol*tmpData.T);

	double highstrike = LAMath::exp(LAMathFXVolatilitySurfaceGenerate::FindLogStrikeFromDelta(0.25, highvol, 1, tmpData ) ) * tmpData.F;
	
	double rd = -LAMath::log(tmpData.Pd) / Td;
	double rf = -LAMath::log(tmpData.Pf) / Td;

	AnalyticGKParam* gkBase = dynamic_cast<AnalyticGKParam* >(gkParam->clone());
	gkBase->rd = rd;
	gkBase->rf = rf;
	gkBase->Td = Td;
	gkBase->F = tmpData.F;
	gkBase->Vol = atmvol;
	gkBase->S = mSpotRate;

	
	LAString premcall = LAString(GK) + LAString(PREM) + LAString(CALL);
	LAString premput = LAString(GK) + LAString(PREM) + LAString(PUT);
	LAString vegacall = LAString(GK) + LAString(VEGA) + LAString(CALL);
	LAString vegaput = LAString(GK) + LAString(VEGA) + LAString(PUT);
	LAString vannacall = LAString(GK) + LAString(VANNA) + LAString(CALL);
	LAString vannaput = LAString(GK) + LAString(VANNA) + LAString(PUT);
	LAString volgacall = LAString(GK) + LAString(VOLGA) + LAString(CALL);
	LAString volgaput = LAString(GK) + LAString(VOLGA) + LAString(PUT);

	std::map<LAString, LABlackScholesBase*> &var = LACoreComponentManager::getBlackComponentMap();
	std::map<LAString, LABlackScholesBase*>::iterator itcalc = var.begin();
	
	itcalc = var.find(premcall);
	LABlackScholesBase* panalytic;

	AnalyticGKParam* gkATMStrkBS = dynamic_cast<AnalyticGKParam* >(gkBase->clone());
	gkATMStrkBS->Vol = atmvol;
	gkATMStrkBS->K = atmstrike;
	
	AnalyticGKParam* gkLowStrk = dynamic_cast<AnalyticGKParam* >(gkBase->clone());
	gkLowStrk->Vol = lowvol;
	gkLowStrk->K = lowstrike;

	AnalyticGKParam* gkLowStrkBS = dynamic_cast<AnalyticGKParam* >(gkBase->clone());
	gkLowStrkBS->Vol = atmvol;
	gkLowStrkBS->K = lowstrike;

	AnalyticGKParam* gkHighStrk = dynamic_cast<AnalyticGKParam* >(gkBase->clone());
	gkHighStrk->Vol = highvol;
	gkHighStrk->K = highstrike;
	
	AnalyticGKParam* gkHighStrkBS = dynamic_cast<AnalyticGKParam* >(gkBase->clone());
	gkHighStrkBS->Vol = atmvol;
	gkHighStrkBS->K = highstrike;

	//get premium
	panalytic = var.find(premcall)->second;
	double atmprem = panalytic->calc(*gkATMStrkBS);
	double highprem = panalytic->calc(*gkHighStrk);
	double highBSprem = panalytic->calc(*gkHighStrkBS);
	panalytic = var.find(premput)->second;
	double lowprem = panalytic->calc(*gkLowStrk);
	double lowBSprem = panalytic->calc(*gkLowStrkBS);
	//get vega
	panalytic = var.find(vegacall)->second;
	double atmvega = panalytic->calc(*gkATMStrkBS);
	double highBSvega = panalytic->calc(*gkHighStrkBS);
	panalytic = var.find(vegaput)->second;
	double lowBSvega = panalytic->calc(*gkLowStrkBS);
	//get vanna
	panalytic = var.find(vannacall)->second;
	double atmvanna = panalytic->calc(*gkATMStrkBS);
	double highBSvanna = panalytic->calc(*gkHighStrkBS);
	panalytic = var.find(vannaput)->second;
	double lowBSvanna = panalytic->calc(*gkLowStrkBS);
	//get volga
	panalytic = var.find(volgacall)->second;
	double atmvolga = panalytic->calc(*gkATMStrkBS);
	double highBSvolga = panalytic->calc(*gkHighStrkBS);
	panalytic = var.find(volgaput)->second;
	double lowBSvolga = panalytic->calc(*gkLowStrkBS);

	DoubleMatrix AT(3,DoubleVector(3,0.0));
	//ATM vega, vanna volga
	AT[0][0] = atmvega;
	AT[0][1] = atmvanna;
	AT[0][2] = atmvolga;
	//RR vega, vanna volga
	AT[1][0] = highBSvega - lowBSvega;
	AT[1][1] = highBSvanna - lowBSvanna;
	AT[1][2] = highBSvolga - lowBSvolga;
	//BF vega, vanna volga
	AT[2][0] = 0.5 * ( highBSvega + lowBSvega) - atmvega;
	AT[2][1] = 0.5 * ( highBSvanna + lowBSvanna) - atmvanna;
	AT[2][2] = 0.5 * ( highBSvolga + lowBSvolga) - atmvolga;

	//cost vector (atm RR BF)
	DoubleVector I(3,0.0);
	I[0] = 0.0;
	I[1] =  (highprem - lowprem) - (highBSprem - lowBSprem);
	I[2] = (0.5 * (highprem + lowprem) - atmprem) - (0.5 * (highBSprem + lowBSprem) - atmprem);

	LAMatrix matAT(AT);
	const LAMatrix& invmat = matAT.inverseMatrix();
	LAMatrix matI(I);
	const LAMatrix& Omega = invmat * matI;
	DoubleVector ret;
	for (unsigned int i = 0; i < AT.size(); i++)
		ret.push_back(Omega.getValue(i,0));

	//insert map
	mOmegaMap.insert(std::make_pair(termpoint, ret));
	DoubleVector strksvec(3,atmstrike);
	strksvec[0] = lowstrike;
	strksvec[2] = highstrike;
	mStrikeMap.insert(std::make_pair(termpoint, strksvec));
	
	delete gkBase;
	delete gkATMStrkBS;
	delete gkLowStrk;
	delete gkLowStrkBS;
	delete gkHighStrk;
	delete gkHighStrkBS;

	return;
}

/*!
    @brief return function value
	@param[in] maturityTerm365 point
    @return MarketPriceVec

	rett[0] is OmegaVega
	rett[1] is OmegaVanna
	rett[2] is OmegaVolga

*/

const DoubleVector& 
LAMathVolFuncFXVannaVolga::getMarketPriceVec(double maturityTerm365) const
{
	std::map<double, DoubleVector>::iterator itMarketPrice = mOmegaMap.find(maturityTerm365);

	if (itMarketPrice == mOmegaMap.end())
		throw LACoreInvalidData("Term is not consistente with VannaVolgaMatrix",__FILE__,__LINE__);

	return itMarketPrice->second;
}

const DoubleVector& 
LAMathVolFuncFXVannaVolga::getStrikeVec(double maturityTerm365) const
{
	std::map<double, DoubleVector>::iterator itStrikeMap = mStrikeMap.find(maturityTerm365);

	if (itStrikeMap == mStrikeMap.end())
		throw LACoreInvalidData("Term is not consistente with VannaVolgaStikes",__FILE__,__LINE__);

	return itStrikeMap->second;
}

