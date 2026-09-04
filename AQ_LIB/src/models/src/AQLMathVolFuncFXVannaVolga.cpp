/*! @file
    @brief Source code of class to represent FX volatility function

	This class derives from AQLFunctionBase
*/


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLMathVolFuncFXVannaVolga.h"
#include "AQLAlgorithm.h"
#include "AQLMathFXVolatilitySurfaceGenerate.h"
#include "AQLAnalyticFormula.h"
#include "AQLMathYieldCurve.h"
#include "AQLDataReference.h"
#include "AQLBlackScholesCalc.h"
#include "AQLCoreComponentManager.h"
#include "AQLMatrix.h"
#include "AQLLinearRatesOptionValueDataProvider.h"
#include "AQLMathFXEntity.h"

using namespace std;

//================ AQLMathVolFuncFXVannaVolga ===================================
/*!
	@brief constructor

*/
AQLMathVolFuncFXVannaVolga::AQLMathVolFuncFXVannaVolga(AQLDataInstance* dataInstance, const ATMInterpolationMethod &atmMethod,
							const std::vector<FXOptionData> &opdata, const std::vector<SmileData> &smiledata,
							const DoubleVector& matuterms365, const AQLString& dYieldDataName, const AQLString& fYieldDataName,
							double spotrate)
:AQLFunctionBase(),mpDataInstance(dataInstance), mAtmMethod(atmMethod), mOpData(opdata),mSmileData(smiledata), 
mMatuTerms365(matuterms365), mdYieldDataName(dYieldDataName),
mfYieldDataName(fYieldDataName),mSpotRate(spotrate)
{
}
/*!
	@brief destructor
*/
AQLMathVolFuncFXVannaVolga::~AQLMathVolFuncFXVannaVolga(void)
{
}


