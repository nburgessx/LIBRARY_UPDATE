//  2008, Mizuho International London.
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LAMathJamshidianRStarFinder.h"
#include "LAObject.h"
#include "LADataProcedure.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataReference.h"
#include "LADataMultiReference.h"
#include "LADataInstance.h"
#include "LAPriceDataManager.h"
#include "LAObjectPool.h"
#include "LACoreTemplateType.h"
#include "LAMathDefine.h"
#include "LAPriceDataCalendar.h"
#include "LAPriceDataSlidingRule.h"
#include "LAPriceDataDayCount.h"
#include "LADataReference.h"
#include "LAPriceDataFunction.h"
#include "LABasic.h"
#include "LAAlgorithm.h"
#include "LAModelDynamicsHW1FCurve.h"

using namespace std;
//////////////////LAMathRstarFinder

LAMathJamshidianRStarFinder::LAMathJamshidianRStarFinder(double strike, double premium, const DoubleVector& grids, 
													  const DoubleVector& deltas, LARatesPathElementCurve* pcurve)
													  : LAFunctionBase(), mStrike(strike),mPrem(premium),mGrids(grids),mDelta(deltas),is_cloned(false)
{
	mpCurve = pcurve;
}

LAMathJamshidianRStarFinder::~LAMathJamshidianRStarFinder()
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
LAMathJamshidianRStarFinder::LAMathJamshidianRStarFinder(const LAMathJamshidianRStarFinder &rhs) 
:
LAFunctionBase(rhs),mStrike(rhs.mStrike),mPrem(rhs.mPrem),mGrids(rhs.mGrids),mDelta(rhs.mDelta),is_cloned(true),
mpCurve(rhs.mpCurve !=0 ? dynamic_cast<LARatesPathElementCurve*>(rhs.mpCurve->clone()) : 0)
{
}

/*!
    @brief Return this function type
    @return function type
*/

function_t
LAMathJamshidianRStarFinder::getType() const
{
	return FN_JAMSHIDIANRSTARFINDER;
}

LACoreFunctionBase*
LAMathJamshidianRStarFinder::clone() const
{
    try 
	{
		return new LAMathJamshidianRStarFinder(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}


double
LAMathJamshidianRStarFinder::operator()(const DoubleArray& x) const
{
	if(x.size() == 1)
		return operator()(x[0]);
	throw LACoreInvalidData("parameter size must be one", __FILE__, __LINE__);
}


double
LAMathJamshidianRStarFinder::operator()(const double& x) const
{
	double r0 = x;
	dynamic_cast<LARatesPathElementBase *>(mpCurve)->set(r0);
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
