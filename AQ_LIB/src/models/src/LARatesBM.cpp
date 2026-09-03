/*! @file
    @brief Source code of brownian motion class


*/
//  2007, Mizuho International London..


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LARatesBM.h"
#include "LARandBase.h"

using namespace std;

#define GENERATOR	1
#define OTHERBM	2

//================ LARatesBM ===================================
/*!
	@brief constructor
	@param[in] timegrid time grid
	@param[in] rand rand generator
	@param[in] factor_num number of factor
*/
LARatesBM::LARatesBM(const DoubleArray& timegrid, const LARandBase& rand, unsigned int factor_num)
: mType(GENERATOR), mID(0), mRefCount(0), mpRand(0), mTimeGrid(timegrid),
   mIsAntithetic(false), mIsOdd(true), mDeleteFlag(false) 
{
	if (rand.getDim()[0] != factor_num * (timegrid.size() - 1))
	{
		//error
		throw LACoreInvalidData("rand generator dimension is not valid", __FILE__, __LINE__);
	}
	mpRand = dynamic_cast<LARandBase*>(rand.clone());
	
	mBuff.resize(rand.getDim()[0]);
	mData.resize(timegrid.size() - 1);
	for (unsigned int i = 0; i < mData.size(); i++)
		mData[i].resize(factor_num);
	
	mDataInstanceDt.resize(timegrid.size() - 1);
	for (unsigned int i = 0; i < mDataInstanceDt.size(); i++)
		mDataInstanceDt[i] = LAMath::sqrt(timegrid[i + 1] - timegrid[i]);
	
	mLoading.resize(factor_num);
	for (unsigned int i = 0; i < mLoading.size(); i++)
	{
		mLoading[i].resize(factor_num);
		for (unsigned int j = 0; j < mLoading[i].size(); j++)
		{
			mLoading[i][j].resize(timegrid.size() - 1);
			if (i == j)
			{
				for (unsigned int k = 0; k < mLoading[i][j].size(); k++)
					mLoading[i][j][k] = 1.0;
			}
			else
			{
				for (unsigned int k = 0; k < mLoading[i][j].size(); k++)
					mLoading[i][j][k] = 0.0;
			}
		}
	}

}

/*!
	@brief constructor
	@param[in] timegrid time grid
	@param[in] rand rand generator
	@param[in] loading factor loading(i * j * time)
*/
LARatesBM::LARatesBM(const DoubleArray& timegrid, const LARandBase& rand, const std::vector<DoubleMatrix>& loading)
: mType(GENERATOR), mID(0), mRefCount(0), mpRand(0), mLoading(loading), mTimeGrid(timegrid),
   mIsAntithetic(false), mIsOdd(true), mDeleteFlag(false)
{
	if (rand.getDim()[0] != loading.at(0).size() * (timegrid.size() - 1))
	{
		//error
		throw LACoreInvalidData("rand generator dimension is not valid", __FILE__, __LINE__);
	}

	mpRand = dynamic_cast<LARandBase*>(rand.clone());	
	
	mBuff.resize(rand.getDim()[0]);
	mData.resize(timegrid.size() - 1);
	unsigned int count = loading.size();
	unsigned int pos = 0;
	for (unsigned int i = 0; i < mData.size(); i++)
	{
		if (loading[pos][0].size() < i + 1)
		{	
			count--;
			pos++;
		}
		mData[i].resize(count);
	}
	
	mDataInstanceDt.resize(timegrid.size() - 1);
	for (unsigned int i = 0; i < mDataInstanceDt.size(); i++)
		mDataInstanceDt[i] = LAMath::sqrt(timegrid[i + 1] - timegrid[i]);

}

/*!
	@brief constructor
	@param[in] bm BM classes
	@param[in] loading factor loading(i * j * time)
	@param[in] deleteflag delete flag of input LARatesBM	
*/
LARatesBM::LARatesBM(vector<pair<LARatesBM*, unsigned int> >& bm, 
			   const std::vector<DoubleMatrix>& loading,
			    bool deleteflag)
