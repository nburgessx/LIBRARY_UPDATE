/*! @file
    @brief Source code of bank account numeraire class
*/

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLRatesNumeraireBankAccountHW.h"
#include "AQLModelDynamicsCurve.h"
#include "AQLRatesSDEBase.h"
#include "AQLAlgorithm.h"
#include "AQLBasic.h"
#include "AQLLinearInterpolation.h"
#include <algorithm>

using namespace std;

const double INFINITESIMAL = 1E-7; 

//================ AQLRatesNumeraireBankAccountHW ===================================
/*!
	@brief default constructor
	@param[in] T payment time of last forward libor 
*/
AQLRatesNumeraireBankAccountHW::AQLRatesNumeraireBankAccountHW() 
: AQLRatesNumeraireBase(true), mUpdateAdjValFlag(true), mCalcStartTime(0.0)
{

}
/*!
	@brief  constructor
	@param[in] timeGrid timeGrid for basis value
	@param[in] basis vector of basis value
*/
AQLRatesNumeraireBankAccountHW::AQLRatesNumeraireBankAccountHW(const AQLString& basisName, const DoubleArray& timeGrid, const DoubleArray& basis)
: AQLRatesNumeraireBase(basisName, timeGrid, basis, true), mUpdateAdjValFlag(true), mCalcStartTime(0.0)
{
	mpCurveWithBasis = getCurveWithBasis();
	mpCurveWithBasis->setBasisCurve(mpBasisCurve);
}
/*!
	@brief copy constructor
*/
/*AQLRatesNumeraireBankAccountHW::AQLRatesNumeraireBankAccountHW(const AQLRatesNumeraireBankAccountHW& v) 
: AQLRatesNumeraireBase(v)
{

}*/

