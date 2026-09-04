/*! @file
    @brief Source code of drift function of SMM sde (annuity measure)



*/
//  2007, AlgoQuantHub..

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLPriceDriftSMMAnnuity.h"

#include "AQLMathPathEntity.h"
#include "AQLDataHolder.h"
#include "AQLDataVector.h"
#include "AQLDataReference.h"
#include "AQLObjectHolder.h"
#include "AQLMathAttrSDE.h"
#include "AQLRatesSDEBase.h"
#include "AQLModelDynamicsCurve.h"
#include "AQLMathCorrelation.h"
#include "AQLMathVolFuncBase.h"
#include "AQLAlgorithm.h"

#include "AQLPriceDriftQuantAdjustment.h"

using namespace std;
#define COR "COR"

//================ AQLPriceDriftSMMAnnuity ===================================
/*!
	@brief default constructor
	@param[in] s spread added to S (for displaced diffusion case)

*/
AQLPriceDriftSMMAnnuity::AQLPriceDriftSMMAnnuity(double s)
: AQLPriceDriftSMMBase(s)
{
}
/*!
	@brief default constructor
	@param[in] sdeAttrName data name of sde
	@param[in] i suffix
	@param[in] tenor tenor
	@param[in] deltatenor delta of tenor
	
*/
AQLPriceDriftSMMAnnuity::AQLPriceDriftSMMAnnuity(const AQLString& sdeAttrName, unsigned int i, const DoubleArray& tenor, const DoubleArray& delta_tenor, double s)
: AQLPriceDriftSMMBase(sdeAttrName, i, tenor, delta_tenor, s)
{
}
/*!
	@brief copy constructor
	@param[in] v copy source
*/
AQLPriceDriftSMMAnnuity::AQLPriceDriftSMMAnnuity(const AQLPriceDriftSMMAnnuity& v) 
: AQLPriceDriftSMMBase(v)
{
}

/*!
	@brief destructor
*/
AQLPriceDriftSMMAnnuity::~AQLPriceDriftSMMAnnuity() 
{

}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLPriceDriftSMMAnnuity::clone() const	
{
    try 
	{
		return new AQLPriceDriftSMMAnnuity(*this);
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
AQLPriceDriftSMMAnnuity::isTypeOf(function_t id) const
{
	return (id == FN_DRIFTSMMANNUITY ? true : AQLMathDriftFuncBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLPriceDriftSMMAnnuity::getType() const
{
	return FN_DRIFTSMMANNUITY;
}


/*!
    @brief Return drift value
	@param[in] x input value, x[0] = t, x[1] = S_{s}, x[2] = S_{s+1}, x[x.size()-1]= S_{M},where S_{s} reset time > t and S_{s-1} reset time <= t
    @return drift value

*/
double
AQLPriceDriftSMMAnnuity::operator()(const DoubleArray& x) const
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
		// no cache case. calculate all.
		for (int j = mDeltaTenor.size() - 1; j > static_cast<int>(m_i); j--, k--)
		{
			if (flag_data)
				ret += getLpart(x[k], mDeltaTenor[j]) 
								* mCorData[mPos_old][j - m_i - 1] 
								* get_s(static_cast<int>(m_i), j, mDeltaTenor, x)
								* mVolatility[j][0]->operator ()(x);
			else
				ret += getLpart(x[k], mDeltaTenor[j]) 
								* mCorrelation[m_i][j]->operator ()(x[0])
								* get_s(static_cast<int>(m_i), j, mDeltaTenor, x)
								* mVolatility[j][0]->operator ()(x);
		}
	}
	else
	{
		//use cache data
		if (m_i == mDeltaTenor.size() + 1 - x.size())
		{
			for (int j = mDeltaTenor.size() - 1; j > static_cast<int>(m_i); j--, k--)
				(*mpCache)[j] = getLpart(x[k], mDeltaTenor[j]) * get_s(static_cast<int>(m_i), j, mDeltaTenor, x) * mVolatility[j][0]->operator ()(x);
		}
		for (int j = mDeltaTenor.size() - 1; j > static_cast<int>(m_i); j--)
		{
			if (flag_data)
				ret += (*mpCache)[j] * mCorData[mPos_old][j - m_i - 1];
			else
				ret += (*mpCache)[j] * mCorrelation[m_i][j]->operator ()(x[0]);
		}		
	}
	ret *= 1 / get_s(static_cast<int>(m_i), static_cast<int>(m_i), mDeltaTenor, x);
	ret *= -mVolatility[m_i][0]->operator ()(x);   	
	return ret;
/*	
	//double s = 0.0;
	//double _s = 0.0;
	k = x.size() - 1;
	for (int j = mDeltaTenor.size() - 1; j >= static_cast<int>(m_i); j--, k--)
	{
		s = mDeltaTenor[j];
		for (int l = j; l >= static_cast<int>(m_i) + 1; l--)
			s *= 1 + mDeltaTenor[l] * x[k + l - j];
		_s += s;
	}
*/
}

/*!
	@brief set up this class
	@param[in] path path object 
*/
void
AQLPriceDriftSMMAnnuity::setUp(AQLMathPathEntity& path)
{
	AQLPriceDriftSMMBase::setUp(path);

	mCorData.clear();
	mCorData.resize(mpTimes->size());
	for (unsigned int i = 0; i < mpTimes->size(); i++)
	{		
		if ((*mpTimes)[i] >= mTenor[m_i])
		{
			mCorData.resize(i);
			break;
		}				
		mCorData[i].resize(mDeltaTenor.size() - m_i - 1);
		for (unsigned int j = 0; j < mCorData[i].size(); j++)
		 	mCorData[i][j] = mCorrelation[m_i][m_i + j + 1]->operator ()((*mpTimes)[i]);
	}
}
