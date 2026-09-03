/*! @file
    @brief Source code of drift function of LMM sde (terminal measure)



*/
//  2007, AlgoQuantHub..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAPriceDriftLMMTerminal.cpp
//
//  SYNOPSIS    :       LAPriceDriftLMMTerminal
//  DESCRIPTION :       Source code of drift function of LMM sde (terminal measure)
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


#include "LAPriceDriftLMMTerminal.h"

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

using namespace std;

#define COR "COR"
const double INFINITESIMAL = 1E-7; 

//================ LAPriceDriftLMMTerminal ===================================
/*!
	@brief default constructor
	@param[in] Q val for displaced diffusion case
*/
LAPriceDriftLMMTerminal::LAPriceDriftLMMTerminal(double Q)
: LAPriceDriftLMMBase(Q)
{

}
/*!
	@brief default constructor
	@param[in] sdeAttrName data name of sde
	@param[in] i suffix
	@param[in] tenor tenor
	@param[in] delta_tenor delta of tenor
	@param[in] Q val for displaced diffusion case
*/
LAPriceDriftLMMTerminal::LAPriceDriftLMMTerminal(const LAString& sdeAttrName, unsigned int i, const DoubleArray& tenor, const DoubleArray& delta_tenor, double Q)
: LAPriceDriftLMMBase(sdeAttrName, i , tenor, delta_tenor, Q)
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
LAPriceDriftLMMTerminal::LAPriceDriftLMMTerminal(const LAPriceDriftLMMTerminal& v) 
: LAPriceDriftLMMBase(v)
{
/*	mCorrelation.resize(v.mCorrelation.size());
	for (unsigned int i = 0; i < mCorrelation.size(); i++)
	{
		mCorrelation[i].resize(v.mCorrelation[i].size());
		for (unsigned int j = 0; j < mCorrelation[i].size(); j++)
		{
			if (m_i == 0) mCorrelation[i][j] = dynamic_cast<LAFunctionBase*>(v.mCorrelation[i][j]->clone());
			else mCorrelation[i][j] = dynamic_cast<LAFunctionBase*>(v.mCorrelation[i][j]);
		}
	}*/
/*	if (m_i == 0 && v.mpCache != 0)
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
		mpCovariance = new LARatesCovariance(*v.mpCovariance);*/
}