/*!
	@brief destructor
*/
AQLRatesNumeraireBankAccountHW::~AQLRatesNumeraireBankAccountHW() 
{

}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLRatesNumeraireBankAccountHW::clone() const
{
    try 
	{
		return new AQLRatesNumeraireBankAccountHW(*this);
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
AQLRatesNumeraireBankAccountHW::isTypeOf(function_t id) const
{
	return (id==FN_NUMERAIREBANKACCOUNTHW ? true : AQLRatesNumeraireBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLRatesNumeraireBankAccountHW::getType() const
{
	return FN_NUMERAIREBANKACCOUNTHW;
}


/*!
	@brief get numeraire value at t
	@param[in] t time
	@return numeraire value
*/
double
AQLRatesNumeraireBankAccountHW::operator()(double t) const
{
	if (mUpdateFlag) calcNumeraire();

	if (t > mTimeGrid.back() + INFINITESIMAL || t < 0.0)
	{
		//error
        throw AQLCoreInvalidData("input t is before 0 or after Terminal", __FILE__, __LINE__);
	}

	unsigned int pos;
	AQLAlgorithm::locate<DoubleArray, double>(mTimeGrid, t, mTimeGrid.size(), pos);
	
	if (pos == mTimeGrid.size())
		return mNumeraireArray.back();
	else if (mTimeGrid[pos] == t)
		return mNumeraireArray[pos];
	else
        {
        	
		return mNumeraireArray[pos - 1] *
				AQLMath::exp(
				mSpotArray[pos - 1] * (t - mTimeGrid[pos - 1])
				+ 0.5 * (mSpotArray[pos] - mSpotArray[pos - 1]) * (t - mTimeGrid[pos - 1]) *  (t - mTimeGrid[pos - 1])  / (mTimeGrid[pos] - mTimeGrid[pos - 1])
				- (mInitialSpotArray[pos - 1] * (t - mTimeGrid[pos - 1])
				+ 0.5 * (mInitialSpotArray[pos] - mInitialSpotArray[pos - 1]) * (t - mTimeGrid[pos - 1]) *  (t - mTimeGrid[pos - 1])  / (mTimeGrid[pos] - mTimeGrid[pos - 1]))	
				)
				* mpInitialCurve->getP(mTimeGrid[pos - 1]) / mpInitialCurve->getP(t);
		}
	
	
	
	/*{
		const AQLRatesPathElementCurve& curve = getCurve(mTimeGrid[pos - 1]);		
		return mNumeraireArray[pos - 1] *
			AQLMath::exp(-AQLMath::log(curve.getP(t)) + (mSpotArray[pos] - curve.getF(mTimeGrid[pos])) * 0.5 *  (t - mTimeGrid[pos - 1]) *  (t - mTimeGrid[pos - 1]) / (mTimeGrid[pos] - mTimeGrid[pos - 1]));
	}*/
}

/*!
	@brief set curve

	@param[in] t time
	@return pcurve curve object
									
	@note this class is not pointer owner of the of curve 
*/
void
AQLRatesNumeraireBankAccountHW::setCurve(double t, const AQLRatesPathElementCurve* pcurve)
{
	AQLRatesNumeraireBase::setCurve(t, pcurve);
	if (mCalcStartTime > t)
	{
		mCalcStartTime = t;
	}
	//mUpdateFlag = true;
}

//======================================
// calc nuemraire	
void
AQLRatesNumeraireBankAccountHW::calcNumeraire(void) const 
{		
	unsigned int size = mCurves.size();
	mNumeraireArray.resize(size);
	mSpotArray.resize(size);
	mTimeGrid.resize(size);
	//mAdjVal.resize(size - 1);
	// update adjval
	if (mAdjVal.size() < size - 1)
	{
		mUpdateAdjValFlag = true;
	}

	map<double, const AQLRatesPathElementCurve*>::const_iterator it = mCurves.begin();
	const AQLRatesPathElementCurve* pCurve;
	for (unsigned int i = 0; i < size; i++, it++)
	{
		mTimeGrid[i] = it->first;
		if (mCalcStartTime > mTimeGrid[i])
		{
			continue;
		}
		mSpotArray[i] = it->second->get()[0];
		//with basis curve
		if (mpCurveWithBasis != 0)
		{
			if (!is_cache[mTimeGrid[i]])
			{

				if(0==i)
				{
					cache[mTimeGrid[0]] = 0.0;
				}
				else if(1 == i)
				{
					double deltat = mTimeGrid[i]-mTimeGrid[i-1];
					cache[mTimeGrid[i]] = 1.0/deltat *( (*mpBasisCurve)(mTimeGrid[i])*mTimeGrid[i] - (*mpBasisCurve)(mTimeGrid[i-1])*mTimeGrid[i-1] );
					cache[mTimeGrid[0]] = cache[mTimeGrid[i]];					
					mSpotArray[0]+=cache[mTimeGrid[0]];
				}
				else
				{
					double deltat = mTimeGrid[i]-mTimeGrid[i-1];
					cache[mTimeGrid[i]] = 2.0/deltat *( (*mpBasisCurve)(mTimeGrid[i])*mTimeGrid[i] - (*mpBasisCurve)(mTimeGrid[i-1])*mTimeGrid[i-1] ) - cache[mTimeGrid[i-1]];
				}
				is_cache[mTimeGrid[i]] = true;

			}
			mSpotArray[i] += cache[mTimeGrid[i]];
		}		
		
		if (i == 0) mNumeraireArray[i] = 1.0;
		else
		{
			if (mUpdateAdjValFlag)
			{
				mNumeraireArray[i] = mNumeraireArray[i - 1] * 				
					AQLMath::exp((mSpotArray[i - 1] + mSpotArray[i]) * 0.5 * (mTimeGrid[i] - mTimeGrid[i - 1]));
			}
			else
			{
				mNumeraireArray[i] = mNumeraireArray[i - 1] * 				
					AQLMath::exp((mSpotArray[i - 1] + mSpotArray[i]) * 0.5 * (mTimeGrid[i] - mTimeGrid[i - 1]) + mAdjVal[i - 1]);
			}
		}
		pCurve = it->second;
		
	}
	if (mCalcStartTime < mTimeGrid.back())
	{
		mCalcStartTime = mTimeGrid.back();
	}
	if (mUpdateAdjValFlag) setAdjVal();
	mUpdateFlag = false;
}

//==========================================
void
AQLRatesNumeraireBankAccountHW::setSDE(AQLRatesSDEBase* pSDE)
{
	mUpdateAdjValFlag = true;

	AQLRatesNumeraireBase::setSDE(pSDE);

	//initial curve
	mpInitialCurve = dynamic_cast<const AQLRatesPathElementCurve*>(pSDE->getInitialValue());

	if (mpCurveWithBasis != 0)
	{
		dynamic_cast<AQLRatesNumeraireBankAccountHW::AQLRatesCurveWithBasisHW*>(mpCurveWithBasis)
			->setBasisForwardCurve(mpSDE->getBM()->getTimeGrid());
	}
}

AQLRatesNumeraireBankAccountHW::AQLRatesCurveWithBasis*
AQLRatesNumeraireBankAccountHW::getCurveWithBasis() const
{
	try 
	{
		return new AQLRatesCurveWithBasisHW();
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

AQLRatesPathElementBase*
AQLRatesNumeraireBankAccountHW::AQLRatesCurveWithBasisHW::clone() const
{
	try 
	{
		return new AQLRatesCurveWithBasisHW();
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }	
}

// setUp Adjust Value of Numeraire
void
AQLRatesNumeraireBankAccountHW::setAdjVal() const
{
	unsigned int size = mTimeGrid.size();
	unsigned int size_a = mAdjVal.size();
	mInitialSpotArray.resize(size);
	mAdjVal.resize(size - 1);
	if (size_a == 0)
	{
		mInitialSpotArray[0] = mpInitialCurve->getF(mTimeGrid[0]);
	}
	double adj = 1.0;
	//for (unsigned int i = 1; i < size; i++)
	for (unsigned int i = 1; i <= size_a; i++)
	{
		adj *= AQLMath::exp(mAdjVal[i - 1]);
		mNumeraireArray[i] *= adj;
	}
	for (unsigned int i = size_a + 1; i < size; i++)
	{
		mInitialSpotArray[i] = mpInitialCurve->getF(mTimeGrid[i]);
		mAdjVal[i - 1] = AQLMath::log(mpInitialCurve->getP(mTimeGrid[i - 1]) / mpInitialCurve->getP(mTimeGrid[i]))
			- (mInitialSpotArray[i - 1] + mInitialSpotArray[i]) * 0.5 * (mTimeGrid[i] - mTimeGrid[i - 1]);
		adj *= AQLMath::exp(mAdjVal[i - 1]);
		mNumeraireArray[i] *= adj;
	}
	mUpdateAdjValFlag = false;
}


double
AQLRatesNumeraireBankAccountHW::AQLRatesCurveWithBasisHW::getF(double T, double dt) const
{
	return mpCurve->getF(T, dt) + mBasisForwardCurve(T);
}

// setup basis forward curve
void
AQLRatesNumeraireBankAccountHW::AQLRatesCurveWithBasisHW::setBasisForwardCurve(const DoubleArray& timeGrid)
{
	unsigned int size = timeGrid.size();
	DoubleArray data(size,0.0);
	for (unsigned int i = 1; i < size; i++)
	{
		double deltat = timeGrid[i] - timeGrid[i-1];
		if (i == 1)
		{
			data[1] = 1.0 / deltat * ((*mpBasisCurve)(timeGrid[i]) * timeGrid[i] - (*mpBasisCurve)(timeGrid[i - 1]) * timeGrid[i - 1] );
			data[0] = data[1];
		}
		else
			data[i] = 2.0 / deltat * ((*mpBasisCurve)(timeGrid[i]) * timeGrid[i] - (*mpBasisCurve)(timeGrid[i - 1]) * timeGrid[i - 1] ) - data[i - 1];
	}
	mBasisForwardCurve.set(timeGrid, data);
	AQLLinearInterpolation inter;
	mBasisForwardCurve.setInterpolation(inter);
}

// set basis spread
/*!
	@param[in] basis name
	@param[in] time grid
	@param[in] basis

*/	
void
AQLRatesNumeraireBankAccountHW::setBasisSpread(const AQLString& basisName, const DoubleArray& timeGrid, const DoubleArray& basis)
{
	AQLRatesNumeraireBase::setBasisSpread(basisName, timeGrid, basis);
	if (mpCurveWithBasis && mpSDE)
	{
		dynamic_cast<AQLRatesNumeraireBankAccountHW::AQLRatesCurveWithBasisHW*>(mpCurveWithBasis)
			->setBasisForwardCurve(mpSDE->getBM()->getTimeGrid());
	}

	is_cache.clear();
	cache.clear();
	mCalcStartTime = 0.0;
	mUpdateFlag = true;
	mUpdateAdjValFlag = true;
	mAdjVal.clear();
}

// set basis spread
/*!
	@param[in] basis name
	@param[in] time grid
	@param[in] basis

*/	
void
AQLRatesNumeraireBankAccountHW::setBasisName(const AQLString& basisName)
{
	if (basisName != mBasisName)
	{
		AQLRatesNumeraireBase::setBasisName(basisName);
		if (mpCurveWithBasis && mpSDE)
		{
			dynamic_cast<AQLRatesNumeraireBankAccountHW::AQLRatesCurveWithBasisHW*>(mpCurveWithBasis)
				->setBasisForwardCurve(mpSDE->getBM()->getTimeGrid());
		}

		is_cache.clear();
		cache.clear();
		mCalcStartTime = 0.0;
		mUpdateFlag = true;
		mUpdateAdjValFlag = true;
		mAdjVal.clear();
	}
}

/*!
    @brief reset
*/
void
AQLRatesNumeraireBankAccountHW::reset()
{
	AQLRatesNumeraireBase::reset();
	mCalcStartTime = 0.0;
	is_cache.clear();
	cache.clear();
	//mUpdateFlag = true;
	mUpdateAdjValFlag = true;
	mAdjVal.clear();
}