: mType(OTHERBM), mID(0), mRefCount(0),  mpRand(0), mBM(bm), mLoading(loading),
   mIsAntithetic(false), mIsOdd(true), mDeleteFlag(deleteflag)
{

	for (unsigned int i = 0; i < mBM.size(); i++)
	{
		if ( i > 0 )
		{
			unsigned int size = mBM[0].first->getTimeGrid().size();
			if (size != mBM[i].first->getTimeGrid().size())
			{
				//error
				throw LACoreInvalidData("time grid is not consistent", __FILE__, __LINE__);
			}
			for (unsigned int j = 0; j < size ; j++)
			{
				if (mBM[0].first->getTimeGrid()[j] != mBM[i].first->getTimeGrid()[j])
				{
					//error
					throw LACoreInvalidData("time grid is not consistent", __FILE__, __LINE__);
				}

			}
		}
		if (mDeleteFlag) mBM[i].first->mRefCount++;
	}

	mData.resize(mBM[0].first->getTimeGrid().size() - 1);
	unsigned int count = loading.size();
	unsigned int pos = 0;
	for (unsigned int i = 0; i < mData.size(); i++)
	{
		if (loading[pos][0].size() < i + 1)
		{	
			count--;
			pos++;
		}
		mData[i].resize(count);
	}
}
/*!
	@brief copy constructor
	@param[in] v copy source
*/
LARatesBM::LARatesBM(const LARatesBM& v)
: mRefCount(0), mType(v.mType), mpRand(0), mLoading(v.mLoading), mID(v.mID), mBM(v.mBM)
, mIsAntithetic(v.mIsAntithetic), mIsOdd(v.mIsOdd), mData(v.mData), mDataInstanceDt(v.mDataInstanceDt)
, mTimeGrid(v.mTimeGrid), mBuff(v.mBuff), mAntiData(v.mAntiData), mDeleteFlag(v.mDeleteFlag)  

{
	if (mType == GENERATOR) mpRand = dynamic_cast<LARandBase*>(v.mpRand->clone());
	else if (mDeleteFlag)
	{
		for (unsigned int i = 0; i < mBM.size(); i++) 
			mBM[i].first->mRefCount++;
	}
}

