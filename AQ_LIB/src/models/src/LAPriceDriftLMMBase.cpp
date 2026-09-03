/*! @file
    @brief Source code of abstruct base class of drift function of LMM sde



*/
//  2007, AlgoQuantHub..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAPriceDriftLMMBase.cpp
//
//  SYNOPSIS    :       LAPriceDriftLMMBase
//  DESCRIPTION :       Source code of abstruct base class of drift function of LMM sde (spot measure)
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


#include "LAPriceDriftLMMBase.h"

#include "LAMathPathEntity.h"
#include "LADataHolder.h"
#include "LADataVector.h"
#include "LADataReference.h"
#include "LAObjectHolder.h"
#include "LAMathAttrSDE.h"
#include "LARatesSDEBase.h"
#include "LAModelDynamicsCurve.h"
#include "LAMathCorrelation.h"
#include "LAMathVolFuncBase.h"
#include "LAAlgorithm.h"

#include "LAPriceDriftQuantAdjustment.h"
#include "LABasic.h"

using namespace std;
#define COR "COR"

//================ LAPriceDriftLMMBase ===================================
/*!
	@brief default constructor
	@param[in] Q val for displaced diffusion case
*/
LAPriceDriftLMMBase::LAPriceDriftLMMBase(double Q)
: LAMathDriftFuncBase(), m_i(0), mpTimes(0), mPos_old(0), mpCache(0), mpCache2(0), mpCache_before(0), mpCacheTime(0), mpCacheTime_before(0)
, mpCovariance(0), mSpread(0.0), mIsCovSetuped(false), mLogQ(-LAMath::log(Q)/LAMath::log(2.0)), mL_i(0.0)
{

}
/*!
	@brief default constructor
	@param[in] sdeAttrName data name of sde
	@param[in] i suffix
	@param[in] tenor tenor
	@param[in] deltatenor delta of tenor
	@param[in] Q val for displaced diffusion case

*/
LAPriceDriftLMMBase::LAPriceDriftLMMBase(const LAString& sdeAttrName, unsigned int i, const DoubleArray& tenor, const DoubleArray& delta_tenor, double Q)
: LAMathDriftFuncBase(), mTenor(tenor), mDeltaTenor(delta_tenor), m_i(i), mSDEAttrName(sdeAttrName),
mpTimes(0), mPos_old(0), mpCache(0), mpCache2(0), mpCache_before(0), mpCacheTime(0), mpCacheTime_before(0), mpCovariance(0)
, mSpread(0.0), mIsCovSetuped(false), mLogQ(-LAMath::log(Q)/LAMath::log(2.0)), mL_i(0.0)
{
	if (tenor.size() != delta_tenor.size() + 1)
	{
		//error
		throw LACoreInvalidData("tenor size must be delta_tenor size plus one", __FILE__, __LINE__);
	}
	if (mTenor.at(0) == 0.0)
	{	
		mTenor.erase(mTenor.begin());
		mDeltaTenor.erase(mDeltaTenor.begin());
	}
}
/*!
	@brief copy constructor
	@param[in] v copy source
*/
LAPriceDriftLMMBase::LAPriceDriftLMMBase(const LAPriceDriftLMMBase& v) 
: LAMathDriftFuncBase(v), mTenor(v.mTenor), mDeltaTenor(v.mDeltaTenor), m_i(v.m_i), mVolatility(v.mVolatility),
mSDEAttrName(v.mSDEAttrName), mpTimes(v.mpTimes), mCorData(v.mCorData), mPos_old(v.mPos_old)
, mpCache(0), mpCache2(0), mpCache_before(0), mpCacheTime(0), mpCacheTime_before(0), mpCovariance(0)
, mSpread(v.mSpread), mIsCovSetuped(v.mIsCovSetuped),mLogQ(v.mLogQ),mL_i(v.mL_i)
{
	mCorrelation.resize(v.mCorrelation.size());
	for (unsigned int i = 0; i < mCorrelation.size(); i++)
	{
		mCorrelation[i].resize(v.mCorrelation[i].size());
		for (unsigned int j = 0; j < mCorrelation[i].size(); j++)
		{
			if (m_i == 0) mCorrelation[i][j] = dynamic_cast<LAFunctionBase*>(v.mCorrelation[i][j]->clone());
			else mCorrelation[i][j] = dynamic_cast<LAFunctionBase*>(v.mCorrelation[i][j]);
		}
	}
	if (m_i == 0 && v.mpCache != 0)
		mpCache = new DoubleArray(*v.mpCache);
	if (m_i == 0 && v.mpCache2 != 0)
		mpCache2 = new DoubleArray(*v.mpCache2);
	if (m_i == 0 && v.mpCache_before != 0)
		mpCache_before = new DoubleArray(*v.mpCache_before);
	if (m_i == 0 && v.mpCacheTime != 0)
		mpCacheTime = new double(*v.mpCacheTime);
	if (m_i == 0 && v.mpCacheTime_before != 0)
		mpCacheTime_before = new double(*v.mpCacheTime_before);
	if (m_i == 0 && v.mpCovariance != 0)
		mpCovariance = new LARatesCovariance(*v.mpCovariance);
}

