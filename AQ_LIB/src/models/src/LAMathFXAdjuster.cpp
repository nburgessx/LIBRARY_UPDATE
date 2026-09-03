/*! @file
    @brief Source code of class to adjust FX value

	This class derives from LAFunctionBase

*/
//  2007, Mizuho International London.

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAMathFXAdjuster.h
//
//  SYNOPSIS    :       LAMathFXAdjuster
//  DESCRIPTION :       Source code of class  to adjust FX value
//						This class derives from LAFunctionBase
//                      
//  VERSION		:
////X///////////////////X///////////////////////////////X///////////////////

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LAMathFXAdjuster.h"


using namespace std;

//================ LAMathFXAdjuster ===================================
/*!
	@brief constructor

	@param[in] maxFXMultiplier  max fx multiplier
	@param[in] volFX           fx volatility function  
	@param[in] delFlg          pointer owner flag
	@param[in] exAdjFlg        extra adjustment flag 

*/
LAMathFXAdjuster::LAMathFXAdjuster(double maxFXMultiplier, const LAMathVolFuncFX *pVolFX, bool delFlg, bool exAdjFlg) 
: LAFunctionBase(), mMaxFXMultiplier(maxFXMultiplier), mpVolFX(pVolFX), mDelFlg(delFlg), mExAdjFlg(exAdjFlg),
mpPath(0), mpNume(0)
{

}

/*!
	@brief destructor
*/
LAMathFXAdjuster::~LAMathFXAdjuster(void)
{
	if (mDelFlg)
	{
		delete mpVolFX;
	}
}


/*!
	@brief copy constructor
*/
LAMathFXAdjuster::LAMathFXAdjuster(const LAMathFXAdjuster &rhs) 
: LAFunctionBase(), mMaxFXMultiplier(rhs.mMaxFXMultiplier), mpVolFX(rhs.mpVolFX), mDelFlg(rhs.mDelFlg), mExAdjFlg(rhs.mExAdjFlg),
mpPath(rhs.mpPath), mpNume(rhs.mpNume), mFXCorrectnum(rhs.mFXCorrectnum), mFXCorrectden(rhs.mFXCorrectden)
{
	if (mDelFlg)
	{
		mpVolFX = dynamic_cast<const LAMathVolFuncFX *>(rhs.mpVolFX->clone());
	}
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LAMathFXAdjuster::clone() const
{
    try 
	{
		return new LAMathFXAdjuster(*this);
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
LAMathFXAdjuster::isTypeOf(function_t id) const
{
	return (id == FN_FXADJUSTER ? true : LAFunctionBase::isTypeOf(id));
}

/*!
    @brief return this function type
    @return function type
*/
function_t
LAMathFXAdjuster::getType() const
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
LAMathFXAdjuster::operator()(const DoubleArray& x) const
{
	if (x.size() < 2)
	{
		throw LACoreInvalidData("argument size is less than two ", __FILE__, __LINE__);
	}

	if (mpVolFX)
	{
		unsigned int idx = mpVolFX->searchIndex(x[0]);
		double scaler = 1.0 + mMaxFXMultiplier * mpVolFX->getSigma()[idx] * LAMath::sqrt(x[0]);
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
			val = LAMath::min(maxSpotFX, 2.0 * minSpotFX - x[1]);
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
				const LARatesSDEBase &sde = dynamic_cast<const LAMathAttrSDE &>
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
				return LAMath::min(maxSpotFX, 2.0 * minSpotFX - val);
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
void LAMathFXAdjuster::setPathEntity(const LAMathPathEntity &path)
{
	mpPath = &path;
	mFXCorrectnum.clear();
	mFXCorrectden.clear();
}