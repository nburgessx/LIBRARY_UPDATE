/*! @file
    @brief Source code of brownian motion class


*/
//  2009, AlgoQuantHub.


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LARatesBM_BB.h"
#include "AQLRandBase.h"

#include <cstdlib>

using namespace std;

#define GENERATOR	1
#define OTHERBM	2

//================ LARatesBM_BB ===================================
/*!
	@brief constructor
	@param[in] timegrid time grid
	@param[in] rand rand generator
	@param[in] factor_num number of factor
*/
LARatesBM_BB::LARatesBM_BB(const DoubleArray& timegrid, const AQLRandBase& rand, unsigned int factor_num)
:LARatesBM(timegrid, rand, factor_num), mNumberOfSteps(timegrid.size() - 1), mNumFactor(factor_num), mLeftIndex(mNumberOfSteps), mRightIndex(mNumberOfSteps), 
mBridgeIndex(mNumberOfSteps), mLeftWeight(mNumberOfSteps), mRightWeight(mNumberOfSteps), mStdDiv(mNumberOfSteps), mSqrtdt(mNumberOfSteps)
{
    mTGrid = timegrid;
    mTGrid.erase(mTGrid.begin());     

    init();
}

/*!
	@brief constructor
	@param[in] timegrid time grid
	@param[in] rand rand generator
	@param[in] loading factor loading(i * j * time)
*/
LARatesBM_BB::LARatesBM_BB(const DoubleArray& timegrid, const AQLRandBase& rand, const std::vector<DoubleMatrix>& loading)
: LARatesBM(timegrid, rand, loading), mNumberOfSteps(timegrid.size() - 1), mNumFactor(1), mLeftIndex(mNumberOfSteps), mRightIndex(mNumberOfSteps), mBridgeIndex(mNumberOfSteps),
mLeftWeight(mNumberOfSteps), mRightWeight(mNumberOfSteps), mStdDiv(mNumberOfSteps), mSqrtdt(mNumberOfSteps)
{
    mTGrid = timegrid;
    mTGrid.erase(mTGrid.begin());

	mNumFactor = loading[0].size();

    init();
 }

/*!
	@brief constructor
	@param[in] bm BM classes
	@param[in] loading factor loading(i * j * time)
	@param[in] deleteflag delete flag of input LARatesBM_BB	
*/
LARatesBM_BB::LARatesBM_BB(std::vector<std::pair<LARatesBM*, unsigned int> >& bm,
                     const std::vector<DoubleMatrix>& loading,
                     bool deleteflag
                     )
: LARatesBM(bm, loading, deleteflag), mNumberOfSteps(bm[0].first->getTimeGrid().size() - 1),  mNumFactor(1), mLeftIndex(mNumberOfSteps), mRightIndex(mNumberOfSteps),
mBridgeIndex(mNumberOfSteps), mLeftWeight(mNumberOfSteps), mRightWeight(mNumberOfSteps), mStdDiv(mNumberOfSteps),mSqrtdt(mNumberOfSteps)
{
    mTGrid = bm[0].first->getTimeGrid();
    mTGrid.erase(mTGrid.begin());

    init();
     
}

/*!
	@brief copy constructor
	@param[in] v copy source
*/
LARatesBM_BB::LARatesBM_BB(const LARatesBM_BB& rhs)
: LARatesBM(rhs), mNumberOfSteps(rhs.mNumberOfSteps), mNumFactor(rhs.mNumFactor), mLeftIndex(rhs.mLeftIndex), mRightIndex(rhs.mRightIndex),
mBridgeIndex(rhs.mBridgeIndex), mLeftWeight(rhs.mLeftWeight), mRightWeight(rhs.mRightWeight), mStdDiv(rhs.mStdDiv)
{
}