/*!
	@brief destructor
*/
LAPriceDriftLMMBase::~LAPriceDriftLMMBase() 
{
	if (m_i == 0)
	{
		for (unsigned int i = 0; i < mCorrelation.size(); i++)
			for (unsigned int j = 0; j < mCorrelation[i].size(); j++)
				delete mCorrelation[i][j];
		if (mpCache != 0) delete mpCache;
		if (mpCache2 != 0) delete mpCache2;
		if (mpCache_before != 0) delete mpCache_before;
		if (mpCacheTime != 0) delete mpCacheTime;
		if (mpCacheTime_before != 0) delete mpCacheTime_before;
		if (mpCovariance != 0) delete mpCovariance;
	}
}
/*!
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LAPriceDriftLMMBase::isTypeOf(function_t id) const
{
	return (id==FN_DRIFTLMMBASE ? true : LAMathDriftFuncBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LAPriceDriftLMMBase::getType() const
{
	return FN_DRIFTLMMBASE;
}

/*!
    @brief return string representaion
    @return string representaion (sde attr name : suffix : tenor : deltatenor)
*/
LAString
LAPriceDriftLMMBase::convertToString(void) const
{
	LAString ret;
	ret += mSDEAttrName;
	ret += ":";
	ret += LADataInt(m_i).convertToString();
	ret += ":";
	ret += LADataDoubles(mTenor).convertToString();
	ret += ":";
	ret += LADataDoubles(mDeltaTenor).convertToString();

	return ret;
}

/*!
    @brief transform from string representaion
    @param[in] string representaion  (sde attr name : suffix : tenor : deltatenor)
*/
void
LAPriceDriftLMMBase::convertFromString(const LAString& str)
{
	LADataStrings tmp;
	tmp.convertFromString(str);
	if (tmp.getSize() < 5 || tmp.getSize() % 2 == 0)
	{
		//error
		throw LACoreInvalidData("Format is something wrong", __FILE__, __LINE__);
	}

	mSDEAttrName = tmp.get()[0];
	m_i = tmp.get()[1].getIntValue();
	unsigned int size = tmp.getSize() - 2;
	mTenor.resize(size / 2 + 1);
	mDeltaTenor.resize(size / 2);
	for (unsigned int i = 0; i < mTenor.size(); i++)
		mTenor[i] = tmp.get()[2 + i].getDoubleValue();
	size = 2 + mTenor.size();
	for (unsigned int i = 0; i < mDeltaTenor.size(); i++)
		mDeltaTenor[i] = tmp.get()[size + i].getDoubleValue();

	if (mTenor.at(0) == 0.0)
	{	
		mTenor.erase(mTenor.begin());
		mDeltaTenor.erase(mDeltaTenor.begin());
	}
}

/*!
	@brief set tenor
	@param[in] tenor tenor 
	@param[in] delta_tenor span of each libor

	@note day count of tenor must be same as one of path object 
	@note day count of delta_tenor must be same as one of libor
*/
void
LAPriceDriftLMMBase::setTenor(const DoubleArray& tenor, const DoubleArray& delta_tenor)
{
	if (tenor.size() != delta_tenor.size() + 1)
	{
		//error
		throw LACoreInvalidData("tenor size must be delta_tenor size plus one", __FILE__, __LINE__);
	}
	mTenor = tenor;
	mDeltaTenor = delta_tenor;
	if (mTenor.at(0) == 0.0)
	{	
		mTenor.erase(mTenor.begin());
		mDeltaTenor.erase(mDeltaTenor.begin());
	}
}


