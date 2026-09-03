/*! @file
    @brief LALinearRatesVolatility manager class
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LALinearRatesVolatility.cpp
//
//  DESCRIPTION :       LALinearRatesVolatility 
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
#include "LALinearRatesVolatility.h"
#include "LAMathVolFuncFXStrangleSolver.h"
//hishida vannavolga
#include "LAMathVolFuncFXVannaVolga.h"
#include "LALinearRatesOptionValueDataProvider.h"
#include "AQLDataVector.h"
#include "AQLDataMultiReference.h"
#include "LAPriceCashFlowGenerator.h"
#include "AQLMathDefine.h"
#include "LALinearRatesOptionValue.h"
#include "LAMathVolFuncIRSABR.h"
#include "LAMathDateCalculations.h"
#include "LAMathSwaptionVolUtility.h"
#include "LAMathYieldCurve.h"
#include "AQLDataReference.h"
#include "LAMathSABR.h"
#include "LAMathCurveFuncUtility.h"
#include "AQLDataMatrix.h"
#include "LAMathYieldCurvePro.h"

using namespace std;

//================ LALinearRatesVolatility ===================================
// constructor
/*!*/
LALinearRatesVolatility::LALinearRatesVolatility(void)
{}
// destructor
/*!*/
LALinearRatesVolatility::~LALinearRatesVolatility(void)
{}
// ! setupvol
void 
LALinearRatesVolatility::setVolatility(AQLDataProvider* dp, AQLObject& object, AQLString model)
{
	return;
}

//================ LAPricePlainVolatilityFromDirectInput ===================================
// constructor
/*!*/
LAPricePlainVolatilityFromDirectInput::LAPricePlainVolatilityFromDirectInput(void)
{}
// destructor
/*!*/
LAPricePlainVolatilityFromDirectInput::~LAPricePlainVolatilityFromDirectInput(void)
{}
// ! setupvol
void 
LAPricePlainVolatilityFromDirectInput::setVolatility(AQLDataProvider* dp, AQLObject& object, AQLString model)
{
	AQLDataHolder* dh = &(object.getData(PRICING_DATA_VOLATILITYDIRECTINPUT,ISDEFINED));
	double volval = dynamic_cast<AQLDataDouble &>(dh->get()).get();

	LALinearRatesOptionValueDataProvider* dataProvider = dynamic_cast<LALinearRatesOptionValueDataProvider*>(dp);
	for (unsigned int i = 0; i < dataProvider->mParam[0].size(); i++)
	{
		dataProvider->mParam[0][i]->Vol = volval; 
	}

	return;
}

//================ LAPricePlainVolatilityFromDirectInputOfCahslets ===================================
// constructor
/*!*/
LAPricePlainVolatilityFromDirectInputOfCashlets::LAPricePlainVolatilityFromDirectInputOfCashlets(void)
{}
// destructor
/*!*/
LAPricePlainVolatilityFromDirectInputOfCashlets::~LAPricePlainVolatilityFromDirectInputOfCashlets(void)
{}
// ! setupvol
void 
LAPricePlainVolatilityFromDirectInputOfCashlets::setVolatility(AQLDataProvider* dp, AQLObject& object, AQLString model)
{
	LALinearRatesOptionValueDataProvider* dataProvider = dynamic_cast<LALinearRatesOptionValueDataProvider*>(dp);

	AQLDataHolder* dh = &(object.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL));
	AQLDataMultiReference& legs = dynamic_cast<AQLDataMultiReference &>(dh->get());
	AQLObject& leg = legs.get(0).get();

	dh = &(leg.getData(PRICING_DATA_CASHLETS, ISNOTNULL));
	AQLDataMultiReference& cashlets = dynamic_cast<AQLDataMultiReference &>(dh->get());
	unsigned N = cashlets.getSize();
	//check
	if (N != dataProvider->mParam.size())
		throw AQLCoreInvalidData("Cashlets size error",__FILE__,__LINE__);

	for (unsigned int i = 0; i < N; i++)
	{
		if (dataProvider->mParam[i].size() == 0)
			continue;

		AQLObject& ecash = cashlets.get(i).get();
		dh = &(ecash.getData(PRICING_DATA_COUPONINFOS, ISNOTNULL));
		AQLDataMultiReference& coupons = dynamic_cast<AQLDataMultiReference &>(dh->get());
		AQLObject& ecoupon = coupons.get(0).get();

		dh = &(ecoupon.getData(PRICING_DATA_VOLATILITYDIRECTINPUTS,ISDEFINED));
		const DoubleArray& vols = dynamic_cast<const AQLDataDoubles &>(dh->get()).get();
		if (vols.size() != dataProvider->mParam[i].size())
			throw AQLCoreInvalidData("Volatility size error",__FILE__,__LINE__);

		for (unsigned int j = 0; j < dataProvider->mParam[i].size(); j++)
		{
			dataProvider->mParam[i][j]->Vol = vols[j]; 
		}
	}

	return;
}

