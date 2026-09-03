/*! @file
    @brief Source code of drift function of LMM sde (spot measure)



*/
//  2007, AlgoQuantHub..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAPriceDriftLMMSpot.cpp
//
//  SYNOPSIS    :       LAPriceDriftLMMSpot
//  DESCRIPTION :       Source code of drift function of LMM sde (spot measure)
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


#include "LAPriceDriftLMMSpot.h"

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

using namespace std;
#define COR "COR"
const double INFINITESIMAL = 1E-7; 

//================ LAPriceDriftLMMSpot ===================================
/*!
	@brief default constructor
	@param[in] Q val for displaced diffusion case
*/
LAPriceDriftLMMSpot::LAPriceDriftLMMSpot(double Q)
: LAPriceDriftLMMBase(Q)
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
LAPriceDriftLMMSpot::LAPriceDriftLMMSpot(const LAString& sdeAttrName, unsigned int i, const DoubleArray& tenor, const DoubleArray& delta_tenor, double Q)
: LAPriceDriftLMMBase(sdeAttrName, i, tenor, delta_tenor, Q)
{

}
/*!
	@brief copy constructor
	@param[in] v copy source
*/
LAPriceDriftLMMSpot::LAPriceDriftLMMSpot(const LAPriceDriftLMMSpot& v) 
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
	}
