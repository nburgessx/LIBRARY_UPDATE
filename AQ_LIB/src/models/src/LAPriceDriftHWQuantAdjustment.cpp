/*! @file
    @brief Source code of drift function of quant adjustment of HW model



*/
//  2008, Mizuho International London.. 

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAPriceDriftHWQuantAdjustment.cpp
//
//  SYNOPSIS    :       LAPriceDriftHWQuantAdjustment
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


#include "LAPriceDriftHWQuantAdjustment.h"
#include "LAPriceFXVolatility.h"
#include "LAMathPathEntity.h"
#include "LADataHolder.h"
#include "LADataVector.h"
#include "LADataReference.h"
#include "LAObjectHolder.h"
#include "LAMathAttrSDE.h"
#include "LAPriceDataFunction.h"
#include "LAMathVolFuncBase.h"
#include "LARatesSpotSDE.h"
#include "LAModelDynamicsCurve.h"
#include "LAModelDynamicsScalar.h"
#include "LAAlgorithm.h"
#include "LAConstant.h"
#include "LA1DDataSet.h"
#include "LASplineInterpolation.h"
#include "LAStepInterpolation.h"
#include "LAGaussLegendre.h"
#include "LACombinationFunc.h"
#include "LABasic.h"
#include "LAPriceDriftHW.h"
#include "LAMathVolFuncFXDD.h"

using namespace std;

#define GAUSSLEGENDREPOINTNUM 20


//================ LAPriceDriftHWQuantAdjustment ===================================
/*!
	@brief default constructor
	@param[in] pDriftIR drift class before quant adjustment
*/
LAPriceDriftHWQuantAdjustment::LAPriceDriftHWQuantAdjustment(const LARatesNumeraireBase& domestic_nu, const LAMathVolFuncFXDD& fxvol, LARatesSpotSDE& fx_sde, 
									LAPriceDriftHW* pDriftIR, const DoubleArray& cor,
									double fx_criteria)
: mpNumeraireD(&domestic_nu), mpFxVolatility(&fxvol), mpSDEFX(&fx_sde), 
mpDriftIR(pDriftIR), mCorrelation(cor), mIsSetUped(false), mPos_old(0), mFxCriteria(fx_criteria)
{
	if (pDriftIR == 0)
	{
		//error
		throw LACoreInvalidData("input IR drift is NULL", __FILE__, __LINE__);
	}
}



/*!
	@brief default constructor
	@param[in] sdeAttrNameIR_D data name of domestic ir model
	@param[in] sdeAttrNameIR_F data name of foreign ir model
	@param[in] sdeAttrNameFX data name of fx model
	@param[in] pDriftIR drift class before quant adjustment
*/
LAPriceDriftHWQuantAdjustment::LAPriceDriftHWQuantAdjustment(const LAString& sdeAttrNameIR_D, 
													   const LAString& sdeAttrNameIR_F, 
													   const LAString& sdeAttrNameFX, 
													   LAPriceDriftHW* pDriftIR,
													   double fx_criteria)
: mpNumeraireD(0), mpFxVolatility(0), mpSDEFX(0),
mSDEAttrNameIR_D(sdeAttrNameIR_D), mSDEAttrNameIR_F(sdeAttrNameIR_F), mSDEAttrNameFX(sdeAttrNameFX), mpDriftIR(pDriftIR)
, mIsSetUped(false), mPos_old(0), mFxCriteria(fx_criteria)
{
	if (pDriftIR == 0)
	{
		//error
		throw LACoreInvalidData("input IR drift is NULL", __FILE__, __LINE__);
	}
}


/*!
	@brief copy constructor
	@param[in] v copy source
*/
LAPriceDriftHWQuantAdjustment::LAPriceDriftHWQuantAdjustment(const LAPriceDriftHWQuantAdjustment& v) 
: LAMathDriftFuncBase(v), 
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
		mpDriftIR = dynamic_cast<LAPriceDriftHW*>(v.mpDriftIR->clone());
}