//================ LAPriceFXStrangleSolverVolatility ===================================
// constructor
/*!

*/
LAPriceFXStrangleSolverVolatility::LAPriceFXStrangleSolverVolatility(void)
:LALinearRatesVolatility()
{
}
// destructor
/*!

*/
LAPriceFXStrangleSolverVolatility::~LAPriceFXStrangleSolverVolatility(void)
{ 
}
// ! setupvol
void 
LAPriceFXStrangleSolverVolatility::setVolatility(AQLDataProvider* dp, AQLObject& object, AQLString model)
{
	
	DoubleVector volvec(2,0.0);
	
	LAPriceFXOptionValueDataProvider* dataProvider = dynamic_cast<LAPriceFXOptionValueDataProvider*>(dp);
	dataProvider->mVolfunc = dataProvider->mpvanilla->getFXVolFunc(dataProvider->mfxcur);
	const LAMathVolFuncFXStrangleSolver& fxvol = dynamic_cast<const LAMathVolFuncFXStrangleSolver&>(*(dataProvider->mVolfunc));
	ATMInterpolationMethod interpatm = fxvol.getATMInterpolationMethod();
	InterpolationVariable interpvari = fxvol.getInterpolationVariable();

	std::vector <AnalyticGKParam *> gkparam(dataProvider->mParam[0].size());
	for (unsigned int i = 0; i < dataProvider->mParam[0].size(); i++)
	{

		gkparam[i] = dynamic_cast<AnalyticGKParam* >(dataProvider->mParam[0][i]);

		if (interpvari == VariableLogStrike && gkparam[i]->F != 0.0)
			volvec[0] = AQLMath::log(gkparam[i]->K/gkparam[i]->F);
		else
			throw AQLCoreInvalidData("Not support now", __FILE__,__LINE__);
	
		if (interpatm == TermWeighted || TermNoWeighted)
		{
			//volvec[1]= gkparam[i]->actT;
			//hishida vannavolga
			volvec[1]= gkparam[i]->Te;
		}
		else
			volvec[1]=  static_cast<double>(dataProvider->mMaturityDate.intervalDays(dataProvider->mAsofDate));
		
		gkparam[i]->Vol = (dataProvider->mVolfunc)->operator ()(volvec);
	}
	
	return;
}

//================ LAPriceFXStrangleSolverATMVolatility ===================================
// constructor
/*!

*/
LAPriceFXStrangleSolverATMVolatility::LAPriceFXStrangleSolverATMVolatility(void)
:LALinearRatesVolatility()
{
}
// destructor
/*!

*/
LAPriceFXStrangleSolverATMVolatility::~LAPriceFXStrangleSolverATMVolatility(void)
{ 
}
// ! setupvol
void 
LAPriceFXStrangleSolverATMVolatility::setVolatility(AQLDataProvider* dp, AQLObject& object, AQLString model)
{
	DoubleVector volvec(1,0.0);
	
	LAPriceFXOptionValueDataProvider* dataProvider =  dynamic_cast<LAPriceFXOptionValueDataProvider*>(dp);
	dataProvider->mVolfunc = dataProvider->mpvanilla->getFXVolFunc(dataProvider->mfxcur);
	const LAMathVolFuncFXStrangleSolver& fxvol = dynamic_cast<const LAMathVolFuncFXStrangleSolver&>(*(dataProvider->mVolfunc));
	ATMInterpolationMethod interpatm = fxvol.getATMInterpolationMethod();
	InterpolationVariable interpvari = fxvol.getInterpolationVariable();

	std::vector <AnalyticGKParam *> gkparam(dataProvider->mParam[0].size());
	for (unsigned int i = 0; i < dataProvider->mParam[0].size(); i++)
	{

		gkparam[i] = dynamic_cast<AnalyticGKParam* >(dataProvider->mParam[0][i]);

		
		if (interpatm == TermWeighted || TermNoWeighted)
		{
			//volvec[0]= gkparam[i]->actT;
			//hishida vannavolga
			volvec[0]= gkparam[i]->Te;
		}
		else
			volvec[0]=  static_cast<double>(dataProvider->mMaturityDate.intervalDays(dataProvider->mAsofDate));
		
		gkparam[i]->Vol = (dataProvider->mVolfunc)->operator ()(volvec);
	}
	return;
}

