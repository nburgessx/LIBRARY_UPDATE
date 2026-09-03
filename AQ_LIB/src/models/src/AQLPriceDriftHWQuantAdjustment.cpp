/*! @file
    @brief Source code of drift function of quant adjustment of HW model



*/
//  2008, AlgoQuantHub.. 

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLPriceDriftHWQuantAdjustment.cpp
//
//  SYNOPSIS    :       AQLPriceDriftHWQuantAdjustment
//  DESCRIPTION :       Source code of drift function of quant adjustment of HW model
//
//  SEE ALSO    :       
//  TYME        :       CLASS
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLPriceDriftHWQuantAdjustment.h"
#include "AQLPriceFXVolatility.h"
#include "AQLMathPathEntity.h"
#include "AQLDataHolder.h"
#include "AQLDataVector.h"
#include "AQLDataReference.h"
#include "AQLObjectHolder.h"
#include "AQLMathAttrSDE.h"
#include "AQLPriceDataFunction.h"
#include "AQLMathVolFuncBase.h"
#include "AQLRatesSpotSDE.h"
#include "AQLModelDynamicsCurve.h"
#include "AQLModelDynamicsScalar.h"
#include "AQLAlgorithm.h"
#include "AQLConstant.h"
#include "AQL1DDataSet.h"
#include "AQLSplineInterpolation.h"
#include "AQLStepInterpolation.h"
#include "AQLGaussLegendre.h"
#include "AQLCombinationFunc.h"
#include "AQLBasic.h"
#include "AQLPriceDriftHW.h"
#include "AQLMathVolFuncFXDD.h"

using namespace std;

#define GAUSSLEGENDREPOINTNUM 20


//================ AQLPriceDriftHWQuantAdjustment ===================================
/*!
	@brief default constructor
	@param[in] pDriftIR drift class before quant adjustment
*/
AQLPriceDriftHWQuantAdjustment::AQLPriceDriftHWQuantAdjustment(const AQLRatesNumeraireBase& domestic_nu, const AQLMathVolFuncFXDD& fxvol, AQLRatesSpotSDE& fx_sde, 
									AQLPriceDriftHW* pDriftIR, const DoubleArray& cor,
									double fx_criteria)
: mpNumeraireD(&domestic_nu), mpFxVolatility(&fxvol), mpSDEFX(&fx_sde), 
mpDriftIR(pDriftIR), mCorrelation(cor), mIsSetUped(false), mPos_old(0), mFxCriteria(fx_criteria)
{
	if (pDriftIR == 0)
	{
		//error
		throw AQLCoreInvalidData("input IR drift is NULL", __FILE__, __LINE__);
	}
}



/*!
	@brief default constructor
	@param[in] sdeAttrNameIR_D data name of domestic ir model
	@param[in] sdeAttrNameIR_F data name of foreign ir model
	@param[in] sdeAttrNameFX data name of fx model
	@param[in] pDriftIR drift class before quant adjustment
*/
AQLPriceDriftHWQuantAdjustment::AQLPriceDriftHWQuantAdjustment(const AQLString& sdeAttrNameIR_D, 
													   const AQLString& sdeAttrNameIR_F, 
													   const AQLString& sdeAttrNameFX, 
													   AQLPriceDriftHW* pDriftIR,
													   double fx_criteria)
: mpNumeraireD(0), mpFxVolatility(0), mpSDEFX(0),
mSDEAttrNameIR_D(sdeAttrNameIR_D), mSDEAttrNameIR_F(sdeAttrNameIR_F), mSDEAttrNameFX(sdeAttrNameFX), mpDriftIR(pDriftIR)
, mIsSetUped(false), mPos_old(0), mFxCriteria(fx_criteria)
{
	if (pDriftIR == 0)
	{
		//error
		throw AQLCoreInvalidData("input IR drift is NULL", __FILE__, __LINE__);
	}
}


/*!
	@brief copy constructor
	@param[in] v copy source
*/
AQLPriceDriftHWQuantAdjustment::AQLPriceDriftHWQuantAdjustment(const AQLPriceDriftHWQuantAdjustment& v) 
: AQLMathDriftFuncBase(v), 
mpNumeraireD(v.mpNumeraireD),
mpFxVolatility(v.mpFxVolatility),
mpSDEFX(v.mpSDEFX), 
mpDriftIR(0),
mCorrelation(v.mCorrelation),
mMinFx(v.mMinFx),
mSDEAttrNameIR_D(v.mSDEAttrNameIR_D),
mSDEAttrNameIR_F(v.mSDEAttrNameIR_F),
mSDEAttrNameFX(v.mSDEAttrNameFX),
mIsSetUped(v.mIsSetUped),	
mPos_old(v.mPos_old),
mCache1(v.mCache1),	
mCache2(v.mCache2),		
mCache3(v.mCache3),
mFxCriteria(v.mFxCriteria)
{

	if (v.mpDriftIR != 0)
		mpDriftIR = dynamic_cast<AQLPriceDriftHW*>(v.mpDriftIR->clone());
}

