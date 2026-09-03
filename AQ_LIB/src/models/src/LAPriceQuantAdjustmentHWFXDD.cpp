/*! @file
    @brief Source code of base class of quanto adjustment function class



*/
//  2012, AlgoQuantHub.

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAPriceQuantAdjustmentHWFXDD.cpp
//
//  SYNOPSIS    :       LAPriceQuantAdjustmentHWFXDD
//  DESCRIPTION :       Source of base class of quanto adjustment class
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


#include "LAPriceQuantAdjustmentHWFXDD.h"
#include "LAMathVolFuncBase.h"
#include "LAPriceDriftHWQuantAdjustment.h"
#include "LAMathVolFuncFXDD.h"
#include "LAPriceDriftHW.h"
#include "LAMathPathEntity.h"
#include "AQLDataHolder.h"
#include "AQLDataVector.h"
#include "LARatesSpotSDE.h"



using namespace std;

//================ LAPriceQuantAdjustmentHWFXDD ===================================
/*!
	@brief default constructor
*/
LAPriceQuantAdjustmentHWFXDD::LAPriceQuantAdjustmentHWFXDD(const AQLString& sdeAttrNameIR_F, const AQLString& sdeAttrNameFX, const bool isAdjustInPath, const double fxCriteria, const double quantoCriteria )
: LAPriceQuantAdjustmentFuncBase(), mSDEAttrNameIR_F(sdeAttrNameIR_F), mSDEAttrNameFX(sdeAttrNameFX), mIsAdjustInPath(isAdjustInPath), mFXCriteria(fxCriteria), mQuantoCriteria(quantoCriteria),
mpDriftIR(0), mpNumeraireF(0), mpSDEFX(0), mpFxVolatility(0), mPos_old(0), mIsSetUped(false), mIRCache1(0.0), mIRCache2(0.0)
{
}

/*!
	@brief copy constructor
	@param[in] v copy source
*/
LAPriceQuantAdjustmentHWFXDD::LAPriceQuantAdjustmentHWFXDD(const LAPriceQuantAdjustmentHWFXDD& v) 
: LAPriceQuantAdjustmentFuncBase(v), 
mSDEAttrNameIR_F(v.mSDEAttrNameIR_F),
mSDEAttrNameFX(v.mSDEAttrNameFX),
mIsAdjustInPath(v.mIsAdjustInPath),
mFXCriteria(v.mFXCriteria),
mQuantoCriteria(v.mQuantoCriteria),
mpDriftIR(v.mpDriftIR),
mpNumeraireF(v.mpNumeraireF),
mpSDEFX(v.mpSDEFX),
mpFxVolatility(v.mpFxVolatility),
mIsSetUped(false),
mIRCache1(v.mIRCache1),
mIRCache2(v.mIRCache2)
{

}

/*!
	@brief copy constructor
*/
/*LAPriceQuantAdjustmentHWFXDD::LAPriceQuantAdjustmentHWFXDD(const LAPriceQuantAdjustmentHWFXDD& v) 
: AQLCoreFunctionBase(v)
{

}*/