/*!
	@brief Make copy(clone) of this class
	@return Adress to LARatesBM_BB
*/
LARatesBM*
LARatesBM_BB::clone() const
{
    try
    {
		return new LARatesBM_BB(*this);
    }
    catch (bad_alloc &e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
	@brief destructor
*/
LARatesBM_BB::~LARatesBM_BB()
{
}

/*!
	@brief calulate brownian motion value
	@param [in] isIdling idling or not
*/
void
LARatesBM_BB::calcBM(bool isIdling)
{
	if (isIdling)
	{
		if (mType == GENERATOR)
		{
			if (!mIsAntithetic || mIsOdd)
				mpRand->getUniforms(mBuff);
			mID = rand() + 1;
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

            DoubleMatrix theWienerProcessPath;
            unsigned int numTGrid = mData.size();
             
            DoubleArray tmp1(numTGrid);
            DoubleArray tmp2(numTGrid);

            unsigned int k = 0;
            DoubleMatrix tmp;
            for (unsigned int i = 0; i < mNumFactor; ++i)
            {           
                for (unsigned int j = 0; j < numTGrid; ++j)
                {
                    tmp1[j] = mBuff[k++];
                }
                buildPath(tmp2, tmp1);
                tmp.push_back(tmp2);
            }

            k = 0;
            for (unsigned int j = 0; j < numTGrid; ++j)
            {
                for (unsigned int i = 0; i < mNumFactor; ++i)
                {
                    mBuff[k++] = tmp[i][j];
                }
            }           

			if (mLoading.size() == 0)
			{
				unsigned int k = 0;
				for (unsigned int i = 0; i < mData.size(); i++)
					for (unsigned int j = 0; j < mData[i].size(); j++)
						mData[i][j] = mBuff[k++] * mDataInstanceDt[i];
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
							mData[i][j] += mLoading[pos + j][k][i] * mBuff[l++] * mDataInstanceDt[i];
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
	@brief build path
	@param [out] path
	@param [in] gaussians
*/
void
LARatesBM_BB::buildPath(DoubleArray &path, const vector<double> &gaussians)
{
	if (gaussians.size() != mNumberOfSteps)
	{
		AQLString msg("gaussians.size() and mNumberOfSteps are not consistent!");
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	} 

	if (path.size() != mNumberOfSteps)
	{
		AQLString msg("path.size() and mNumberOfSteps are not consistent!");
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	} 

	path[mNumberOfSteps - 1] = mStdDiv.front() * gaussians.front();

	for (unsigned int j = 0, i = 1; i < mNumberOfSteps; i++)
	{
	j = mLeftIndex[i];
	unsigned int k = mRightIndex[i];
	unsigned int l = mBridgeIndex[i];

	if( j != 0 )
	{
		path[l] = mLeftWeight[i] * path[j - 1] + mRightWeight[i] * path[k] + mStdDiv[i] * gaussians[i];
	}
	else
	{
		path[l] = mRightWeight[i] * path[k] + mStdDiv[i] * gaussians[i];
	}
	}

	for (unsigned int i = mNumberOfSteps - 1; i >= 1; --i)
	{
		path[i] -= path[i - 1];
		path[i] /= mSqrtdt[i];
	}
	path[0] /= mSqrtdt[0];
}

/*!
	@brief init method
	
*/
void
LARatesBM_BB::init()
{
	mSqrtdt[0] = std::sqrt(mTGrid[0]);
	for(unsigned int i = 1; i < mNumberOfSteps; ++i)
	{
		mSqrtdt[i] = std::sqrt(mTGrid[i] - mTGrid[i - 1]);
	}

	vector <unsigned long> map(mNumberOfSteps, 0);

	// map is used to indicate which points are already constructed. If map[i] is zero, path point i
	// is yet unconstructed.  map[i]-1 is the index of the variate that constructs the path point # i.
	map[mNumberOfSteps - 1] = 1;                       //  The first point in the construction is the global step.
	mBridgeIndex[0] = mNumberOfSteps - 1;             //  The global step is constructed from the first variate.
	mStdDiv[0] = sqrt(mTGrid[mNumberOfSteps - 1]);  //  The variance of the global step is mNumberOfSteps*1.0.
	mLeftWeight[0] = mRightWeight[0] = 0.0;           //  The global step to the last point in time is special.

	for (unsigned int i = 1, j = 0  ; i < mNumberOfSteps; ++i)
	{
		while (map[j])                                //  Find the next unpopulated entry in the map.
		{
			++j;
		}
		unsigned int k = j;

		while (!map[k])                               //  Find the next populated entry in the map from there.
		{
			++k;
		}                         
		unsigned int l = j + (( k - 1 - j ) >> 1);         //  l-1 is now the index of the point to be constructed next.

		map[l]=i;
		mBridgeIndex[i] = l;                           //  The i-th Gaussian variate will be used to set point l-1.
		mLeftIndex[i]   = j;
		mRightIndex[i]  = k;

		if( j != 0 )
		{
			mLeftWeight[i]  = (mTGrid[k] - mTGrid[l]) / (mTGrid[k] - mTGrid[j - 1]);
			mRightWeight[i] = (mTGrid[l] - mTGrid[j - 1]) / (mTGrid[k] - mTGrid[j - 1]);
			mStdDiv[i]       = sqrt(((mTGrid[l] - mTGrid[j - 1]) * (mTGrid[k] - mTGrid[l])) / (mTGrid[k] - mTGrid[j - 1]));
		}
		else
		{
			mLeftWeight[i]  = (mTGrid[k] - mTGrid[l]) / mTGrid[k];
			mRightWeight[i] =  mTGrid[l] / mTGrid[k];
			mStdDiv[i]       = sqrt(mTGrid[l] * (mTGrid[k] - mTGrid[l]) / mTGrid[k]);
		}
		j = k + 1;

		if ( mNumberOfSteps <= j )	                   //	Wrap around.
		{
			j = 0;
		}
	}
}