//hishida vannavolga
//================ LAPriceFXStrangleSolverVolatility ===================================
// constructor
/*!

*/
LAPriceFXVannaVolgaVolatility::LAPriceFXVannaVolgaVolatility(void)
:LALinearRatesVolatility()
{
}
// destructor
/*!

*/
LAPriceFXVannaVolgaVolatility::~LAPriceFXVannaVolgaVolatility(void)
{ 
}
// ! setupvol
void 
LAPriceFXVannaVolgaVolatility::setVolatility(AQLDataProvider* dp, AQLObject& object, AQLString model)
{

	//temporary after that, we must edit 
	DoubleVector volvec(1,0.0);
	
	LAPriceFXOptionValueDataProvider* dataProvider =  dynamic_cast<LAPriceFXOptionValueDataProvider*>(dp);
	dataProvider->mVolfunc = dataProvider->mpvanilla->getFXVolFunc(dataProvider->mfxcur);
	const LAMathVolFuncFXVannaVolga& fxvol = dynamic_cast<const LAMathVolFuncFXVannaVolga&>(*(dataProvider->mVolfunc));
	ATMInterpolationMethod interpatm = fxvol.getATMInterpolationMethod();
	
	std::vector <AnalyticGKParam *> gkparam(dataProvider->mParam[0].size());
	for (unsigned int i = 0; i < dataProvider->mParam[0].size(); i++)
	{

		gkparam[i] = dynamic_cast<AnalyticGKParam* >(dataProvider->mParam[0][i]);

		if (interpatm == TermWeighted || interpatm == TermNoWeighted)
		{	
			//volvec[0]= gkparam[i]->actT;
			//vannavolga to be constitent with FXStrangleSolver
			volvec[0]= gkparam[i]->Te;
		}

		else
			volvec[0]=  static_cast<double>(dataProvider->mMaturityDate.intervalDays(dataProvider->mAsofDate));
		
		gkparam[i]->Vol = (dataProvider->mVolfunc)->operator ()(volvec);
	}
	return;
	
	
	
	return;
}


