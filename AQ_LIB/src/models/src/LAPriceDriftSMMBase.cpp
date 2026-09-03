/*! @file
    @brief Source code of abstruct base class of drift function of SMM sde



*/
//  2007, AlgoQuantHub..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAPriceDriftSMMBase.cpp
//
//  SYNOPSIS    :       LAPriceDriftSMMBase
//  DESCRIPTION :       Source code of abstruct base class of drift function of SMM sde
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


#include "LAPriceDriftSMMBase.h"

#include "LAMathPathEntity.h"
#include "AQLDataHolder.h"
#include "AQLDataVector.h"
#include "AQLDataReference.h"
#include "AQLObjectHolder.h"
#include "LAMathAttrSDE.h"
#include "LARatesSDEBase.h"
#include "LAModelDynamicsCurve.h"
#include "LAMathCorrelation.h"
#include "LAMathVolFuncBase.h"
#include "AQLAlgorithm.h"

//#include "LAPriceDriftQuantAdjustment.h"

using namespace std;
#define COR "COR"

//================ LAPriceDriftSMMBase ===================================
/*!
	@brief default constructor
	@param[in] s spread added to S (for displaced diffusion case)
*/
LAPriceDriftSMMBase::LAPriceDriftSMMBase(double s)
: LAMathDriftFuncBase(), m_i(0), mpTimes(0), mPos_old(0), mpCache(0), mSpread(s)
{

}
/*!
	@brief default constructor
	@param[in] sdeAttrName data name of sde
	@param[in] i suffix
	@param[in] tenor tenor
	@param[in] deltatenor delta of tenor
	@param[in] s spread added to L (for displaced diffusion case)

*/
LAPriceDriftSMMBase::LAPriceDriftSMMBase(const AQLString& sdeAttrName, unsigned int i, const DoubleArray& tenor, const DoubleArray& delta_tenor, double s)
: LAMathDriftFuncBase(), mTenor(tenor), mDeltaTenor(delta_tenor), m_i(i), mSDEAttrName(sdeAttrName),
mpTimes(0), mPos_old(0), mpCache(0), mSpread(s)
{
	if (tenor.size() != delta_tenor.size() + 1)
	{
		//error
		throw AQLCoreInvalidData("tenor size must be delta_tenor size plus one", __FILE__, __LINE__);
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
LAPriceDriftSMMBase::LAPriceDriftSMMBase(const LAPriceDriftSMMBase& v) 
: LAMathDriftFuncBase(v), mTenor(v.mTenor), mDeltaTenor(v.mDeltaTenor), m_i(v.m_i), mVolatility(v.mVolatility),
mSDEAttrName(v.mSDEAttrName), mpTimes(v.mpTimes), mCorData(v.mCorData), mPos_old(v.mPos_old), mpCache(0), mSpread(v.mSpread)
{
	mCorrelation.resize(v.mCorrelation.size());
	for (unsigned int i = 0; i < mCorrelation.size(); i++)
	{
		mCorrelation[i].resize(v.mCorrelation[i].size());
		for (unsigned int j = 0; j < mCorrelation[i].size(); j++)
		{
			if (m_i == 0) mCorrelation[i][j] = dynamic_cast<AQLFunctionBase*>(v.mCorrelation[i][j]->clone());
			else mCorrelation[i][j] = dynamic_cast<AQLFunctionBase*>(v.mCorrelation[i][j]);
		}
	}
	if (m_i == 0 && v.mpCache != 0)
		mpCache = new DoubleArray(*v.mpCache);
}

/*!
	@brief destructor
*/
LAPriceDriftSMMBase::~LAPriceDriftSMMBase() 
{
	if (m_i == 0)
	{
		for (unsigned int i = 0; i < mCorrelation.size(); i++)
			for (unsigned int j = 0; j < mCorrelation[i].size(); j++)
				delete mCorrelation[i][j];
		if (mpCache != 0) delete mpCache;
	}
}
/*!
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LAPriceDriftSMMBase::isTypeOf(function_t id) const
{
	return (id==FN_DRIFTSMMBASE ? true : LAMathDriftFuncBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LAPriceDriftSMMBase::getType() const
{
	return FN_DRIFTSMMBASE;
}

/*!
    @brief return string representaion
    @return string representaion (sde attr name : suffix : tenor : deltatenor)
*/
AQLString
LAPriceDriftSMMBase::convertToString(void) const
{
	AQLString ret;
	ret += mSDEAttrName;
	ret += ":";
	ret += AQLDataInt(m_i).convertToString();
	ret += ":";
	ret += AQLDataDoubles(mTenor).convertToString();
	ret += ":";
	ret += AQLDataDoubles(mDeltaTenor).convertToString();

	return ret;
}

/*!
    @brief transform from string representaion
    @param[in] string representaion  (sde attr name : suffix : tenor : deltatenor)
*/
void
LAPriceDriftSMMBase::convertFromString(const AQLString& str)
{
	AQLDataStrings tmp;
	tmp.convertFromString(str);
	if (tmp.getSize() < 5 || tmp.getSize() % 2 == 0)
	{
		//error
		throw AQLCoreInvalidData("Format is something wrong", __FILE__, __LINE__);
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
LAPriceDriftSMMBase::setTenor(const DoubleArray& tenor, const DoubleArray& delta_tenor)
{
	if (tenor.size() != delta_tenor.size() + 1)
	{
		//error
		throw AQLCoreInvalidData("tenor size must be delta_tenor size plus one", __FILE__, __LINE__);
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
LAPriceDriftSMMBase::setUp(LAMathPathEntity& path)
{
	AQLDataHolder* dh = &path.getData(mSDEAttrName, ISNOTNULL);
	LAMathAttrSDE* pattrsde = &dynamic_cast<LAMathAttrSDE&>(dh->get());
	mVolatility = pattrsde->getSDE().getVolatility();
	mpTimes = 0;
	mpTimes = &pattrsde->getSDE().getBM()->getTimeGrid();	

	if (m_i == 0)
	{
		AQLString name = COR;
		name += "_";
		name += mSDEAttrName;
		dh = &path.getData(name, ISNOTNULL);
		AQLDataReference* ref = &dynamic_cast<AQLDataReference&>(dh->get());
		LAMathCorrelation* cor = &dynamic_cast<LAMathCorrelation&>(ref->get().get());
		mCorrelation = cor->getCorrelationFunc();

		if (mpCache != 0) {delete mpCache; mpCache = 0;}
		mpCache = new DoubleArray(mDeltaTenor.size());
	}	
	else
	{
		AQLFunctionBase* method = pattrsde->getSDE().getDrift()[0];
		//if (method->isTypeOf(FN_DRIFTQUANTADJ))
		//	method = dynamic_cast<LAPriceDriftQuantAdjustment*>(method)->getDrift();
	 
		if (!method->isTypeOf(getType()))
		{
			//error
			throw AQLCoreInvalidData("dirft function is not LAPriceDriftSMMBase", __FILE__, __LINE__);
		}
		LAPriceDriftSMMBase* drift = dynamic_cast<LAPriceDriftSMMBase*>(method);
		mCorrelation = drift->mCorrelation;
		mpCache = drift->mpCache;
	}

	mPos_old = 0;

}