/*!
	@brief Make copy(clone) of this class
	@return Adress to LARatesBM
*/
LARatesBM*
LARatesBM::clone() const
{
    try
    {
		return new LARatesBM(*this);
    }
    catch (bad_alloc &e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
	@brief destructor
*/
LARatesBM::~LARatesBM() 
{
	mData.clear();
	mAntiData.clear();
	mDataInstanceDt.clear();
	mLoading.clear();
	mTimeGrid.clear();
	mBuff.clear();
	delete mpRand;
	if (mType == OTHERBM && mDeleteFlag) 
	{
		for (unsigned int i = 0; i < mBM.size(); i++)
		{
			if (--mBM[i].first->mRefCount == 0)	
				delete mBM[i].first;
		}
		mBM.clear();
	}
}
/*!
	@brief get time grid
	@return time grid
*/
const DoubleArray&
LARatesBM::getTimeGrid (void) const
{
	if (mType == GENERATOR) return mTimeGrid;
	else return mBM[0].first->getTimeGrid();
}
/*!
	@brief get current id
	@return current id
*/
unsigned int
LARatesBM::getCurrentID() const
{
	if (mType == GENERATOR) return mID;
	else return mBM[0].first->getCurrentID();
}
/*!
	@brief get brownian motion value
	@return brownian motion value
*/
const DoubleMatrix&	
LARatesBM::getBM()
{
/*	if (mType != GENERATOR && mID != getCurrentID() && (!mIsAntithetic || mIsOdd))
		calcBM();
	
	if (!mIsAntithetic || mIsOdd)
	{
		mIsOdd = !mIsOdd;
		return mData;
	}
	else
	{
		mIsOdd = !mIsOdd;
		for (unsigned int i = 0; i < mData.size(); i++)
			for (unsigned int j = 0; j < mData[j].size(); j++)
				mAntiData[i][j] = -mData[i][j];
		
		return mAntiData;
	}*/
	
	if (mType != GENERATOR && mID != getCurrentID())
		calcBM();
	
	if (!mIsAntithetic || !mIsOdd)
		return mData;
	else
		return mAntiData;
	

}
/*!
	@brief calulate brownian motion value
	@param [in] isIdling idling or not
*/
void
LARatesBM::calcBM(bool isIdling)
{
	if (isIdling)
	{
		if (mType == GENERATOR)
		{
			if (!mIsAntithetic || mIsOdd)
				mpRand->getUniforms(mBuff);
			++mID;
		}
		else
			mID = getCurrentID();
		if (mIsAntithetic) mIsOdd = !mIsOdd;
		return;
	}

	
	if (mType == GENERATOR)
	{
		if (!mIsAntithetic || mIsOdd)
		{
			mpRand->getGaussians(mBuff);
			if (mLoading.size() == 0)
			{
				unsigned int k = 0;
				for (unsigned int i = 0; i < mData.size(); i++)
					for (unsigned int j = 0; j < mData[i].size(); j++)
						mData[i][j] = mBuff[k++] * mDataInstanceDt[i] ;
			}
			else
			{
				for (unsigned int i = 0; i < mData.size(); i++)
				{				
					unsigned int pos = mLoading.size() - mData[i].size();
					for (unsigned int j = 0; j < mData[i].size(); j++)
					{
						mData[i][j] = 0;
						unsigned int l = i * mLoading[0].size();
						for (unsigned int k = 0; k < mLoading[pos + j].size(); k++)
							mData[i][j] += mLoading[pos + j][k][i] * mBuff[l++] * mDataInstanceDt[i] ;
					}			
				}	
			}
		}
		++mID;
	}
	else
	{
		if (!mIsAntithetic || mIsOdd)
		{
			vector<const DoubleMatrix*> tmp(mBM.size());		
			for (unsigned int i = 0; i < tmp.size(); i++)
				tmp[i] = &mBM[i].first->getBM();

			for (unsigned int i = 0; i < mData.size(); i++)
			{				
				unsigned int pos = mLoading.size() - mData[i].size();
				for (unsigned int j = 0; j < mData[i].size(); j++)
				{
					mData[i][j] = 0;
					for (unsigned int k = 0; k < mLoading[pos + j].size(); k++)
						mData[i][j] += mLoading[pos + j][k][i] * (*tmp[k])[i][mBM[k].second];
				}			
			}	
		}
		mID = getCurrentID();
	}

	if (mIsAntithetic && !mIsOdd)
		for (unsigned int i = 0; i < mData.size(); i++)
			for (unsigned int j = 0; j < mData[i].size(); j++)
				mAntiData[i][j] = -mData[i][j];
	
	if (mIsAntithetic) mIsOdd = !mIsOdd;
}
 
/*!
	@brief set antithetic or not
	@param[in] flag antithetic flag
*/		
void
LARatesBM::setAntithetic(bool flag)
{
	mIsAntithetic = flag;
	if (mIsAntithetic) mAntiData = mData;//for memroy allocation
	else mAntiData.clear();
	mIsOdd = true;
}
/*!
	@brief get seed
	@return seed
*/
const UlongArray&
LARatesBM::getSeed() const
{
	if (mType == GENERATOR)
		return mpRand->getSeed();
	else
		throw LACoreInvalidData("this method can be available GENERATOR type of BM", __FILE__, __LINE__);

}
/*!
	@brief get dimension
	@return dimension
*/
const UintArray&
LARatesBM::getDim() const
{
	if (mType == GENERATOR)
		return mpRand->getDim();
	else
		throw LACoreInvalidData("this method can be available GENERATOR type of BM", __FILE__, __LINE__);
}


/*!
	@brief set seed
	@param[in] seedValue seed
*/
void
LARatesBM::setSeed(const UlongArray& seedValue)
{
	if (mType == GENERATOR)
		mpRand->setSeed(seedValue);
	else
		throw LACoreInvalidData("this method can be available GENERATOR type of BM", __FILE__, __LINE__);

}
/*!
	@brief set dimension
	@param[in] dimValue dimension
*/
/*void
LARatesBM::setDim(const UintArray& dimValue)
{
	if (mType == GENERATOR)
		mpRand->setDim(dimValue);
	else
		throw LACoreInvalidData("this method can be available GENERATOR type of BM", __FILE__, __LINE__);
}*/
/*!
	@brief calculate correlation between this BM and other BM
	@param[in] bm other BM
	@param[in] pos_other suffix of other BM
	@param[in] pos_this suffix of this BM

*/
DoubleArray
LARatesBM::calcCorrelation(const LARatesBM& bm, unsigned int pos_other, unsigned int pos_this) const
{
	if (getTimeGrid().size() != bm.getTimeGrid().size())
	{
		//error
		throw LACoreInvalidData("time grid size of this BM and input BM are not same", __FILE__, __LINE__);
	}	
	
	DoubleArray ret(getTimeGrid().size(), 0);
	unsigned int i;
	if (mType == GENERATOR && bm.mType == GENERATOR)
	{
		if (this == &bm)
		{
			if (mLoading.size() == 0) return ret;
			
			unsigned int size_this = mLoading[pos_this][0].size();
			unsigned int size_other = mLoading[pos_other][0].size();
			unsigned int size_min = size_this > size_other ? size_other : size_this;
			
			for (i = 0; i < size_min; i++)
			{
				
				double tmp1 = 0/*, tmp2 = 0, tmp3 = 0*/;

				for (unsigned int j = 0; j < mLoading[pos_this].size(); j++)
				{
					tmp1 += mLoading[pos_this][j][i] * mLoading[pos_other][j][i];
//					tmp1 += mLoading[i][pos_this][j] * mLoading[i][pos_other][j];
				//	tmp2 += mLoading[i][pos_this][j] * mLoading[i][pos_this][j];
				//	tmp3 += mLoading[i][pos_other][j] * mLoading[i][pos_other][j];
				}
				ret[i] = tmp1;// / LAMath::sqrt(tmp2 * tmp3);
			}
			ret[i] = ret[i - 1];
		}
		else return ret;
	}
	else if (mType != GENERATOR)
	{
		DoubleArray tmp1(ret.size(), 0);	
		//DoubleArray tmp2(ret.size(), 0);
		
		for (unsigned int j = 0; j < mLoading[pos_this].size(); j++)
		{
			const DoubleArray& cor = bm.calcCorrelation(*mBM[j].first, mBM[j].second, pos_other);
			for (unsigned int i = 0; i < mLoading[pos_this][0].size(); i++)
			{						
				tmp1[i] += mLoading[pos_this][j][i] * cor[i];
				//tmp2[i] += mLoading[i][pos_this][j] * mLoading[i][pos_this][j];
			}
		}
		for (i = 0; i < ret.size() - 1; i++)
			ret[i] = tmp1[i]/* / LAMath::sqrt(tmp2[i])*/;
		ret[i] = ret[i - 1];		
	}
	else 
		return bm.calcCorrelation(*this, pos_this, pos_other);
	return ret;
}


/*!
	@brief get generator BM
	@return generator type BM that this BM refers(this BM not generator case) or this(this BM generator case)
*/
/*LARatesBM*
LARatesBM::getBaseBM() void
{
	if (mType == GENERATOR) return this;
	else return 
}*/


/*!
	@brief get all reference BM
	@return get all reference BM
*/
set<LARatesBM*>
LARatesBM::getReferenceBM()
{
	set<LARatesBM*> ret;
	if (mType == GENERATOR) return ret;
	for (unsigned int i = 0; i < mBM.size(); i++)
	{
		ret.insert(mBM[i].first);	
		set<LARatesBM*> bms = mBM[i].first->getReferenceBM();
		if (bms.size() != 0) ret.insert(bms.begin(), bms.end());
	}
	
	return ret;
}

/*!
	@brief check generator type or not
	@return true:generator type,false:not generator type
*/
bool
LARatesBM::isGeneratorType(void) const
{
	return (mType == GENERATOR);
}