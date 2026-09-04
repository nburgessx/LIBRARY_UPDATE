/*! @file
    @brief Source code of class to represent LMM volatility function

	This class derives from AQLFunctionBase
*/


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLMathVolFuncLMM.h"


using namespace std;

//================ AQLMathVolFuncLMM ===================================
/*!
	@brief constructor
	@param[in] funcTenor  tenor function
	@param[in] funcTerm   term function
	@param[in] TFix        fixing term
	@param[in] TFix_30_360 fixing term
	@param[in] int        libor suffix
	@param[in] multiple    multiple param
	@param[in] delFlg     tenor and term function delete flg
*/

AQLMathVolFuncLMM::AQLMathVolFuncLMM(const AQLFunctionBase *funcTenor, const AQLFunctionBase *funcTerm, 
				const DoubleArray &TFix, const DoubleArray &TFix_30_360, int suffix, double multiple, bool delFlg)
: AQLFunctionBase(), mpFuncTenor(funcTenor), mpFuncTerm(funcTerm), mTFix(TFix), 
  mTFix_30_360(TFix_30_360), m_t(DoubleArray()), m_T(0.0), mMultiple(multiple), mDelFlg(delFlg)
{
	// data size check
	const int dataSize = TFix.size();
	if (dataSize != static_cast<int>(TFix_30_360.size()))
	{
		throw AQLCoreInvalidData("data size must be same !! ", __FILE__, __LINE__);
	}
	// suffix check
	if (dataSize <= suffix)
	{
		throw AQLCoreInvalidData("suffix must be less than data size  !! ", __FILE__, __LINE__);
	}

	m_T = mTFix_30_360[suffix];
	// resize member array
	mTFix.resize(suffix + 1);
	mTFix_30_360.resize(suffix + 1);
	m_t.resize(suffix, 0.0);

	for (int i = 0; i < suffix; ++i)
	{
		m_t[i] = 0.5 * (mTFix_30_360[i] + mTFix_30_360[i + 1]);
	}
}

/*!
	@brief destructor
*/
AQLMathVolFuncLMM::~AQLMathVolFuncLMM(void)
{
	if (mDelFlg)
	{
		delete mpFuncTerm;
		delete mpFuncTenor;
	}
}


/*!
	@brief copy constructor
*/
AQLMathVolFuncLMM::AQLMathVolFuncLMM(const AQLMathVolFuncLMM &rhs) 
: AQLFunctionBase(), mpFuncTenor(0), mpFuncTerm(0), mTFix(rhs.mTFix), 
  mTFix_30_360(rhs.mTFix_30_360), m_t(rhs.m_t), m_T(rhs.m_T), mMultiple(rhs.mMultiple), mDelFlg(rhs.mDelFlg)
{
	if (mDelFlg)
	{
		// pointer owner
		if (rhs.mpFuncTenor)
		{
			mpFuncTenor = dynamic_cast<const AQLFunctionBase *>(rhs.mpFuncTenor->clone());
		}
		if (rhs.mpFuncTerm)
		{
			mpFuncTerm = dynamic_cast<const AQLFunctionBase *>(rhs.mpFuncTerm->clone());
		}	
	}
	else
	{
		mpFuncTenor = rhs.mpFuncTenor;
		mpFuncTerm = rhs.mpFuncTerm;
	}
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLMathVolFuncLMM::clone() const
{
    try 
	{
		return new AQLMathVolFuncLMM(*this);
    }
    catch (bad_alloc &e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief check function for this class ID
    @param[in] id ID to check
    @return true or false
*/
bool
AQLMathVolFuncLMM::isTypeOf(function_t id) const
{
	return (id == FN_VOLFUNCLMM ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
    @brief return this function type
    @return function type
*/
function_t
AQLMathVolFuncLMM::getType() const
{
	return FN_VOLFUNCLMM;
}


/*!
    @brief return function value
	@param[in] x point
    @return function value
*/
double
AQLMathVolFuncLMM::operator()(const DoubleArray& x) const
{
	if (static_cast<int>(x.size()) < 1)
	{
		throw AQLCoreInvalidData("The argument DoubleArray's size must be more than zero !", __FILE__, __LINE__);
	}
	return operator()(x[0]);
}


/*!
    @brief return function value
	@param[in] t term
    @return function value
*/
double
AQLMathVolFuncLMM::operator()(double t) const
{
	if ((t < 0.0) || (m_t.size() <= 0) || 
			(t < mTFix.front()) || (t > mTFix.back()) || !mpFuncTerm || !mpFuncTenor)
	{
		return 0.0;
	}

	double tenor = 0.0;
	double t_ = 0.0;
	if (t != mTFix.front() && t != mTFix.back())
	{
		// search calc index
		const unsigned int indx = static_cast<unsigned int>
							(--upper_bound(mTFix.begin(), mTFix.end(), t) - mTFix.begin());
		tenor = m_T - m_t[indx];
		t_ = m_t[indx];
	}
	else if (t == mTFix.front())
	{
		tenor = m_T - m_t.front();
		t_ = m_t.front();
	}
	else
	{
		tenor = m_T - m_t.back();
		t_ = m_t.back();
	}

	const double vol = mpFuncTerm->operator()(t_) * mpFuncTenor->operator()(tenor) * mMultiple;	
	if (vol > 0.0)
	{
		return vol;
	}
	else
	{
		//return -vol;
		return 0.0;
	}

}