//================ LAPriceIRSABRVolatility ===================================
// constructor
/*!

*/
LAPriceIRSABRVolatility::LAPriceIRSABRVolatility(void)
:LALinearRatesVolatility()
{
}
// destructor
/*!

*/
LAPriceIRSABRVolatility::~LAPriceIRSABRVolatility(void)
{ 
}
// ! setupvol
void 
LAPriceIRSABRVolatility::setVolatility(AQLDataProvider* dp, AQLObject& object, AQLString model)
{
	DoubleVector volvec(4,0.0);
	
	LAPriceIROptionValueDataProvider* dataProvider =  dynamic_cast<LAPriceIROptionValueDataProvider*>(dp);
	
	//dataProvider->mVolfunc = dataProvider->mpvanilla->getIRVolFunc(dataProvider->mpvcur);
	dataProvider->mVolfunc = dataProvider->mpvanilla->getIRVolFunc(dataProvider->mnumerairecur);
	const LAMathVolFuncIRSABR& irvol = dynamic_cast<const LAMathVolFuncIRSABR&>(*(dataProvider->mVolfunc));

	const LAMathYieldCurve &yc = dataProvider->mpvanilla->getIRCurve(dataProvider->mnumerairecur);
	AQLString curveid = dynamic_cast<const AQLDataString &>(yc.getYieldData().get().getData(CALIBRATION_DATA_NAME,ISNOTNULL).get()).get();
	
	
	//dataProvider->mUnTenor == NULL means we do not use volatility
	if (dataProvider->mUnTenor.size() <= 0)
		return ;

	AQLString convid;	
	int y,m,d,w;

	const AQLDate &asofdate = dataProvider->mpvanilla->getAsOfDate();
	if (dataProvider->mParam.size() != dataProvider->mMaturityDates.size() ||
		dataProvider->mParam.size() != dataProvider->mFCurveTypes.size() )
		throw AQLCoreInvalidData("Cashlets size error",__FILE__,__LINE__);

	/*if (dataProvider->mIsFWDInter)
	{

		DoubleArray fwds;
		DoubleArray terms;
		DoubleArray taus;
		DoubleMatrix fwdsGridMat;
		yc.getBaseForwardRate(dataProvider->mFCurveType, terms, fwdsGridMat, taus, fwds);
		dataProvider->mpFWDInter->set(terms, fwds);
	}*/
	
	for (unsigned int i = 0; i < dataProvider->mParam.size(); i++)
	{
		if (dataProvider->mParam[i].size() == 0)
			continue;
		
		std::vector <AnalyticBKParam *> bkparam(dataProvider->mParam[i].size());
		irvol.setUnderlying(dataProvider->mUnderlying[i]);
		double mtenorval = 0.;
		if (asofdate < dataProvider->mMaturityDates[i])
		{
			LAMathDateCalculations::termStrtoYMDW(dataProvider->mUnTenor[i], y, m, d, w);
			mtenorval = static_cast<double > (y) + static_cast<double > (m) / 12.;
			//select convid this is temporary now
			if (mtenorval <= 0.5)
				convid = irvol.getCapConvID();
			else
				convid = irvol.getSwapConvID();
		}

		const double forwardShiftValue(irvol.getForwardShiftValue());

		for (unsigned int j = 0; j < dataProvider->mParam[i].size(); j++)
		{
			// past date
			if (asofdate >= dataProvider->mMaturityDates[i])
			{
				dataProvider->mParam[i][j]->Vol = 0.0;
			}
			else
			{
				//volvec[0] expiry point
				//volvec[1] tenor point
				//volvec[2] forward
				//volvec[3] strike
				bkparam[j] = dynamic_cast<AnalyticBKParam* >(dataProvider->mParam[i][j]);
				volvec[0] = bkparam[j]->actT;
				volvec[0] = (volvec[0] >= 0.0) ? volvec[0] : 0.0;
				volvec[1] = mtenorval;
				volvec[2] = (dataProvider->mIsFRateFromCurve) ? LAMathSwaptionVolUtility::getForward(	object.getDataInstance(),
																							dataProvider->mMaturityDates[i],
																							dataProvider->mUnTenor[i],
																							curveid,
																							convid,
																							dataProvider->mFCurveTypes[i],
																							dataProvider->mDCurveTypes[i],
																							dataProvider->mIsFWDInter) 
														: bkparam[j]->F;
				volvec[2] += forwardShiftValue;
				volvec[3] = bkparam[j]->K;
				volvec[3] += forwardShiftValue;
				if (irvol.getApproxmethod() == APPROXIMATION_HAGAN)
				{
					volvec[2] = (volvec[2] > eps_SABR) ? volvec[2] : eps_SABR;
					volvec[3] = (volvec[3] > eps_SABR) ? volvec[3] : eps_SABR;
				}
				else
				{
					volvec[2] = AQLMath::max(volvec[2], 0.0001);
					volvec[3] = AQLMath::max(volvec[3], 0.0001);
				}
				dataProvider->mParam[i][j]->Vol = (dataProvider->mVolfunc)->operator ()(volvec);
			}
		}
	}

	return;
}

//================ LAPricePlainVolatilityPVVolMatrixUse ===================================
// constructor
/*!

*/
LAPricePlainVolatilityPVVolMatrixUse::LAPricePlainVolatilityPVVolMatrixUse(void)
:LALinearRatesVolatility()
{
}
// destructor
/*!

*/
LAPricePlainVolatilityPVVolMatrixUse::~LAPricePlainVolatilityPVVolMatrixUse(void)
{ 
}
// ! setupvol
void 
LAPricePlainVolatilityPVVolMatrixUse::setVolatility(AQLDataProvider* dp, AQLObject& object, AQLString model)
{
	LALinearRatesOptionValueDataProvider* dataProvider = dynamic_cast<LALinearRatesOptionValueDataProvider*>(dp);
	AQLDataHolder* dh = &(object.getData(PRICING_DATA_PVVOLMATRIX, ISNOTNULL));
	const DoubleMatrix& volmat = dynamic_cast<const AQLDataDoubleMatrix &>(dh->get()).get();
	if (volmat.size() != dataProvider->mParam.size())
		throw AQLCoreInvalidData("Volatility Matrix Size Error",__FILE__,__LINE__);

	for (unsigned int i = 0; i < dataProvider->mParam.size(); i++)
	{
		if (volmat[i].size() != dataProvider->mParam[i].size())
			throw AQLCoreInvalidData("Volatility Matrix Size Error",__FILE__,__LINE__);

		if (dataProvider->mParam[i].size() == 0)
			continue;
		for (unsigned int j = 0; j < dataProvider->mParam[i].size(); j++)
		{
			dataProvider->mParam[i][j]->Vol = volmat[i][j];
		}
	}
	return;
}