/*!
	@brief destructor
*/
LAPriceDriftLMMTerminal::~LAPriceDriftLMMTerminal() 
{

}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LAPriceDriftLMMTerminal::clone() const	
{
    try 
	{
		return new LAPriceDriftLMMTerminal(*this);
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
LAPriceDriftLMMTerminal::isTypeOf(function_t id) const
{
	return (id==FN_DRIFTLMMTERMINAL ? true : LAPriceDriftLMMBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LAPriceDriftLMMTerminal::getType() const
{
	return FN_DRIFTLMMTERMINAL;
}


/*!
    @brief Return drift value
	@param[in] x input value, x[0] = t, x[1] = L_{s}, x[2] = L_{s+1}, x[x.size()-1]= L_{M},where L_{s} reset time >t and L_{s-1} reset time <= t  
    @return drift value

*/
double
LAPriceDriftLMMTerminal::operator()(const DoubleArray& x) const
{
	if (mDeltaTenor.size() - m_i >= x.size()) return 0;
	
	
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
		else if (!LAAlgorithm::find<DoubleArray, double>(*mpTimes, x[0], 0, mpTimes->size() - 1, pos))
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
		for (int j = mDeltaTenor.size() - 1; j > static_cast<int>(m_i); j--, k--)
		{
			if (flag_data)
				ret += getLpart(x[k], mDeltaTenor[j]) * mCorData[mPos_old][j - m_i - 1]
						* mVolatility[j][0]->operator ()(x);
			else
				ret += getLpart(x[k], mDeltaTenor[j]) * mCorrelation[m_i][j]->operator ()(x[0])
						* mVolatility[j][0]->operator ()(x);

		}
	}
	else 
	{
		//use cache data
		if (/*m_i == mDeltaTenor.size() + 1 - x.size() && */(*mpCacheTime) != x[0])
		{
			for (int j = mDeltaTenor.size() - 1; j > static_cast<int>(m_i); j--, k--)
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
		for (int j = mDeltaTenor.size() - 1; j > static_cast<int>(m_i); j--, k--)
		{
			if (flag_data)
				ret += (*mpCache2)[j] * mCorData[mPos_old][j - m_i - 1];
						
			else
				ret += (*mpCache2)[j] * mCorrelation[m_i][j]->operator ()(x[0]);
		}		
	}
	
	ret *= -mVolatility[m_i][0]->operator ()(x);   	
 	return ret;	

}


/*!
    @brief Return integral result
	@param[in] x integral region
    @return integral result
*/
double
LAPriceDriftLMMTerminal::integral(const std::vector<std::pair<double,double> >& x) const
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
		else if (!LAAlgorithm::find<DoubleArray, double>(*mpTimes, x[0].second, 0, mpTimes->size() - 1, pos_e))
			pos_e = 0;
		mPos_old = pos_e; 		
		
		bool flag = false;
		if (*mpCacheTime != x[0].second)
		{
			mX[0] = x[0].second;
			flag = true;
		}		
		const vector<DoubleMatrix>& cov = mpCovariance->getIntegratedCovData(); 
		for (unsigned int i = m_i + 1; i < size; i++)
		{
			double ret2 = (*mpCache_before)[i];
			if (flag)
			{
				(*mpCache)[i] = getLpart(x[i + 1].second, mDeltaTenor[i]);// * mVolatility[i][0]->operator ()(x[0].second);
				(*mpCache2)[i] = (*mpCache)[i] * mVolatility[i][0]->operator ()(mX);
			}
			ret2 += (*mpCache)[i];
			
			if (pos_e != 0) ret += ret2 * cov[pos_e][i][m_i];//mpCovariance->getIntegratedCov(m_i, i, 0, pos_e);
			else if (mTenor[m_i] < x[0].second)
				ret += ret2 * mpCovariance->getIntegratedCov(m_i, i, 0.0, mTenor[m_i]);
			else
				ret += ret2 * mpCovariance->getIntegratedCov(m_i, i, 0.0, x[0].second);
		}

		if (pos_e == 1 && m_i + 1 == mDeltaTenor.size()) *mpCacheTime = -1.0;
		else *mpCacheTime = x[0].second;
		return -0.5 * ret;
	}

	if (mTenor[m_i] <= x[0].first + INFINITESIMAL) return 0.0;

	unsigned pos_s = 0, pos_e = 0;
	if (mpCache != 0 && mpCache_before != 0)
	{
		if (mPos_old + 1 < (*mpTimes).size() && x[0].second == (*mpTimes)[mPos_old + 1])
			pos_e = mPos_old + 1;
		else if (!LAAlgorithm::find<DoubleArray, double>(*mpTimes, x[0].second, 0, mpTimes->size() - 1, pos_e))
			pos_e = 0;
		mPos_old = pos_e; 					
	
		if (pos_e != 0)
		{
			if (x[0].first == (*mpTimes)[mPos_old - 1])
				pos_s = mPos_old - 1;
			else if (!LAAlgorithm::find<DoubleArray, double>(*mpTimes, x[0].first, 0, mpTimes->size() - 1, pos_s))
				pos_s = 0;
		}
	}

	for (unsigned int i = m_i + 1 ; i < size; i++)
	{
		double ret2;
		
		if (mpCache_before != 0 && mpCache != 0 && (m_i == 0 || mTenor[m_i - 1] + INFINITESIMAL < x[0].second))
		{
			if (*mpCacheTime == x[0].first) (*mpCache_before)[i] = (*mpCache)[i];
			else (*mpCache_before)[i] = getLpart(x[i + 1].first, mDeltaTenor[i]);// * mVolatility[i][0]->operator ()(x[0].first);
		}		

		if (mpCache_before != 0)
			ret2 = (*mpCache_before)[i];		
		else
		{
			ret2 = getLpart(x[i + 1].first, mDeltaTenor[i]);// * mVolatility[i][0]->operator ()(x[0].first);
		}

		if (mpCache != 0 && (m_i == 0 || mTenor[m_i - 1] + INFINITESIMAL < x[0].second))
		{
			(*mpCache)[i] = getLpart(x[i + 1].second, mDeltaTenor[i]);// * mVolatility[i][0]->operator ()(x[0].second);
			(*mpCache2)[i] = (*mpCache)[i] * mVolatility[i][0]->operator ()(x[0].second);
		}
		
		if (mpCache != 0) ret2 += (*mpCache)[i];
		else
		{
			ret2 += getLpart(x[i + 1].second, mDeltaTenor[i]);// * mVolatility[i][0]->operator ()(x[0].second);
		}
	
		if (pos_s != 0 && pos_e != 0)
			ret += ret2 * mpCovariance->getIntegratedCov(i, m_i, pos_s, pos_e);
		else if (mTenor[m_i] < x[0].second)
			ret += ret2 * mpCovariance->getIntegratedCov(m_i, i,  x[0].first, mTenor[m_i]);
		else
			ret += ret2 * mpCovariance->getIntegratedCov(m_i, i,  x[0].first, x[0].second);
	}



	return -0.5 * ret;
}

/*!
	@brief set up this class
	@param[in] path path object 
*/
void
LAPriceDriftLMMTerminal::setUp(LAMathPathEntity& path)
{
	LAPriceDriftLMMBase::setUp(path);

	mCorData.clear();
	mCorData.resize(mpTimes->size());
	for (unsigned int i = 0; i < mpTimes->size(); i++)
	{		
		if ((*mpTimes)[i] + INFINITESIMAL >= mTenor[m_i])
		{
			mCorData.resize(i);
			break;
		}				
		mCorData[i].resize(mDeltaTenor.size() - m_i - 1);
		for (unsigned int j = 0; j < mCorData[i].size(); j++)
		 	mCorData[i][j] = mCorrelation[m_i][m_i + j + 1]->operator ()((*mpTimes)[i]);	
	}


}

