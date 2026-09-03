/*! @file
    @brief Source code of drift function of LMM sde (forward measure)



*/
//  2007, AlgoQuantHub..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAPriceDriftLMMForward.cpp
//
//  SYNOPSIS    :       LAPriceDriftLMMForward
//  DESCRIPTION :       Source code of drift function of LMM sde (forward measure)
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


#include "LAPriceDriftLMMForward.h"

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

using namespace std;

#define COR "COR"
const double INFINITESIMAL = 1E-7; 

//================ LAPriceDriftLMMForward ===================================
/*!
	@brief default constructor
	@param[in] point forward measure point(DiscountBond who's maturity is  mPoint-th forward libor cf time(mTenor[mPoint + 1]) is numerarire)  
	@param[in]Q val for displaced diffusion case
*/
LAPriceDriftLMMForward::LAPriceDriftLMMForward(unsigned int point, double Q)
: LAPriceDriftLMMBase(Q), mPoint(point)
{
}
/*!
	@brief default constructor
	@param[in] sdeAttrName data name of sde
	@param[in] i suffix
	@param[in] tenor tenor
	@param[in] deltatenor delta of tenor
	@param[in] point forward measure point(DiscountBond who's maturity is  mPoint-th forward libor cf time(mTenor[mPoint + 1]) is numerarire)  
	@param[in] Q val for displaced diffusion case
*/
LAPriceDriftLMMForward::LAPriceDriftLMMForward(const AQLString& sdeAttrName, unsigned int i, const DoubleArray& tenor, const DoubleArray& delta_tenor, unsigned int point, double Q)
: LAPriceDriftLMMBase(sdeAttrName, i, tenor, delta_tenor, Q), mPoint(point)
{
	if (tenor.at(0) == 0.0)
	{
		if (mPoint == 0)
		{
			throw AQLCoreInvalidData("if tenor[0] = 0.0, point must be more than zero", __FILE__, __LINE__);
		}
		mPoint--;
	}
}


/*!
	@brief copy constructor
	@param[in] v copy source
*/
LAPriceDriftLMMForward::LAPriceDriftLMMForward(const LAPriceDriftLMMForward& v) 
: LAPriceDriftLMMBase(v), mPoint(v.mPoint)
{

}

