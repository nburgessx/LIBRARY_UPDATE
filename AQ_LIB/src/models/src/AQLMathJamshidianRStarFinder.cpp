#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "AQLMathJamshidianRStarFinder.h"
#include "AQLObject.h"
#include "AQLDataProcedure.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataReference.h"
#include "AQLDataMultiReference.h"
#include "AQLDataInstance.h"
#include "AQLPriceDataManager.h"
#include "AQLObjectPool.h"
#include "AQLCoreTemplateType.h"
#include "AQLMathDefine.h"
#include "AQLPriceDataCalendar.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLPriceDataDayCount.h"
#include "AQLDataReference.h"
#include "AQLPriceDataFunction.h"
#include "AQLBasic.h"
#include "AQLAlgorithm.h"
#include "AQLModelDynamicsHW1FCurve.h"

using namespace std;
//////////////////AQLMathRstarFinder

AQLMathJamshidianRStarFinder::AQLMathJamshidianRStarFinder(double strike, double premium, const DoubleVector& grids, 
													  const DoubleVector& deltas, AQLRatesPathElementCurve* pcurve)
													  : AQLFunctionBase(), mStrike(strike),mPrem(premium),mGrids(grids),mDelta(deltas),is_cloned(false)
{
	mpCurve = pcurve;
}

AQLMathJamshidianRStarFinder::~AQLMathJamshidianRStarFinder()
{
	if(is_cloned)
	{
		delete mpCurve;
		mpCurve=0;
	}
	
}

/*!
	@brief copy constructor
*/
AQLMathJamshidianRStarFinder::AQLMathJamshidianRStarFinder(const AQLMathJamshidianRStarFinder &rhs) 
:
AQLFunctionBase(rhs),mStrike(rhs.mStrike),mPrem(rhs.mPrem),mGrids(rhs.mGrids),mDelta(rhs.mDelta),is_cloned(true),
mpCurve(rhs.mpCurve !=0 ? dynamic_cast<AQLRatesPathElementCurve*>(rhs.mpCurve->clone()) : 0)
{
}

/*!
    @brief Return this function type
    @return function type
*/

function_t
AQLMathJamshidianRStarFinder::getType() const
{
	return FN_JAMSHIDIANRSTARFINDER;
}

AQLCoreFunctionBase*
AQLMathJamshidianRStarFinder::clone() const
{
    try 
	{
		return new AQLMathJamshidianRStarFinder(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}


double
AQLMathJamshidianRStarFinder::operator()(const DoubleArray& x) const
{
	if(x.size() == 1)
		return operator()(x[0]);
	throw AQLCoreInvalidData("parameter size must be one", __FILE__, __LINE__);
}


double
AQLMathJamshidianRStarFinder::operator()(const double& x) const
{
	double r0 = x;
	dynamic_cast<AQLRatesPathElementBase *>(mpCurve)->set(r0);
	mpCurve->set_t(mGrids[0]);
	
	unsigned int N = mDelta.size();
	double annuity = 0.0;

	unsigned int i = 0;
	for(;i<N;i++)
	{
		annuity += mpCurve->getP(mGrids[i+1]) * mDelta[i]; 
	}
	double cpnP = mStrike * annuity + mpCurve->getP(mGrids[i]);

	double ret = 10000*(1.0-cpnP);
	return ret;
}
