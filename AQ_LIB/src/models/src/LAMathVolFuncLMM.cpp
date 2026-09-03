/*! @file
    @brief Source code of class to represent LMM volatility function

	This class derives from LAFunctionBase

*/
//  2007, Mizuho International London.

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAMathVolFuncLMM.h
//
//  SYNOPSIS    :       LAMathVolFuncLMM
//  DESCRIPTION :       Source code of class  to represent volatility of LMM
//						This class derives from LAFunctionBase
//                      
//  VERSION		:
////X///////////////////X///////////////////////////////X///////////////////

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LAMathVolFuncLMM.h"


using namespace std;

//================ LAMathVolFuncLMM ===================================
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

LAMathVolFuncLMM::LAMathVolFuncLMM(const LAFunctionBase *funcTenor, const LAFunctionBase *funcTerm, 
				const DoubleArray &TFix, const DoubleArray &TFix_30_360, int suffix, double multiple, bool delFlg)
: LAFunctionBase(), mpFuncTenor(funcTenor), mpFuncTerm(funcTerm), mTFix(TFix), 
  mTFix_30_360(TFix_30_360), m_t(DoubleArray()), m_T(0.0), mMultiple(multiple), mDelFlg(delFlg)
{
	// data size check
	const int dataSize = TFix.size();
	if (dataSize != static_cast<int>(TFix_30_360.size()))
	{
		throw LACoreInvalidData("data size must be same !! ", __FILE__, __LINE__);
	}
	// suffix check
	if (dataSize <= suffix)
	{
		throw LACoreInvalidData("suffix must be less than data size  !! ", __FILE__, __LINE__);
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
LAMathVolFuncLMM::~LAMathVolFuncLMM(void)
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
LAMathVolFuncLMM::LAMathVolFuncLMM(const LAMathVolFuncLMM &rhs) 
: LAFunctionBase(), mpFuncTenor(0), mpFuncTerm(0), mTFix(rhs.mTFix), 
  mTFix_30_360(rhs.mTFix_30_360), m_t(rhs.m_t), m_T(rhs.m_T), mMultiple(rhs.mMultiple), mDelFlg(rhs.mDelFlg)
{
	if (mDelFlg)
	{
		// pointer owner
		if (rhs.mpFuncTenor)
		{
			mpFuncTenor = dynamic_cast<const LAFunctionBase *>(rhs.mpFuncTenor->clone());
		}
		if (rhs.mpFuncTerm)
		{
			mpFuncTerm = dynamic_cast<const LAFunctionBase *>(rhs.mpFuncTerm->clone());
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
LACoreFunctionBase*	
LAMathVolFuncLMM::clone() const
{
    try 
	{
		return new LAMathVolFuncLMM(*this);
    }
    catch (bad_alloc &e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief check function for this class ID
    @param[in] id ID to check
    @return true or false
*/
bool
LAMathVolFuncLMM::isTypeOf(function_t id) const
{
	return (id == FN_VOLFUNCLMM ? true : LAFunctionBase::isTypeOf(id));
}

/*!
    @brief return this function type
    @return function type
*/
function_t
LAMathVolFuncLMM::getType() const
{
	return FN_VOLFUNCLMM;
}


/*!
    @brief return function value
	@param[in] x point
    @return function value
*/
double
LAMathVolFuncLMM::operator()(const DoubleArray& x) const
{
	if (static_cast<int>(x.size()) < 1)
	{
		throw LACoreInvalidData("The argument DoubleArray's size must be more than zero !", __FILE__, __LINE__);
	}
	return operator()(x[0]);
}


/*!
    @brief return function value
	@param[in] t term
    @return function value
*/
double
LAMathVolFuncLMM::operator()(double t) const
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