/*!
	@brief destructor
*/
LAPriceDriftHWQuantAdjustment::~LAPriceDriftHWQuantAdjustment() 
{
	if (mpDriftIR != 0) delete mpDriftIR;
}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LAPriceDriftHWQuantAdjustment::clone() const	
{
    try 
	{
		return new LAPriceDriftHWQuantAdjustment(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}
/*!
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LAPriceDriftHWQuantAdjustment::isTypeOf(function_t id) const
{
	return (id==FN_DRIFTHWQUANTADJ ? true : LAMathDriftFuncBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LAPriceDriftHWQuantAdjustment::getType() const
{
	return FN_DRIFTHWQUANTADJ;
}


/*!
    @brief Return drift value
	@param[in] x input value, x[0] = ts, x[1] = te, x[2] = rf
    @return drift value

*/
double
LAPriceDriftHWQuantAdjustment::operator()(const DoubleArray& x) const
{
	if (!mIsSetUped) setUp();
	
	const DoubleArray& timegrid = mpSDEFX->getBM()->getTimeGrid();
	unsigned int pos;
	if (x[0] == 0.0) pos = 0;
	else if (x[0] == timegrid[mPos_old]) pos = mPos_old;
	else if (mPos_old + 2 < timegrid.size() && x[0] == timegrid[mPos_old + 1]) pos = mPos_old + 1;
	else if (!LAAlgorithm::find<DoubleArray, double>(timegrid, x[0], 0, timegrid.size() - 1, pos))
	{
		//error
		LAString msg = "Time =" + LADataDouble(x[0]).convertToString();
		msg += " is not in sde integral time grid";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
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
LAString
LAPriceDriftHWQuantAdjustment::convertToString(void) const
{
	LAString ret;
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
	ret += LADataDouble(mFxCriteria).convertToString();
	return ret;

}

/*!
    @brief transform from string representaion
    @param[in] string representaion  (domestic ir sde attr name : foreign ir sde attr name : fx sde attr name : mpDriftIR.convertToString() : fxcriteria)
*/
void
LAPriceDriftHWQuantAdjustment::convertFromString(const LAString& str)
{
	LADataStrings tmp;
	tmp.convertFromString(str);
	if (tmp.getSize() < 4)
	{
		//error
		throw LACoreInvalidData("Format is something wrong", __FILE__, __LINE__);
	}

	mSDEAttrNameIR_D = tmp.get()[0];
	mSDEAttrNameIR_F = tmp.get()[1];
	mSDEAttrNameFX = tmp.get()[2];
	
	mFxCriteria = tmp.get().back().getDoubleValue();

	if (tmp.getSize() == 4 || mpDriftIR == 0)
	{
		return;
	}
	
	LAString str2 = tmp.get()[3];
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
LAPriceDriftHWQuantAdjustment::setUp(LAMathPathEntity& path)
{
	//domestic numeraire
	LADataHolder* dh = &path.getData(mSDEAttrNameIR_D, ISNOTNULL);
	LAMathAttrSDE* pattrsde = &dynamic_cast<LAMathAttrSDE&>(dh->get());
	mpNumeraireD = pattrsde->getSDE().getNumeraire();

	//fx sde and vol
	dh = &path.getData(mSDEAttrNameFX, ISNOTNULL);
	pattrsde = &dynamic_cast<LAMathAttrSDE&>(dh->get());
	mpSDEFX = dynamic_cast<LARatesSpotSDE*>(&pattrsde->getSDE());
	const LAFunctionBase* pvol = mpSDEFX->getVolatility()[0][0];
	if (pvol->isTypeOf(FN_VOLFUNCBASE))
		pvol = dynamic_cast<const LAMathVolFuncBase*>(pvol)->getVolatility();
	if (pvol->isTypeOf(FN_VOLFUNCFXDD))
		mpFxVolatility = dynamic_cast<const LAMathVolFuncFXDD*>(pvol);
	else
	{
		//error
		throw LACoreInvalidData("fx volatility function is not LAMathVolFuncFXDD", __FILE__, __LINE__);
	}

	//drift ir setup
	mpDriftIR->setUp(path);

	//correlation 
	dh = &path.getData(mSDEAttrNameIR_F, ISNOTNULL);
	pattrsde = &dynamic_cast<LAMathAttrSDE&>(dh->get());
	LARatesBM* bm1 = pattrsde->getSDE().getBM();
	LARatesBM* bm2 = mpSDEFX->getBM();
	mCorrelation = bm1->calcCorrelation(*bm2, 0, 0);
	
	mPos_old = 0;
	
	setUp();

}

/*!
	@brief set up cache
*/
void
LAPriceDriftHWQuantAdjustment::setUp() const
{
	const DoubleArray& timegrid = mpSDEFX->getBM()->getTimeGrid();
	unsigned int size = timegrid.size() - 1;
	mCache1.resize(size);
	mCache2.resize(size);
	mCache3.resize(size);

	//ir parameter
	const LAMathHWFuncMR&	hwMR = mpDriftIR->getHWMR();
	const LAMathHWFuncSigma& hwSigma = mpDriftIR->getHWSigma();

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
	LA1DDataSet _s, _v;
	_s.set(time, s);
	_v.set(time, v);
	LAStepInterpolation inter;
	_s.setInterpolation(inter);
	_v.setInterpolation(inter);
	
	//
	MMHWQAdjInnerFunc1 func1(hwMR, hwSigma, _v);
	MMHWQAdjInnerFunc2 func2(hwMR, hwSigma, _v, _s);
	MMHWQAdjInnerFunc3 func3(hwMR, hwSigma, _v, _s);

	LAGaussLegendre gl(GAUSSLEGENDREPOINTNUM);
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