*/
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
LAPriceDriftLMMSpot::~LAPriceDriftLMMSpot() 
{

}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LAPriceDriftLMMSpot::clone() const	
{
    try 
	{
		return new LAPriceDriftLMMSpot(*this);
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
LAPriceDriftLMMSpot::isTypeOf(function_t id) const
{
	return (id==FN_DRIFTLMMSPOT ? true : LAMathDriftFuncBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LAPriceDriftLMMSpot::getType() const
{
	return FN_DRIFTLMMSPOT;
}


/*!
    @brief Return drift value
	@param[in] x input value, x[0] = t, x[1] = L_{s}, x[2] = L_{s+1}, x[x.size()-1]= L_{M},where L_{s} reset time > t and L_{s-1} reset time <= t  
    @return drift value

*/
double
LAPriceDriftLMMSpot::operator()(const DoubleArray& x) const
{
	if (mDeltaTenor.size() - m_i >= x.size()) return 0;
	
	double ret = 0.0;
	
	int k = mDeltaTenor.size() - x.size() + 1;
	
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
		for (int j = 1; j <= static_cast<int>(x.size()) - static_cast<int>(mDeltaTenor.size()) + static_cast<int>(m_i); j++, k++)
		{
			if (flag_data)
				ret += getLpart(x[j], mDeltaTenor[k]) * mCorData[mPos_old][j - 1/*k*/]
							* mVolatility[k][0]->operator ()(x);
			else
				ret += getLpart(x[j], mDeltaTenor[k]) * mCorrelation[m_i][k]->operator ()(x[0])
							* mVolatility[k][0]->operator ()(x);
		}
	}
	else
	{
		int l = static_cast<int>(x.size()) - static_cast<int>(mDeltaTenor.size()) + static_cast<int>(m_i);
		if ((*mpCacheTime) != x[0])
		{
			(*mpCache)[m_i] = getLpart(x[l], mDeltaTenor[m_i]);// * mVolatility[m_i][0]->operator ()(x);
			(*mpCache2)[m_i] = (*mpCache)[m_i] * mVolatility[m_i][0]->operator ()(x); 
			if (m_i == mDeltaTenor.size() - 1) *mpCacheTime = x[0];
			if (x[0] == 0.0)
			{
				(*mpCache_before)[m_i] = (*mpCache)[m_i];
				*mpCacheTime_before = 0.0;
			}
		}
		for (int j = 1; j <= l; j++, k++)
		{
			if (flag_data)
				ret += (*mpCache2)[k] * mCorData[mPos_old][j - 1/*k*/];
			else
				ret += (*mpCache2)[k] * mCorrelation[m_i][k]->operator ()(x[0]);
		}
	}
	ret *= mVolatility[m_i][0]->operator ()(x);   	

	return ret;
	
}

/*!
    @brief Return integral result
	@param[in] x integral region
    @return integral result
*/
double
LAPriceDriftLMMSpot::integral(const std::vector<std::pair<double,double> >& x) const
{
//	unsigned int pos;
//	LAAlgorithm::locate<DoubleArray, double>(mTenor, x[0].second, mTenor.size(), pos);
//	if (pos < mTenor.size() && mTenor[pos] == x[0].second) pos++;
	if (!mIsCovSetuped && m_i == 0) setUpCovariance();

	double ret = 0.0;
	//long jump case
	if (mpCache_before != 0 && x[0].first == 0.0 && *mpCacheTime_before == 0.0)
	{
		unsigned int pos_e;
		if (mPos_old + 1 < (*mpTimes).size() && x[0].second == (*mpTimes)[mPos_old + 1])
			pos_e = mPos_old + 1;
		else if (!LAAlgorithm::find<DoubleArray, double>(*mpTimes, x[0].second, 0, mpTimes->size() - 1, pos_e))
			pos_e = 0;
		mPos_old = pos_e; 		

		const vector<DoubleMatrix>& cov = mpCovariance->getIntegratedCovData(); 
		for (int i = m_i ; i >= 0; i--)
		{
			if (mTenor[i] + INFINITESIMAL < x[0].second) break;
			if (i == (int)m_i)
			{
				mX[0] = x[0].second;
				(*mpCache)[m_i] = getLpart(x[m_i + 1].second, mDeltaTenor[m_i]);// * mVolatility[m_i][0]->operator ()(x[0].second);
				(*mpCache2)[m_i] = (*mpCache)[m_i] * mVolatility[m_i][0]->operator ()(mX);
				if (pos_e > 1) *mpCacheTime = x[0].second; // if pos_e == 1, it can be predictor corrector case(not long jump) 
			}
			double ret2 = (*mpCache_before)[i] + (*mpCache)[i];			

			if (pos_e != 0) ret += ret2 * cov[pos_e][m_i][i];//mpCovariance->getIntegratedCov(i, m_i, 0, pos_e);
			else ret += ret2 * mpCovariance->getIntegratedCov(i, m_i, 0.0, x[0].second);
		}
		return 0.5 * ret;
	}
	

	unsigned int pos;
	LAAlgorithm::locate<DoubleArray, double>(mTenor, x[0].second - INFINITESIMAL, mTenor.size(), pos);

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
    	
	for (unsigned int i = pos ; i <= m_i; i++)
	{
		double ret2 = 0.0;
		if (mpCache_before != 0 && mpCache != 0 && i == m_i)
		{
			if (*mpCacheTime == x[0].first) (*mpCache_before)[m_i] = (*mpCache)[m_i];
			else (*mpCache_before)[m_i] = getLpart(x[m_i + 1].first, mDeltaTenor[m_i]);// * mVolatility[m_i][0]->operator ()(x[0].first);
		}

		if (mpCache_before != 0)
			ret2 += (*mpCache_before)[i];
		else
		{
			ret2 += getLpart(x[i + 1].first, mDeltaTenor[i]);// * mVolatility[i][0]->operator ()(x[0].first);
		}

		if (mpCache != 0 && i == m_i)
		{
			(*mpCache)[m_i] = getLpart(x[m_i + 1].second, mDeltaTenor[m_i]);// * mVolatility[m_i][0]->operator ()(x[0].second);
			(*mpCache2)[m_i] = (*mpCache)[m_i] * mVolatility[m_i][0]->operator ()(x[0].second);
		}
			
		if (mpCache != 0) ret2 += (*mpCache)[i];
		else
		{
			ret2 += getLpart(x[i + 1].second, mDeltaTenor[i]);// * mVolatility[i][0]->operator ()(x[0].second);
		}			


		if (pos_s != 0 && pos_e != 0)
			ret += ret2 * mpCovariance->getIntegratedCov(i, m_i, pos_s, pos_e);
		else ret += ret2 * mpCovariance->getIntegratedCov(i, m_i, x[0].first, x[0].second);
	}

	return 0.5 * ret;
}

/*!
	@brief set up this class
	@param[in] path path object 
*/
void
LAPriceDriftLMMSpot::setUp(LAMathPathEntity& path)
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
		mCorData[i].resize(m_i + 1);
//		for (unsigned int j = 0; j <= m_i; j++)
//		 	mCorData[i][j] = mCorrelation[m_i][j]->operator ()((*mpTimes)[i]);

		unsigned int k = 0;
		for (unsigned int j = 0; j <= m_i; j++)
		{
		 	if ((*mpTimes)[i] + INFINITESIMAL >= mTenor[j])
				continue;			
			mCorData[i][k++] = mCorrelation[m_i][j]->operator ()((*mpTimes)[i]);
		}
		mCorData[i].resize(k);
	}
}