/*!
	@brief copy constructor
*/
AQLMathVolFuncFXVannaVolga::AQLMathVolFuncFXVannaVolga(const AQLMathVolFuncFXVannaVolga &rhs)
: mpDataInstance(rhs.mpDataInstance), mAtmMethod(rhs.mAtmMethod), mOpData(rhs.mOpData), mSmileData(rhs.mSmileData),
mMatuTerms365(rhs.mMatuTerms365), mdYieldDataName(rhs.mdYieldDataName), 
mfYieldDataName(rhs.mfYieldDataName), mSpotRate(rhs.mSpotRate), mOmegaMap(rhs.mOmegaMap),mStrikeMap(rhs.mStrikeMap)
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLMathVolFuncFXVannaVolga::clone() const
{
    try 
	{
		//return const_cast<AQLMathVolFuncFXVannaVolga *>(this);
		return new AQLMathVolFuncFXVannaVolga(*this);
	}
    catch (bad_alloc &e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief check function for this class ID
    @param[in] id ID to check
    @return true or false
*/
bool
AQLMathVolFuncFXVannaVolga::isTypeOf(function_t id) const
{
	return (id == FN_VOLFUNCFXVANNAVOLGA ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
    @brief return this function type
    @return function type
*/
function_t
AQLMathVolFuncFXVannaVolga::getType() const
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
AQLMathVolFuncFXVannaVolga::operator()(const DoubleArray& x) const
{
	return AQLMathFXVolatilitySurfaceGenerate::GetATMVolatility(x[0],mAtmMethod,mOpData,mSmileData);
}

void
AQLMathVolFuncFXVannaVolga::setUpVannaVolgaMatrix(AnalyticGKParam* gkParam, AQLDataProvider* dp) const
{
	AQLPriceFXOptionValueDataProvider* dataProvider =  dynamic_cast<AQLPriceFXOptionValueDataProvider*>(dp);

	//term point (exp(-0.5 * v * v * termpoint) must be act/365;
	double termpoint = gkParam->Te;
	std::map<double, DoubleVector>::iterator itMarketPrice = mOmegaMap.find(termpoint);

	if (itMarketPrice != mOmegaMap.end())
		return;
	

	
	//search nearest pos
	unsigned int nearestpos = 0;
	AQLAlgorithm::locate<DoubleArray,double>(mMatuTerms365, termpoint, mMatuTerms365.size(), nearestpos);
	FXOptionData tmpData(mOpData[nearestpos]);
	
	//set df
	AQLString daycount(AC_365I);
    //DOMESTICCURVEID
	AQLMathYieldCurve dcurve(mpDataInstance);
	dcurve.getYieldData().convertFromString(mdYieldDataName);	
	dcurve.setInterpolation("fn_splineinterpolation");
    dcurve.getDayCount().setDayCount(daycount);
   	
	//FOREIGNCURVEID
	AQLMathYieldCurve fcurve(mpDataInstance);
	fcurve.getYieldData().convertFromString(mfYieldDataName);	
	fcurve.setInterpolation("fn_splineinterpolation");
    fcurve.getDayCount().setDayCount(daycount);

	//reset tmpData
	tmpData.T = termpoint;
	tmpData.spotFX = mSpotRate;
	//this Td is temporary, we have to modify it
	AQLMathFXEntity& fxe = dynamic_cast<AQLMathFXEntity &>(dataProvider->mpvanilla->getFXEntity().get().get());
	unsigned int spotlag = fxe.getSpotLag(dataProvider->mfxcur);
	AQLString spotlagday = AQLString(static_cast<int>(spotlag)) + "D";
	//fxe.getCalendarNames().get();
	AQLDate deliverydate = fxe.getSpotDate(dataProvider->mdomcur,dataProvider->mforcur,dataProvider->mMaturityDate);
	AQLPriceDataDayCount act365ISDA(ACT_365_ISDA);
	double Td = act365ISDA.getTerm(dataProvider->mAsofDate,deliverydate,true); 
	if (Td <= 0.0)
		throw AQLCoreInvalidData("DeliveryTerm Error for VannaVolga",__FILE__,__LINE__);
	tmpData.Pd = dcurve.getBasisDF(Td);
    tmpData.Pf = fcurve.getBasisDF(Td);
    tmpData.F = mSpotRate * tmpData.Pf / tmpData.Pd;
    

	//get BF and RR cost
	double lowvol = AQLMathFXVolatilitySurfaceGenerate::GetVolatilityFromMaturityInterp(termpoint,mAtmMethod,mOpData,mSmileData, 0);
	double atmvol = AQLMathFXVolatilitySurfaceGenerate::GetVolatilityFromMaturityInterp(termpoint,mAtmMethod,mOpData,mSmileData, 1);
	double highvol = AQLMathFXVolatilitySurfaceGenerate::GetVolatilityFromMaturityInterp(termpoint,mAtmMethod,mOpData,mSmileData, 2);
	
	double lowstrike = AQLMath::exp(AQLMathFXVolatilitySurfaceGenerate::FindLogStrikeFromDelta(-0.25, lowvol, -1, tmpData ) ) * tmpData.F;
	//hishida 
	//double atmstrikeTemporary = AQLMath::exp(AQLMathFXVolatilitySurfaceGenerate::FindLogStrikeFromDelta(0.50, atmvol, 1, tmpData ) ) * tmpData.F;
	//double atmstrikeWatchDiff = AQLMathFXVolatilitySurfaceGenerate::GetATMStrike(atmvol,tmpData);
	double atmstrike = tmpData.F * AQLMath::exp(-0.5*atmvol*atmvol*tmpData.T);

	double highstrike = AQLMath::exp(AQLMathFXVolatilitySurfaceGenerate::FindLogStrikeFromDelta(0.25, highvol, 1, tmpData ) ) * tmpData.F;
	
	double rd = -AQLMath::log(tmpData.Pd) / Td;
	double rf = -AQLMath::log(tmpData.Pf) / Td;

	AnalyticGKParam* gkBase = dynamic_cast<AnalyticGKParam* >(gkParam->clone());
	gkBase->rd = rd;
	gkBase->rf = rf;
	gkBase->Td = Td;
	gkBase->F = tmpData.F;
	gkBase->Vol = atmvol;
	gkBase->S = mSpotRate;

	
	AQLString premcall = AQLString(GK) + AQLString(PREM) + AQLString(CALL);
	AQLString premput = AQLString(GK) + AQLString(PREM) + AQLString(PUT);
	AQLString vegacall = AQLString(GK) + AQLString(VEGA) + AQLString(CALL);
	AQLString vegaput = AQLString(GK) + AQLString(VEGA) + AQLString(PUT);
	AQLString vannacall = AQLString(GK) + AQLString(VANNA) + AQLString(CALL);
	AQLString vannaput = AQLString(GK) + AQLString(VANNA) + AQLString(PUT);
	AQLString volgacall = AQLString(GK) + AQLString(VOLGA) + AQLString(CALL);
	AQLString volgaput = AQLString(GK) + AQLString(VOLGA) + AQLString(PUT);

	std::map<AQLString, AQLBlackScholesBase*> &var = AQLCoreComponentManager::getBlackComponentMap();
	std::map<AQLString, AQLBlackScholesBase*>::iterator itcalc = var.begin();
	
	itcalc = var.find(premcall);
	AQLBlackScholesBase* panalytic;

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

	AQLMatrix matAT(AT);
	const AQLMatrix& invmat = matAT.inverseMatrix();
	AQLMatrix matI(I);
	const AQLMatrix& Omega = invmat * matI;
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
AQLMathVolFuncFXVannaVolga::getMarketPriceVec(double maturityTerm365) const
{
	std::map<double, DoubleVector>::iterator itMarketPrice = mOmegaMap.find(maturityTerm365);

	if (itMarketPrice == mOmegaMap.end())
		throw AQLCoreInvalidData("Term is not consistente with VannaVolgaMatrix",__FILE__,__LINE__);

	return itMarketPrice->second;
}

const DoubleVector& 
AQLMathVolFuncFXVannaVolga::getStrikeVec(double maturityTerm365) const
{
	std::map<double, DoubleVector>::iterator itStrikeMap = mStrikeMap.find(maturityTerm365);

	if (itStrikeMap == mStrikeMap.end())
		throw AQLCoreInvalidData("Term is not consistente with VannaVolgaStikes",__FILE__,__LINE__);

	return itStrikeMap->second;
}