/*!
	@brief destructor
*/
AQLPriceDriftHWQuantAdjustment::~AQLPriceDriftHWQuantAdjustment() 
{
	if (mpDriftIR != 0) delete mpDriftIR;
}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLPriceDriftHWQuantAdjustment::clone() const	
{
    try 
	{
		return new AQLPriceDriftHWQuantAdjustment(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}
/*!
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLPriceDriftHWQuantAdjustment::isTypeOf(function_t id) const
{
	return (id==FN_DRIFTHWQUANTADJ ? true : AQLMathDriftFuncBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLPriceDriftHWQuantAdjustment::getType() const
{
	return FN_DRIFTHWQUANTADJ;
}


/*!
    @brief Return drift value
	@param[in] x input value, x[0] = ts, x[1] = te, x[2] = rf
    @return drift value

*/
double
AQLPriceDriftHWQuantAdjustment::operator()(const DoubleArray& x) const
{
	if (!mIsSetUped) setUp();
	
	const DoubleArray& timegrid = mpSDEFX->getBM()->getTimeGrid();
	unsigned int pos;
	if (x[0] == 0.0) pos = 0;
	else if (x[0] == timegrid[mPos_old]) pos = mPos_old;
	else if (mPos_old + 2 < timegrid.size() && x[0] == timegrid[mPos_old + 1]) pos = mPos_old + 1;
	else if (!AQLAlgorithm::find<DoubleArray, double>(timegrid, x[0], 0, timegrid.size() - 1, pos))
	{
		//error
		AQLString msg = "Time =" + AQLDataDouble(x[0]).convertToString();
		msg += " is not in sde integral time grid";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	
	//fx
	double fx = mpSDEFX->getPathElement(pos)->get()[0];
	
	double fx_=fx;
	if (fx >= 0 && fx < mMinFx[pos]) fx_ = mMinFx[pos];
	else if (fx < 0 && fx >= -mMinFx[pos]) fx_= -mMinFx[pos];

	//rd
	double rd = mpNumeraireD->getCurve(x[0]).getF(x[0]);
	
	double quantAdj = mCache1[pos] + mCache2[pos] / fx_ + (x[2] - rd) * mCache3[pos] / fx_;
	
	mPos_old = pos;
	
	return (*mpDriftIR)(x) + quantAdj;
}


/*!
    @brief return string representaion
    @return string representaion (domestic ir sde attr name : foreign ir sde attr name : fx sde attr name : mpDriftIR.convertToString() : fxcriteria)
*/
AQLString
AQLPriceDriftHWQuantAdjustment::convertToString(void) const
{
	AQLString ret;
	ret = mSDEAttrNameIR_D;
	ret += ":";
	ret = mSDEAttrNameIR_F;
	ret += ":";
	ret += mSDEAttrNameFX;
	if (mpDriftIR != 0)
	{
		ret += ":";
		ret += mpDriftIR->convertToString();
	}
	ret += ":";
	ret += AQLDataDouble(mFxCriteria).convertToString();
	return ret;

}

/*!
    @brief transform from string representaion
    @param[in] string representaion  (domestic ir sde attr name : foreign ir sde attr name : fx sde attr name : mpDriftIR.convertToString() : fxcriteria)
*/
void
AQLPriceDriftHWQuantAdjustment::convertFromString(const AQLString& str)
{
	AQLDataStrings tmp;
	tmp.convertFromString(str);
	if (tmp.getSize() < 4)
	{
		//error
		throw AQLCoreInvalidData("Format is something wrong", __FILE__, __LINE__);
	}

	mSDEAttrNameIR_D = tmp.get()[0];
	mSDEAttrNameIR_F = tmp.get()[1];
	mSDEAttrNameFX = tmp.get()[2];
	
	mFxCriteria = tmp.get().back().getDoubleValue();

	if (tmp.getSize() == 4 || mpDriftIR == 0)
	{
		return;
	}
	
	AQLString str2 = tmp.get()[3];
	for (unsigned int i = 4; i < tmp.getSize() - 1; i++)
	{
		str2 += ":";
		str2 += tmp.get()[i];
	}
	mpDriftIR->convertFromString(str2);
	
}


/*!
	@brief set up this class
	@param[in] path path object 
*/
void
AQLPriceDriftHWQuantAdjustment::setUp(AQLMathPathEntity& path)
{
	//domestic numeraire
	AQLDataHolder* dh = &path.getData(mSDEAttrNameIR_D, ISNOTNULL);
	AQLMathAttrSDE* pattrsde = &dynamic_cast<AQLMathAttrSDE&>(dh->get());
	mpNumeraireD = pattrsde->getSDE().getNumeraire();

	//fx sde and vol
	dh = &path.getData(mSDEAttrNameFX, ISNOTNULL);
	pattrsde = &dynamic_cast<AQLMathAttrSDE&>(dh->get());
	mpSDEFX = dynamic_cast<AQLRatesSpotSDE*>(&pattrsde->getSDE());
	const AQLFunctionBase* pvol = mpSDEFX->getVolatility()[0][0];
	if (pvol->isTypeOf(FN_VOLFUNCBASE))
		pvol = dynamic_cast<const AQLMathVolFuncBase*>(pvol)->getVolatility();
	if (pvol->isTypeOf(FN_VOLFUNCFXDD))
		mpFxVolatility = dynamic_cast<const AQLMathVolFuncFXDD*>(pvol);
	else
	{
		//error
		throw AQLCoreInvalidData("fx volatility function is not AQLMathVolFuncFXDD", __FILE__, __LINE__);
	}

	//drift ir setup
	mpDriftIR->setUp(path);

	//correlation 
	dh = &path.getData(mSDEAttrNameIR_F, ISNOTNULL);
	pattrsde = &dynamic_cast<AQLMathAttrSDE&>(dh->get());
	AQLRatesBM* bm1 = pattrsde->getSDE().getBM();
	AQLRatesBM* bm2 = mpSDEFX->getBM();
	mCorrelation = bm1->calcCorrelation(*bm2, 0, 0);
	
	mPos_old = 0;
	
	setUp();

}

/*!
	@brief set up cache
*/
void
AQLPriceDriftHWQuantAdjustment::setUp() const
{
	const DoubleArray& timegrid = mpSDEFX->getBM()->getTimeGrid();
	unsigned int size = timegrid.size() - 1;
	mCache1.resize(size);
	mCache2.resize(size);
	mCache3.resize(size);

	//ir parameter
	const AQLMathHWFuncMR&	hwMR = mpDriftIR->getHWMR();
	const AQLMathHWFuncSigma& hwSigma = mpDriftIR->getHWSigma();

	//fx parameter
	const DoubleArray& time = mpFxVolatility->getTimeGrid();
	const DoubleArray& sigma = mpFxVolatility->getSigma();
	const DoubleArray& beta = mpFxVolatility->getBeta();
	const DoubleArray& fwd0 = mpFxVolatility->getForwardFX0();
	unsigned int tsize = time.size();
	DoubleArray v(tsize);
	DoubleArray s(tsize);
	for (unsigned int i = 0; i < tsize; i++)
	{
		v[i] = beta[i] * sigma[i];
		//s[i] = (1.0 - beta[i]) / beta[i] * fwd0[0];
		s[i] = (1.0 - beta[i]) / beta[i] * fwd0[i];
	}
	AQL1DDataSet _s, _v;
	_s.set(time, s);
	_v.set(time, v);
	AQLStepInterpolation inter;
	_s.setInterpolation(inter);
	_v.setInterpolation(inter);
	
	//
	AQLHWQAdjInnerFunc1 func1(hwMR, hwSigma, _v);
	AQLHWQAdjInnerFunc2 func2(hwMR, hwSigma, _v, _s);
	AQLHWQAdjInnerFunc3 func3(hwMR, hwSigma, _v, _s);

	AQLGaussLegendre gl(GAUSSLEGENDREPOINTNUM);
	for (unsigned int i = 0; i < size; i++)
	{
		mCache1[i] = -mCorrelation[i] * func1.integral(timegrid[i], timegrid[i + 1], &gl);
		mCache2[i] = -mCorrelation[i] * func2.integral(timegrid[i], timegrid[i + 1], &gl);
		mCache3[i] = -mCorrelation[i] * func3.integral(timegrid[i], timegrid[i + 1], &gl) - mCache2[i] * timegrid[i];
	}

	//set up mMinFX
	mMinFx.resize(size);
	for (unsigned int i = 0; i < size; i++)
		//mMinFx[i] = mFxCriteria*fwd0[i];
		mMinFx[i] = mFxCriteria * mpFxVolatility->getForwardFX0(timegrid[i]);
	
	mIsSetUped = true;
}