/*!
	@brief set up this class
	@param[in] path path object 
*/
void
LAPriceDriftLMMBase::setUp(LAMathPathEntity& path)
{
	LADataHolder* dh = &path.getData(mSDEAttrName, ISNOTNULL);
	LAMathAttrSDE* pattrsde = &dynamic_cast<LAMathAttrSDE&>(dh->get());
	mVolatility = pattrsde->getSDE().getVolatility();
	const LARatesPathElementCurve* pcurve = dynamic_cast<const LARatesPathElementCurve*>(pattrsde->getSDE().getInitialValue());
	mL_i = (pcurve->getP(mTenor[m_i])/pcurve->getP(mTenor[m_i+1])-1.0)/(mDeltaTenor[m_i]);
	mSpread = mL_i * mLogQ;
	mpTimes = 0;
	mpTimes = &pattrsde->getSDE().getBM()->getTimeGrid();	
	mX.resize(1);
	
	if (m_i == 0)
	{
		LAString name = COR;
		name += "_";
		name += mSDEAttrName;
		dh = &path.getData(name, ISNOTNULL);
		LADataReference* ref = &dynamic_cast<LADataReference&>(dh->get());
		LAMathCorrelation* cor = &dynamic_cast<LAMathCorrelation&>(ref->get().get());

		for (unsigned int i = 0; i < mCorrelation.size(); i++)
			for (unsigned int j = 0; j < mCorrelation[i].size(); j++)
				delete mCorrelation[i][j];
		mCorrelation = cor->getCorrelationFunc();

		if (mpCache != 0) {delete mpCache; mpCache = 0;}
		if (mpCache2 != 0) {delete mpCache2; mpCache2 = 0;}
		if (mpCache_before != 0) {delete mpCache_before; mpCache_before = 0;} 
		if (mpCacheTime != 0) {delete mpCacheTime; mpCacheTime = 0;}
		if (mpCacheTime_before != 0) {delete mpCacheTime_before; mpCacheTime_before = 0;} 
		if (mpCovariance != 0) {delete mpCovariance; mpCovariance = 0;}
		
		mpCache = new DoubleArray(mDeltaTenor.size());
		mpCache2 = new DoubleArray(mDeltaTenor.size());
		mpCache_before = new DoubleArray(mDeltaTenor.size());
		mpCacheTime = new double(-100000000.0);
		mpCacheTime_before = new double(-100000000.0);
		mpCovariance = new LARatesCovariance();
		mIsCovSetuped = false;
	}	
	else
	{
		LAFunctionBase* method = pattrsde->getSDE().getDrift()[0];
		if (method->isTypeOf(FN_DRIFTQUANTADJ))
			method = dynamic_cast<LAPriceDriftQuantAdjustment*>(method)->getDrift();
	
		if (!method->isTypeOf(getType()))
		{
			//error
			throw LACoreInvalidData("dirft function is not LAPriceDriftLMMBase", __FILE__, __LINE__);
		}
		LAPriceDriftLMMBase* drift = dynamic_cast<LAPriceDriftLMMBase*>(method);
		mCorrelation = drift->mCorrelation;
		mpCache = drift->mpCache;
		mpCache2 = drift->mpCache2;
		mpCache_before = drift->mpCache_before;
		mpCacheTime = drift->mpCacheTime;
		mpCacheTime_before = drift->mpCacheTime_before;
		mpCovariance = drift->mpCovariance;

	}

	mPos_old = 0;

	
	//set up covariance
//	if (m_i == 0) setUpCovariance();
}
/*!
    @brief set up covariance
	@note this class is called from setUp(LAMathPathEntity& path)
*/
void
LAPriceDriftLMMBase::setUpCovariance() const
{
	if (m_i != 0) return;
	mIsCovSetuped = true;
	unsigned int size = mVolatility.size();
	vector<const LAFunctionBase*> vol(size);
	vector<vector<const LAFunctionBase*> > cor(size);
	for (unsigned int i = 0; i < size; i++)
	{
		if (mVolatility[i][0]->isTypeOf(FN_VOLFUNCBASE))
			vol[i] = dynamic_cast<LAMathVolFuncBase*>(mVolatility[i][0])->getVolatility();
		else
			vol[i] = mVolatility[i][0];
		cor[i].resize(size);
		for (unsigned int j = 0; j < size; j++)
			cor[i][j] = mCorrelation[i][j];
	}
	
	mpCovariance->setVolaility(vol);
	mpCovariance->setCorrelation(cor);
	if (mpTimes == 0) return;
    DoubleArray resettimegrid = mTenor;
	resettimegrid.pop_back();
	mpCovariance->calcIntegratedCov((*mpTimes), &resettimegrid);
}