/*!
	@brief destructor
*/
LAPriceDriftLMMForward::~LAPriceDriftLMMForward() 
{

}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
LAPriceDriftLMMForward::clone() const	
{
    try 
	{
		return new LAPriceDriftLMMForward(*this);
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
LAPriceDriftLMMForward::isTypeOf(function_t id) const
{
	return (id==FN_DRIFTLMMFORWARD ? true : LAMathDriftFuncBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LAPriceDriftLMMForward::getType() const
{
	return FN_DRIFTLMMFORWARD;
}


/*!
    @brief Return drift value
	@param[in] x input value, x[0] = t, x[1] = L_{s}, x[2] = L_{s+1}, x[x.size()-1]= L_{M},where L_{s} reset time >t and L_{s-1} reset time <= t  
    @return drift value

*/
double
LAPriceDriftLMMForward::operator()(const DoubleArray& x) const
{
	if (mDeltaTenor.size() - m_i >= x.size()) return 0.0;
	if (m_i == mPoint) return 0.0;
	
	double ret = 0.0;
	int k = x.size() - 1;
	
	bool flag_data = true;
	if (mpTimes == 0) flag_data = false;
	else
	{
		unsigned int pos;
		if (x[0] == 0.0) pos = 0;
		else if (x[0] == (*mpTimes)[mPos_old]) pos = mPos_old;
		else if (mPos_old + 1 < mpTimes->size() && x[0] == (*mpTimes)[mPos_old + 1]) pos = mPos_old + 1;
		else if (!AQLAlgorithm::find<DoubleArray, double>(*mpTimes, x[0], 0, mpTimes->size() - 1, pos))
		{
			pos = 0;
			mPos_old = 0;
			flag_data = false;	
		}
		mPos_old = pos; 		
	}		
	
	if (mpCache == 0)
	{
		//no cache case. calculate all.
		if (m_i == mPoint)
			;
		else if (m_i > mPoint)
		{
			unsigned int lnum = mDeltaTenor.size();
			for (unsigned int j = mPoint + 1; j <= m_i; j++)
			{
				if (k + j + 1 <= lnum) continue;
				if (flag_data)
					ret += getLpart(x[k + j + 1 - lnum], mDeltaTenor[j]) * mCorData[mPos_old][j - mPoint -1]
							* mVolatility[j][0]->operator ()(x);
				else
					ret += getLpart(x[k + j + 1 - lnum], mDeltaTenor[j]) * mCorrelation[m_i][j]->operator ()(x[0])
							* mVolatility[j][0]->operator ()(x);				
			}
		}
		else
		{
			unsigned int lnum = mDeltaTenor.size();
			for (unsigned int j = m_i + 1; j <= mPoint; j++)
			{
				if (flag_data)
					ret -= getLpart(x[k + j + 1 - lnum], mDeltaTenor[j]) * mCorData[mPos_old][j - m_i -1]
							* mVolatility[j][0]->operator ()(x);
				else
					ret -= getLpart(x[k + j + 1 - lnum], mDeltaTenor[j]) * mCorrelation[m_i][j]->operator ()(x[0])
							* mVolatility[j][0]->operator ()(x);				
			}
		}
	}
	else 
	{
		//use cache data
		if (/*m_i == mDeltaTenor.size() + 1 - x.size() && */(*mpCacheTime) != x[0])
		{
			int lbound = m_i > mPoint ? static_cast<int>(mPoint) : static_cast<int>(m_i);
			for (int j = mDeltaTenor.size() - 1; j > lbound && k > 0; j--, k--)
			{
				(*mpCache)[j] = getLpart(x[k], mDeltaTenor[j]);// * mVolatility[j][0]->operator ()(x);
				(*mpCache2)[j] = (*mpCache)[j] * mVolatility[j][0]->operator ()(x);
				
				if (x[0] == 0.0)
				{
					//if time = 0.0 then storage data in cache_before for use of integral method(long jump)
					(*mpCache_before)[j] = (*mpCache)[j];
					*mpCacheTime_before = 0.0;
				}				
			}
			*mpCacheTime = x[0];
		}

		k = x.size() - 1;
		if (m_i == mPoint)
			;
		else if (m_i > mPoint)
		{
			unsigned int lnum = mDeltaTenor.size();
			for (unsigned int j = mPoint + 1; j <= m_i; j++)
			{				
				if (k + j + 1 <= lnum) continue;
				if (flag_data)
					ret += (*mpCache2)[j] * mCorData[mPos_old][j - mPoint - 1];
				else
					ret += (*mpCache2)[j] * mCorrelation[m_i][j]->operator ()(x[0]);
			}				
		}
		else
		{
			for (unsigned int j = m_i + 1; j <= mPoint; j++)
			{
				if (flag_data)
					ret -= (*mpCache2)[j] * mCorData[mPos_old][j - m_i - 1];
				else
					ret -= (*mpCache2)[j] * mCorrelation[m_i][j]->operator ()(x[0]);
			}				
		}

	}
	
	ret *= mVolatility[m_i][0]->operator ()(x);   	
 	return ret;	

}
/*!
    @brief transform from string representaion
    @param[in] string representaion  (sde attr name : suffix : tenor : deltatenor)
*/
void
LAPriceDriftLMMForward::convertFromString(const AQLString& str)
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
		if (mPoint == 0)
		{
			throw AQLCoreInvalidData("if tenor[0] = 0.0, point must be more than zero", __FILE__, __LINE__);
		}		
		mPoint--;
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
LAPriceDriftLMMForward::setTenor(const DoubleArray& tenor, const DoubleArray& delta_tenor)
{
	if (tenor.at(0) == 0.0 && mTenor.size() == 0)
	{
		if (mPoint == 0)
		{
			throw AQLCoreInvalidData("if tenor[0] = 0.0, point must be more than zero", __FILE__, __LINE__);
		}		
		mPoint--;
	}


	LAPriceDriftLMMBase::setTenor(tenor, delta_tenor);	
	if (mPoint + 1 >= mTenor.size())
	{
		//error
		throw AQLCoreInvalidData("point + 1 must be less than tenor size", __FILE__, __LINE__);
	}
}

/*!
    @brief Return integral result
	@param[in] x integral region
    @return integral result
*/
double
LAPriceDriftLMMForward::integral(const std::vector<std::pair<double,double> >& x) const
{
	if (x[0].second > mTenor[m_i] + INFINITESIMAL) return 0.0;
	if (!mIsCovSetuped && m_i == 0) setUpCovariance();
	
	double ret = 0.0;
	unsigned int size = x.size() - 1;
	

	if (mpCache_before != 0 && x[0].first == 0.0 && *mpCacheTime_before == 0.0)
	{
		unsigned int pos_e;
		if (mPos_old + 1 < (*mpTimes).size() && x[0].second == (*mpTimes)[mPos_old + 1])
			pos_e = mPos_old + 1;
		else if (!AQLAlgorithm::find<DoubleArray, double>(*mpTimes, x[0].second, 0, mpTimes->size() - 1, pos_e))
			pos_e = 0;
		mPos_old = pos_e; 		

		const vector<DoubleMatrix>& cov = mpCovariance->getIntegratedCovData(); 

		if (m_i == mPoint) 
			;
		else if (m_i > mPoint)
		{
			mX[0] = x[0].second;
			(*mpCache)[m_i] = getLpart(x[m_i + 1].second, mDeltaTenor[m_i]);
			(*mpCache2)[m_i] = (*mpCache)[m_i] * mVolatility[m_i][0]->operator ()(mX[0]);
		}
		else if ((*mpCacheTime) != x[0].second)
		{
			mX[0] = x[0].second;
			for (unsigned int i = m_i + 1; i <= mPoint; i++)
			{
				if (mTenor[i] + INFINITESIMAL < x[0].second) continue;
				(*mpCache)[i] = getLpart(x[i + 1].second, mDeltaTenor[i]);
				(*mpCache2)[i] = (*mpCache)[i] * mVolatility[i][0]->operator ()(mX[0]);
			}
		}		
		
		if (m_i + 1 == size && pos_e == 1) *mpCacheTime = -1.0;
		else *mpCacheTime = x[0].second;
		

		if (m_i == mPoint) 
			;		
		else if (m_i > mPoint)
		{
			for (unsigned int i = m_i; i >= mPoint + 1; i--)
			{
				if (mTenor[i] + INFINITESIMAL < x[0].second) break;
				double ret2 = (*mpCache_before)[i] + (*mpCache)[i];		
				if (pos_e != 0) ret += ret2 * cov[pos_e][m_i][i];//mpCovariance->getIntegratedCov(m_i, i, 0, pos_e);
				else ret += ret2 * mpCovariance->getIntegratedCov(m_i, i, 0.0, x[0].second);
			}
		}
		else
		{
			for (unsigned int i = m_i + 1; i <= mPoint; i++)
			{
				double ret2 = -(*mpCache_before)[i] - (*mpCache)[i];				 
				if (pos_e != 0) ret += ret2 * cov[pos_e][i][m_i];//mpCovariance->getIntegratedCov(m_i, i, 0, pos_e);
				else if (mTenor[m_i] < x[0].second)
					ret += ret2 * mpCovariance->getIntegratedCov(m_i, i, 0.0, mTenor[m_i]);
				else
					ret += ret2 * mpCovariance->getIntegratedCov(m_i, i, 0.0, x[0].second);
			}		

		}
		return 0.5 * ret;
	}

	
	if (mTenor[m_i] <= x[0].first + INFINITESIMAL) return 0.0;

	unsigned pos_s = 0, pos_e = 0;
	if (mpCache != 0 && mpCache_before != 0)
	{
		if (mPos_old + 1 < (*mpTimes).size() && x[0].second == (*mpTimes)[mPos_old + 1])
			pos_e = mPos_old + 1;
		else if (!AQLAlgorithm::find<DoubleArray, double>(*mpTimes, x[0].second, 0, mpTimes->size() - 1, pos_e))
			pos_e = 0;
		mPos_old = pos_e; 					
	
		if (pos_e != 0)
		{
			if (x[0].first == (*mpTimes)[mPos_old - 1])
				pos_s = mPos_old - 1;
			else if (!AQLAlgorithm::find<DoubleArray, double>(*mpTimes, x[0].first, 0, mpTimes->size() - 1, pos_s))
				pos_s = 0;
		}
	}


	if (mpCache_before != 0 && mpCache != 0 && (m_i == 0 || mTenor[m_i - 1] + INFINITESIMAL < x[0].second))
	{
		for (unsigned int i = m_i + 1 ; i < size; i++)
		{
			if (*mpCacheTime == x[0].first)
				(*mpCache_before)[i] = (*mpCache)[i];
			else
				(*mpCache_before)[i] = (*mpCache_before)[i] = getLpart(x[i + 1].first, mDeltaTenor[i]);
			(*mpCache)[i] = getLpart(x[i + 1].second, mDeltaTenor[i]);
			(*mpCache2)[i] = (*mpCache)[i] * mVolatility[i][0]->operator ()(x[0].second);
		}

	}	

	if (m_i == mPoint)
		;
	else if (m_i > mPoint)
	{
		for (unsigned int i = m_i; i >= mPoint + 1; i--)
		{
			if (mTenor[i] + INFINITESIMAL < x[0].second) break;
			double ret2;
			if (mpCache_before != 0)
				ret2 = (*mpCache_before)[i] + (*mpCache)[i];		
			else
				ret2 = getLpart(x[i + 1].first, mDeltaTenor[i]) + getLpart(x[i + 1].second, mDeltaTenor[i]);
			if (pos_s != 0 && pos_e != 0)
				ret += ret2 * mpCovariance->getIntegratedCov(i, m_i, pos_s, pos_e);
			else ret += ret2 * mpCovariance->getIntegratedCov(m_i, i,  x[0].first, x[0].second);
		}
	}
	else
	{
		for (unsigned int i = m_i + 1; i <= mPoint; i++)
		{		
			double ret2;
			if (mpCache_before != 0)
				ret2 = -(*mpCache_before)[i] - (*mpCache)[i];		
			else
				ret2 = -getLpart(x[i + 1].first, mDeltaTenor[i]) - getLpart(x[i + 1].second, mDeltaTenor[i]);
					
			if (pos_s != 0 && pos_e != 0)
				ret += ret2 * mpCovariance->getIntegratedCov(i, m_i, pos_s, pos_e);
			else if (mTenor[m_i] < x[0].second)
				ret += ret2 * mpCovariance->getIntegratedCov(m_i, i,  x[0].first, mTenor[m_i]);
			else
				ret += ret2 * mpCovariance->getIntegratedCov(m_i, i,  x[0].first, x[0].second);
		}		
	}
		
	return 0.5 * ret;
}

/*!
	@brief set up this class
	@param[in] path path object 
*/
void
LAPriceDriftLMMForward::setUp(LAMathPathEntity& path)
{
	if (mPoint + 1 >= mTenor.size())
	{
		//error
		throw AQLCoreInvalidData("point + 1 must be less than tenor size", __FILE__, __LINE__);
	}

	LAPriceDriftLMMBase::setUp(path);

	mCorData.clear();
	if (m_i == mPoint) return;

	mCorData.resize(mpTimes->size());
	for (unsigned int i = 0; i < mpTimes->size(); i++)
	{		
		if ((*mpTimes)[i] + INFINITESIMAL >= mTenor[m_i])
		{
			mCorData.resize(i);
			break;
		}		
		if (m_i < mPoint)
		{
			mCorData[i].resize(mPoint - m_i);
			unsigned int size = mCorData[i].size();
			for (unsigned int j = 0; j < size; j++)
		 		mCorData[i][j] = mCorrelation[m_i][m_i + j + 1]->operator ()((*mpTimes)[i]);		
		}
		else
		{
			mCorData[i].resize(m_i - mPoint);
			unsigned int size = mCorData[i].size();
			for (unsigned int j = 0; j < size; j++)
		 		mCorData[i][j] = mCorrelation[m_i][mPoint + j + 1]->operator ()((*mpTimes)[i]);		
		}
	}
}

