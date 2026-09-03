/*! @file
    @brief Source code of base class of numeraire class



*/
//  2007, AlgoQuantHub..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLRatesNumeraireBase.cpp
//
//  SYNOPSIS    :       AQLRatesNumeraireBase
//  DESCRIPTION :       Declaration of base class of numeraire class
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


#include "AQLRatesNumeraireBase.h"
#include "AQLRatesPEInterpolationBase.h"
#include "AQLModelDynamicsCurve.h"
#include "AQL1DDataSet.h"
#include "AQLSplineInterpolation.h"
#include "AQLLinearInterpolation.h"
#include "AQLRatesSDEBase.h"

#include <limits>

//#ifdef _MSC_VER 	//20070409--Nagase--g++(g++stdext)
//using namespace stdext;
//#else
using namespace std;
//#endif
const double INFINITESIMAL = 1E-7;

//================ AQLRatesNumeraireBase ===================================
/*!
	@brief default constructor
	@param[in] isStochasticIR interest rate is stochastic or not
*/
AQLRatesNumeraireBase::AQLRatesNumeraireBase(bool isStochasticIR) 
: mpInter(0), mIsStochastic(isStochasticIR), mIsLongJump(false), mpBasisCurve(0), mpCurveWithBasis(0), mpSDE(0), 
mUpdateFlag(true), mBasisName(NOBASIS)
{
	mLastCurve.set_t(std::numeric_limits<double>::max());	
}
/*!
	@brief  constructor
	@param[in] basis name
	@param[in] timeGrid timeGrid for basis value
	@param[in] basis vector of basis value
	@param[in] isStochasticIR interest rate is stochastic or not
*/
AQLRatesNumeraireBase::AQLRatesNumeraireBase(const AQLString& basisName, const DoubleArray& timeGrid, const DoubleArray& basis, bool isStochasticIR)
: mpInter(0), mIsStochastic(isStochasticIR), mIsLongJump(false), mpBasisCurve(0), mpCurveWithBasis(0), mpSDE(0), 
mUpdateFlag(true), mBasisName(basisName)
{
	if (timeGrid.size() != basis.size())
	{
		//error
		throw AQLCoreInvalidData("timeGrid must be same size as basis size", __FILE__, __LINE__);
	}	
	try 
	{
		mpBasisCurve = new AQL1DDataSet();
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
	dynamic_cast<AQL1DDataSet*>(mpBasisCurve)->set(timeGrid, basis);
	//AQLSplineInterpolation inter;
	AQLLinearInterpolation inter;
	dynamic_cast<AQL1DDataSet*>(mpBasisCurve)->setInterpolation(inter);

	mBasisCurveMap[basisName] = mpBasisCurve;
}
/*!
	@brief copy constructor
*/
AQLRatesNumeraireBase::AQLRatesNumeraireBase(const AQLRatesNumeraireBase& v) 
: AQLCoreFunctionBase(v)
, mCurves(v.mCurves), mpInter(v.mpInter), mIsStochastic(v.mIsStochastic), mLastCurve(v.mLastCurve),
mIsLongJump(v.mIsLongJump), mpBasisCurve(0), mpCurveWithBasis(0), mpSDE(0), mUpdateFlag(true), 
mBasisName(v.mBasisName)
{
	if (v.mpBasisCurve != 0) mpBasisCurve = dynamic_cast<AQLFunctionBase*>(v.mpBasisCurve->clone());
	if (v.mpCurveWithBasis != 0)
	{
		mpCurveWithBasis = dynamic_cast<AQLRatesCurveWithBasis*>(v.mpCurveWithBasis->clone());
		mpCurveWithBasis->setBasisCurve(mpBasisCurve);
	}
	if (v.mpSDE != 0)
		mpSDE = dynamic_cast<AQLRatesSDEBase*>(v.mpSDE->clone());
	mBasisCurveMap[mBasisName] = mpBasisCurve;
}

/*!
	@brief destructor
*/
AQLRatesNumeraireBase::~AQLRatesNumeraireBase() 
{
	reset();
}

/*!
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLRatesNumeraireBase::isTypeOf(function_t id) const
{
	return (id==FN_NUMERAIREBASE ? true : AQLCoreFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLRatesNumeraireBase::getType() const
{
	return FN_NUMERAIREBASE;
}

/*!
    @brief get curve at t
	@param[in] t time
    @return curve
*/
const AQLRatesPathElementCurve&
AQLRatesNumeraireBase::getCurve(double t) const
{
	if (mpCurveWithBasis == 0) return getCurveWithoutBasis(t);
	else
	{
		const AQLRatesPathElementCurve& curve = getCurveWithoutBasis(t);
		mpCurveWithBasis->setBaseCurve(&curve);
		return *mpCurveWithBasis;
	}
}
/*!
    @brief get initial curve
    @return initial curve
*/
const AQLRatesPathElementCurve&	AQLRatesNumeraireBase::getInitialCurve() const
{
	return *dynamic_cast<const AQLRatesPathElementCurve*>(mpSDE->getInitialValue());
}
/*!
    @brief get curve at t
	@param[in] t time
    @return curve
*/
const AQLRatesPathElementCurve&
AQLRatesNumeraireBase::getCurveWithoutBasis(double t) const
{
	map<double, const AQLRatesPathElementCurve*>::const_iterator it;
	it = mCurves.lower_bound(t - INFINITESIMAL);
	if (it == mCurves.end())
	{
		if (mpInter == NULL)
		{
		      throw AQLCoreInvalidData("interpolation is not setted", __FILE__, __LINE__);
		}		
		map<double, const AQLRatesPathElementCurve*>::const_iterator it2 = mCurves.end();
		it2--;
		return dynamic_cast<const AQLRatesPathElementCurve&>
				(mpInter->value(t, it2->first, mLastCurve.get_t(), *it2->second, mLastCurve));
	}
	else if (/*it->first == t*/ it->first >= t - INFINITESIMAL && it->first <= t + INFINITESIMAL)
		return *it->second;
	else if (it == mCurves.begin())
	{
		//error
        throw AQLCoreInvalidData("input t is before first curve start time", __FILE__, __LINE__);
	}
	else
	{

		if (mpInter == NULL)
		{
		      throw AQLCoreInvalidData("interpolation is not setted", __FILE__, __LINE__);
		}
		map<double, const AQLRatesPathElementCurve*>::const_iterator it2 = it;
		it2--;
		return dynamic_cast<const AQLRatesPathElementCurve&>
					(mpInter->value(t, it2->first, it->first, *it2->second, *it->second));

	}

}

/*!
    @brief reset
*/
void
AQLRatesNumeraireBase::reset()
{
	/*if (mpBasisCurve)
	{
		delete mpBasisCurve;
		mpBasisCurve = 0;
	}*/
	for (std::map<AQLString, AQLFunctionBase*>::iterator it = mBasisCurveMap.begin(); it != mBasisCurveMap.end(); ++it)
	{
		delete it->second;
	}
	mBasisCurveMap.clear();
	mpBasisCurve = 0;

	if (mpCurveWithBasis) 
	{
		delete mpCurveWithBasis;
		mpCurveWithBasis = 0;
	}
	clear();

	mUpdateFlag = true;
}

/*!
    @brief clear data
*/
void
AQLRatesNumeraireBase::clear()
{
	mCurves.clear();
	mUpdateFlag = true;
}

//20070406--Nagase--g++(cpp)
//======================================
// set curve
/*!
	@param[in] t time
	@return pcurve curve object
	@note this class is not pointer owner of the of curve 
*/
void
AQLRatesNumeraireBase::setCurve(double t, const AQLRatesPathElementCurve* pcurve)
{
	mCurves[t] = pcurve;
	mUpdateFlag = true;
}


//20070406--Nagase--g++(cpp)
//==========================================
// set interpolation class
/*!
	@param[in] pinter pointer to interpolation class
	@note this class is not pointer owner of the interpolation class
*/	
void
AQLRatesNumeraireBase::setInterpolationMethod(AQLRatesPEInterpolationBase* pinter)
{
	mpInter = pinter;
}


AQLRatesNumeraireBase::AQLRatesCurveWithBasis*
AQLRatesNumeraireBase::getCurveWithBasis() const
{
	try 
	{
		return new AQLRatesCurveWithBasis();
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}


/*!
    @brief get discount bond price
	@param[in] T maturity
    @return discount bond price
*/
double
AQLRatesNumeraireBase::AQLRatesCurveForNumeraire::getP (double T) const
{
	if (m_t != T)
	{
		//error
		throw AQLCoreInvalidData("curve start time must be same as T", __FILE__, __LINE__);
	}
	return 1.0;
}


AQLRatesPathElementBase*
AQLRatesNumeraireBase::AQLRatesCurveWithBasis::clone() const
{
	try 
	{
		return new AQLRatesCurveWithBasis();
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }	
}

// set basis spread
/*!
	@param[in] basis name
	@param[in] time grid
	@param[in] basis

*/	
void
AQLRatesNumeraireBase::setBasisSpread(const AQLString& basisName, const DoubleArray& timeGrid, const DoubleArray& basis)
{
	//if (mCancelSpread)
	//	return;

	// data size check
	if (timeGrid.size() != basis.size())
	{
		throw AQLCoreInvalidData("timeGrid must be same size as basis size", __FILE__, __LINE__);
	}	
	// delete for clear cache
	//if (mpBasisCurve)
	//{
	//	delete mpBasisCurve;
	//	mpBasisCurve = 0;
	//}
	if (mpCurveWithBasis) 
	{
		delete mpCurveWithBasis;
		mpCurveWithBasis = 0;
	}
	//delete data of basis name in basis curve map
	std::map<AQLString, AQLFunctionBase*>::iterator it = mBasisCurveMap.find(basisName);
	if (it != mBasisCurveMap.end())
	{
		delete it->second;
		mBasisCurveMap.erase(it);
	}
	mBasisName = basisName;

	try 
	{
		mpBasisCurve = new AQL1DDataSet();		
    }
    catch (bad_alloc &e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }

	//AQLSplineInterpolation inter;
	AQLLinearInterpolation inter;
	dynamic_cast<AQL1DDataSet*>(mpBasisCurve)->setInterpolation(inter);
	dynamic_cast<AQL1DDataSet*>(mpBasisCurve)->set(timeGrid, basis);

	mpCurveWithBasis = getCurveWithBasis();
	mpCurveWithBasis->setBasisCurve(mpBasisCurve);

	mBasisCurveMap[basisName] = mpBasisCurve;
}

// set basis name
/*!
	@param[in] basis name
*/	
void
AQLRatesNumeraireBase::setBasisName(const AQLString& basisName)
{
	if (basisName != mBasisName)
	{
		mBasisName = basisName;
		if (basisName == NOBASIS)
		{
			mpBasisCurve = 0;
			if (mpCurveWithBasis) 
			{
				delete mpCurveWithBasis;
				mpCurveWithBasis = 0;
			}
		}
		else
		{
			std::map<AQLString, AQLFunctionBase*>::iterator it = mBasisCurveMap.find(basisName);
			if (it == mBasisCurveMap.end())
			{
				//error
				AQLString errorMsg = basisName + " is not set in mBasisCurveMap!";
				throw AQLCoreInvalidData(errorMsg.getCString(), __FILE__, __LINE__);
			}
			mpBasisCurve = mBasisCurveMap[mBasisName];

			if (!mpCurveWithBasis) 
			{
				mpCurveWithBasis = getCurveWithBasis();
			}

			mpCurveWithBasis->setBasisCurve(mpBasisCurve);
		}
		mUpdateFlag = true;
	}
}

// get basis name
/*!
*/	
AQLString
AQLRatesNumeraireBase::getBasisName()
{
	return mBasisName;
}

void
AQLRatesNumeraireBase::setSDE(AQLRatesSDEBase* pSDE)
{
	mpSDE = pSDE;
	mUpdateFlag = true;
}

//void
//AQLRatesNumeraireBase::isCancelSpread(bool iscancel)
//{
//	mCancelSpread = iscancel;
//}
