/*! @file
    @brief Source code of class to adjust FX value

	This class derives from AQLFunctionBase
*/


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLMathFXAdjuster.h"


using namespace std;

//================ AQLMathFXAdjuster ===================================
/*!
	@brief constructor

	@param[in] maxFXMultiplier  max fx multiplier
	@param[in] volFX           fx volatility function  
	@param[in] delFlg          pointer owner flag
	@param[in] exAdjFlg        extra adjustment flag 

*/
AQLMathFXAdjuster::AQLMathFXAdjuster(double maxFXMultiplier, const AQLMathVolFuncFX *pVolFX, bool delFlg, bool exAdjFlg) 
: AQLFunctionBase(), mMaxFXMultiplier(maxFXMultiplier), mpVolFX(pVolFX), mDelFlg(delFlg), mExAdjFlg(exAdjFlg),
mpPath(0), mpNume(0)
{

}

/*!
	@brief destructor
*/
AQLMathFXAdjuster::~AQLMathFXAdjuster(void)
{
	if (mDelFlg)
	{
		delete mpVolFX;
	}
}


/*!
	@brief copy constructor
*/
AQLMathFXAdjuster::AQLMathFXAdjuster(const AQLMathFXAdjuster &rhs) 
: AQLFunctionBase(), mMaxFXMultiplier(rhs.mMaxFXMultiplier), mpVolFX(rhs.mpVolFX), mDelFlg(rhs.mDelFlg), mExAdjFlg(rhs.mExAdjFlg),
mpPath(rhs.mpPath), mpNume(rhs.mpNume), mFXCorrectnum(rhs.mFXCorrectnum), mFXCorrectden(rhs.mFXCorrectden)
{
	if (mDelFlg)
	{
		mpVolFX = dynamic_cast<const AQLMathVolFuncFX *>(rhs.mpVolFX->clone());
	}
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLMathFXAdjuster::clone() const
{
    try 
	{
		return new AQLMathFXAdjuster(*this);
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
AQLMathFXAdjuster::isTypeOf(function_t id) const
{
	return (id == FN_FXADJUSTER ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
    @brief return this function type
    @return function type
*/
function_t
AQLMathFXAdjuster::getType() const
{
	return FN_FXADJUSTER;
}


/*!
    @brief return function value
	@param[in] x point
    @return function value

	x[1] is fx value
*/
double
AQLMathFXAdjuster::operator()(const DoubleArray& x) const
{
	if (x.size() < 2)
	{
		throw AQLCoreInvalidData("argument size is less than two ", __FILE__, __LINE__);
	}

	if (mpVolFX)
	{
		unsigned int idx = mpVolFX->searchIndex(x[0]);
		double scaler = 1.0 + mMaxFXMultiplier * mpVolFX->getSigma()[idx] * AQLMath::sqrt(x[0]);
		double fx0 = mpVolFX->getForwardFX0()[idx];

		double maxSpotFX = fx0 * scaler;
		double minSpotFX = fx0 / scaler;

		double val = 0.0;

		// adjust fx
		if (maxSpotFX >= x[1] && minSpotFX <= x[1])
		{
			// no need adjustment
			val = x[1];
		}
		else if(maxSpotFX < x[1])
		{
			val = maxSpotFX;
		}
		else
		{
			val = AQLMath::min(maxSpotFX, 2.0 * minSpotFX - x[1]);
		}

		// check extra adjustment
		if (!mExAdjFlg)
		{
			return val;
		}
		else
		{
			if (!mpNume)
			{
				const AQLRatesSDEBase &sde = dynamic_cast<const AQLMathAttrSDE &>
												(mpPath->getData(mpPath->getSDEAttrNames().get()[0], ISNOTNULL).get()).getSDE();
				mpNume = sde.getNumeraire();
			}
			// calc domestic df from numeraire
			double adjDF = 1.0 / mpNume->operator ()(x[0]);
			// do extra adjustment
			mFXCorrectnum[x[0]] += adjDF;
			mFXCorrectden[x[0]] += adjDF * val / fx0;
			double adjFactor = mFXCorrectnum[x[0]] / mFXCorrectden[x[0]];
			val *= adjFactor;

			// check max and min
			if (maxSpotFX >= val && minSpotFX <= val)
			{
				// no need adjustment
				return val;
			}
			else if(maxSpotFX < val)
			{
				return maxSpotFX;
			}
			else
			{
				return AQLMath::min(maxSpotFX, 2.0 * minSpotFX - val);
			}
		}
	}
	else
	{
		return x[1];
	}
}

/*!
    @brief set PathEntity
	@param[in] path

*/
void AQLMathFXAdjuster::setPathEntity(const AQLMathPathEntity &path)
{
	mpPath = &path;
	mFXCorrectnum.clear();
	mFXCorrectden.clear();
}