/*!
	@brief destructor
*/
LAPriceQuantAdjustmentHWFXDD::~LAPriceQuantAdjustmentHWFXDD() 
{

}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
LAPriceQuantAdjustmentHWFXDD::clone() const
{
    try 
	{
		return new LAPriceQuantAdjustmentHWFXDD(*this);
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
LAPriceQuantAdjustmentHWFXDD::isTypeOf(function_t id) const
{
	return (id==FN_QUANTADJUSTMENTFXDD ? true : LAPriceQuantAdjustmentFuncBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LAPriceQuantAdjustmentHWFXDD::getType() const
{
	return FN_QUANTADJUSTMENTFXDD;
}

/*!
    @brief set up
	@param[in] path path object
*/
void
LAPriceQuantAdjustmentHWFXDD::setUp(LAMathPathEntity& path)
{
	//foreign numeraire
	AQLDataHolder *dh = &path.getData(mSDEAttrNameIR_F, ISNOTNULL);
	LAMathAttrSDE *pAttrsde = &dynamic_cast<LAMathAttrSDE &>(dh->get());
	mpNumeraireF = pAttrsde->getSDE().getNumeraire();
	const vector<AQLFunctionBase *> &driftVec = pAttrsde->getSDE().getDrift();
	if (driftVec.empty() || driftVec[0]->isTypeOf(FN_DRIFTHWQUANTADJ))
	{
		throw AQLCoreInvalidData("Foreign ir drift class must be LAPriceDriftHWQuantAdjustment", __FILE__, __LINE__);
	}
	mpDriftIR = dynamic_cast<LAPriceDriftHWQuantAdjustment *>(driftVec[0])->getDrift();
	//drift ir setup
	mpDriftIR->setUp(path);

	//fx sde and vol
	dh = &path.getData(mSDEAttrNameFX, ISNOTNULL);
	pAttrsde = &dynamic_cast<LAMathAttrSDE &>(dh->get());
	mpSDEFX = dynamic_cast<LARatesSpotSDE *>(&pAttrsde->getSDE());
	const AQLFunctionBase* pVol = mpSDEFX->getVolatility()[0][0];
	if (pVol->isTypeOf(FN_VOLFUNCBASE))
	{
		pVol = dynamic_cast<const LAMathVolFuncBase*>(pVol)->getVolatility();
	}
	if (pVol->isTypeOf(FN_VOLFUNCFXDD))
	{
		mpFxVolatility = dynamic_cast<const LAMathVolFuncFXDD*>(pVol);
	}
	else
	{
		//error
		throw AQLCoreInvalidData("fx volatility function is not LAMathVolFuncFXDD", __FILE__, __LINE__);
	}

	//correlation 
	dh = &path.getData(mSDEAttrNameIR_F, ISNOTNULL);
	pAttrsde = &dynamic_cast<LAMathAttrSDE &>(dh->get());
	LARatesBM *bm1 = pAttrsde->getSDE().getBM();
	LARatesBM *bm2 = mpSDEFX->getBM();
	mCorrelation = bm1->calcCorrelation(*bm2, 0, 0);
	
	mPos_old = 0;
	
	setUp();
}


/*!
	@brief set up cache
*/
void
LAPriceQuantAdjustmentHWFXDD::setUp() const
{
	const DoubleArray& timegrid = mpSDEFX->getBM()->getTimeGrid();
	unsigned int size = timegrid.size();
	mECache.resize(size);
	mBCache.resize(size);
	mIRVolCorCache.resize(size);
	mIRVolIntCorCache.resize(size);
	mFXCache1.resize(size);
	mFXCache2.resize(size);
	mX1.resize(size);
	mX2.resize(size);
	mIRQuantAdjustment.resize(size);

	mIRCache1 = 0.0;
	mIRCache2 = 0.0;

	//ir parameter
	const LAMathHWFuncMR&	hwMR = mpDriftIR->getHWMR();
	const LAMathHWFuncSigma& hwSigma = mpDriftIR->getHWSigma();


	for (unsigned int i = 0; i < size - 1; ++i)
	{
		const DoubleArray tmp(1, timegrid[i]);
		const double a = hwMR(tmp);
		const double s = hwSigma(tmp);
		const double tau = timegrid[i + 1] - timegrid[i];
		const double exp = AQLMath::exp(-a * tau);
		const double b = (a != 0.0) ? (1.0 - exp) / a : tau;
		mECache[i] = exp;
		mBCache[i] = b;
		mIRVolCorCache[i] = s * b * mCorrelation[i];
		const double irVolInt = (a != 0.0) ? s * (tau + (exp - 1.0) / a ) / a : 0.5 * s * tau * tau;
		mIRVolIntCorCache[i] = irVolInt * mCorrelation[i];
		const double sigma = mpFxVolatility->getSigma(timegrid[i]);
		const double beta = mpFxVolatility->getBeta(timegrid[i]);
		const double alpha = mpFxVolatility->getAlpha()(timegrid[i]);
		const double fx_s = mpFxVolatility->getForwardFX0(timegrid[i]);
		const double fx_e = mpFxVolatility->getForwardFX0(timegrid[i + 1]);
		const double fxMulti = 0.5 * (1.0 +  fx_e / fx_s);
		const double multi1 = sigma * beta;
		const double multi2 = sigma * alpha * fxMulti;
		mX1[i] = multi1;
		mX2[i] = multi2;
		mFXCache1[i] = mIRVolIntCorCache[i] * multi1;
		mFXCache2[i] = mIRVolIntCorCache[i] * multi2;

	}
	//set up mMinFX
	mMinFX.resize(size);
	for (unsigned int i = 0; i < size; i++)
	{
		mMinFX[i] = mFXCriteria * mpFxVolatility->getForwardFX0(timegrid[i]);
	}

	mIsSetUped = true;
}
/*!
    @brief get quanto adjust
	@param[in] t
*/
double
LAPriceQuantAdjustmentHWFXDD::getQuantAdjust(const double ts) const
{
	unsigned int pos;
	searchTimeGridPos(ts, pos);
	return mIRQuantAdjustment[pos];

}

/*!
    @brief calc quanto adjust
	@param[in] ts
	@param[in] te
	@param[in] r
*/
void
LAPriceQuantAdjustmentHWFXDD::calcQuantAdjust(const double ts, const double te, const double r) const
{
	if (!mIsSetUped) setUp();

	if (ts == 0.0)
	{
		mIRCache1 = 0.0;
		mIRCache2 = 0.0;
	}

	unsigned int pos_s;
	searchTimeGridPos(ts, pos_s);

	unsigned int pos_e;
	searchTimeGridPos(te, pos_e);
	
	//set fxnumerarie
	LARatesPathElementHW1FCurveTMDPT pathCurve(te);
	pathCurve.LARatesPathElementSRCurve::set(r);
	mpNumeraireF->setCurve(te, &pathCurve);

	//fx
	double fx_s = mpSDEFX->getPathElement(pos_s)->get()[0];
	double fx_e = mpSDEFX->getPathElement(pos_e)->get()[0];

	double fx_s_= fx_s;
	if (fx_s >= 0 && fx_s < mMinFX[pos_s]) fx_s_ = mMinFX[pos_s];
	else if (fx_s < 0 && fx_s >= -mMinFX[pos_s]) fx_s_= -mMinFX[pos_s];

	double fx_e_= fx_e;
	if (fx_e >= 0 && fx_e < mMinFX[pos_e]) fx_e_ = mMinFX[pos_e];
	else if (fx_e < 0 && fx_e >= -mMinFX[pos_e]) fx_e_= -mMinFX[pos_e];

	const double tmp = 0.5 * (1.0 / fx_s_ + 1.0 / fx_e_);
	if (mIsAdjustInPath)
	{
		mIRCache1 = 0.0;
		mIRCache2 = 0.0;
	}
	else
	{
		// Accumulating part
		mIRCache1 *= mECache[pos_s];
		mIRCache2 *= mECache[pos_s];
	}

	mIRCache1 += mIRVolCorCache[pos_s] * mX1[pos_s];
	mIRCache2 += mIRVolCorCache[pos_s] * mX2[pos_s] * tmp;
	double quantoAdj = mIRCache1 + mIRCache2;
	//double quantoAdj = mIRCache1[pos_s] + mIRCache2[pos_s] * tmp;
    

	if (mQuantoCriteria < quantoAdj / r)
	{
		quantoAdj = mQuantoCriteria * r;
	}
	else if (quantoAdj / r < -mQuantoCriteria)
	{
		quantoAdj = -mQuantoCriteria * r;
	}

	 mIRQuantAdjustment[pos_s] = quantoAdj;
}

/*!
    @brief calc FX quanto adjust
	@param[in] ts
	@param[in] te
*/
double
LAPriceQuantAdjustmentHWFXDD::getFXQuantAdjustNoFX(const double ts, const double te) const
{
	if (!mIsSetUped) setUp();

	unsigned int pos;
	searchTimeGridPos(ts, pos);

	double val1 = mFXCache1[pos];
	if (!mIsAdjustInPath)
	{
		const double tmp1 = mIRCache1 * mBCache[pos];
		const double accumVal = mIRCache2 * mBCache[pos];
		val1 = mFXCache1[pos] + tmp1;
		val1 += accumVal;
	}
	return val1;
	//return mFXCache1[pos];
}

/*!
    @brief calc FX quanto adjust
	@param[in] ts
	@param[in] te
*/
double
LAPriceQuantAdjustmentHWFXDD::getFXQuantAdjustFXMulti(const double ts, const double te) const
{
	if (!mIsSetUped) setUp();

	unsigned int pos;
	searchTimeGridPos(ts, pos);

	return mFXCache2[pos];
}

/*!
	@brief search timegrid pos
	@param[in] te
	@param[out] pos

*/
void
LAPriceQuantAdjustmentHWFXDD::searchTimeGridPos(const double t, unsigned int &pos)  const
{
	const DoubleArray& timegrid = mpSDEFX->getBM()->getTimeGrid();
	if (t == 0.0) pos = 0;
	else if (t == timegrid[mPos_old]) pos = mPos_old;
	else if (mPos_old + 2 < timegrid.size() && t == timegrid[mPos_old + 1]) pos = mPos_old + 1;
	else if (!AQLAlgorithm::find<DoubleArray, double>(timegrid, t, 0, timegrid.size() - 1, pos))
	{
		//error
		AQLString msg = "Time =" + AQLDataDouble(t).convertToString();
		msg += " is not in sde integral time grid";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	mPos_old = pos;
}
