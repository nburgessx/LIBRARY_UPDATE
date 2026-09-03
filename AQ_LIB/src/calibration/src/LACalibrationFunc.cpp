/*! @file
    @brief Source code of calibration function

	This class derives from LAFunctionBase

*/
//  2008, AlgoQuantHub.

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       MACalibrationFunc.h
//
//  SYNOPSIS    :       MACalibrationFunc
//  DESCRIPTION :       Source code of calibration function
//						This class derives from LAFunctionBase
//                      
//  VERSION		:
////X///////////////////X///////////////////////////////X///////////////////

#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include <algorithm>
#include "LACalibrationFunc.h"
#include "LACoreAppError.h"
#include "LAMathVolFuncFXStrangleSolver.h"
#ifdef __HAS_MIC__

#endif

using namespace std;

//================ MACalibrationFunc ===================================
/*!
	@brief constructor
*/

MACalibrationFunc::MACalibrationFunc(void)
: LAFunctionBase(), mpRealFunc(0), mIsReady(false)
{
}

/*!
	@brief destructor
*/
MACalibrationFunc::~MACalibrationFunc(void)
{
	if (mpRealFunc)
	{
		delete mpRealFunc;
	}
}


/*!
	@brief copy constructor
*/
MACalibrationFunc::MACalibrationFunc(const MACalibrationFunc &rhs) 
: LAFunctionBase(),  mpRealFunc(0), mIsReady(rhs.mIsReady)
{
	if (rhs.mpRealFunc)
	{
		mpRealFunc = dynamic_cast<const LAFunctionBase *>(rhs.mpRealFunc->clone());
	}
}

/*!
    @brief Make copy(clone) of this class realfunction
    @return Deep copy of real function
*/
LACoreFunctionBase*	
MACalibrationFunc::clone() const
{
	try 
	{
		//cout << static_cast<int>(LACoreThread::getThreadID()) << " clone start. this = " << this << endl;
#ifdef __HAS_MIC__
		mMutex.lock();
#endif
		while (!mIsReady && !mpRealFunc)
		{
			//cout << static_cast<int>(LACoreThread::getThreadID()) << " wait.. this = " << this << endl;
#ifdef __HAS_MIC__
			mMutex.unlock();
#endif
			mEvent.wait();
#ifdef __HAS_MIC__
			mMutex.lock();
#endif
			//cout << static_cast<int>(LACoreThread::getThreadID()) << " wait end ! this = " << this << endl;
		}
		//return real method deep copy
		//cout << static_cast<int>(LACoreThread::getThreadID()) << " clone end. this = " << this << endl;
#ifdef __HAS_MIC__
		mMutex.unlock();
#endif
		return mpRealFunc->clone();
	}
	catch (bad_alloc &e)
	{
#ifdef __HAS_MIC__
		mMutex.unlock();
#endif
		throw LACoreSystemError(e.what(), __FILE__, __LINE__);
	}
}

/*!
    @brief check function for this class ID
    @param[in] id ID to check
    @return true or false
*/
bool
MACalibrationFunc::isTypeOf(function_t id) const
{
	return (id == FN_CALIBRATIONFUNC ? true : LAFunctionBase::isTypeOf(id));
}

/*!
    @brief return this function type
    @return function type
*/
function_t
MACalibrationFunc::getType() const
{
	return FN_CALIBRATIONFUNC;
}


/*!
    @brief return function value
	@param[in] x point
    @return function value
*/
double
MACalibrationFunc::operator()(const DoubleArray& x) const
{
	if (!mIsReady || !mpRealFunc)
	{
		throw LACoreInvalidData("This method is not ready. Real method may be NULL", __FILE__, __LINE__);
	}
	return mpRealFunc->operator ()(x);
}


/*!
    @brief return function value
	@param[in] t term
    @return function value
*/
double
MACalibrationFunc::operator()(double t) const
{
	if (!mIsReady || !mpRealFunc)
	{
		throw LACoreInvalidData("This method is not ready. Real method may be NULL", __FILE__, __LINE__);
	}
	return mpRealFunc->operator ()(t);
}

/*!
    @brief set real function
	@param[in] method
*/
void
MACalibrationFunc::setRealFunction(const LAFunctionBase &method)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::Mutex> lock(mMutex);
#endif
	if (mpRealFunc)
	{
		delete mpRealFunc;
	}
	
	mpRealFunc = dynamic_cast<const LAFunctionBase *>(method.clone());
}

/*!
    @brief set on
*/
void
MACalibrationFunc::setOn()
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::Mutex> lock(mMutex);
#endif
	//cout << static_cast<int>(LACoreThread::getThreadID()) << " SetOn called. this = " << this << endl;
	
	if (!mpRealFunc)
	{
		throw LACoreInvalidData("real function is not set yet.", __FILE__, __LINE__);
	}
	mIsReady = true;
	mEvent.notifyAll();

	//cout << static_cast<int>(LACoreThread::getThreadID()) << " SetOn end. this = " << this << endl;
}
