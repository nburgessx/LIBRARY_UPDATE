//  2008, AlgoQuantHub.
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LAMathAntonovFXOptionBetaFixed.h"
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
#include "LAMathDateCalculations.h"
#include "LAPriceCFGenUtility.h"
#include "AQLLinearInterpolation.h"
#include "AQLSplineInterpolation.h"
#include "AQLStepInterpolation.h"
#include "AQLDataMatrix.h"
#include "LAModelDynamicsHW1FCurve.h"

#include "AQLCoreComponentManager.h"

using namespace std;

LAMathAntonovFXOptionBetaFixed::LAMathAntonovFXOptionBetaFixed()
: LAMathAntonovFXOption()
{}

LAMathAntonovFXOptionBetaFixed::LAMathAntonovFXOptionBetaFixed(const LAMathAntonovFXOptionBetaFixed& v)
: LAMathAntonovFXOption(v)
{}



LAMathAntonovFXOptionBetaFixed::~LAMathAntonovFXOptionBetaFixed()
{}

/*!
    @brief Return this function type
    @return function type
*/

function_t
LAMathAntonovFXOptionBetaFixed::getType() const
{
	return FN_ANTONOVFXOPTIOINBETAFIXED;
}

AQLCoreFunctionBase*
LAMathAntonovFXOptionBetaFixed::clone() const
{
    try 
	{
		return new LAMathAntonovFXOptionBetaFixed(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

void			
LAMathAntonovFXOptionBetaFixed::setCalibParamFirst(LAMathAntonovFXOptionDataProvider* dataProvider, double vol, double beta) const
{
	unsigned int size = dataProvider->mTimeGrids.size();
	DoubleVector v(size),s(size);
	
	
	for(unsigned int i=0;i<size;i++)
	{
		if(0.0==dataProvider->mBetaGrids[i])
			throw AQLCoreInvalidData("Beta 0.0 is not allowed",__FILE__,__LINE__);
		//this needs at getA (where is divided by fxgris[i];
		if(0.0==dataProvider->mFxGrids[i])
			throw AQLCoreInvalidData("Fx 0.0 is not allowed",__FILE__,__LINE__);
		
		v[i] = dataProvider->mBetaGrids[i] * dataProvider->mVolGrids[i];
		s[i] = (1.0 - dataProvider->mBetaGrids[i]) / dataProvider->mBetaGrids[i] * dataProvider->mFxGrids[i];
	}

	for(unsigned int i=dataProvider->mSpos;i<=dataProvider->mEpos;i++)
		v[i] = dataProvider->mBetaGrids[i]*vol;

	dataProvider->mVGrids = v;
	dataProvider->mSGrids = s;
	return;
}

void			
LAMathAntonovFXOptionBetaFixed::setCalibParam(LAMathAntonovFXOptionDataProvider* dataProvider, double vol, double beta) const
{
	for(unsigned int i=dataProvider->mSpos ;i<= dataProvider->mEpos;i++)
	{
		dataProvider->mVGrids[i] = vol*dataProvider->mBetaGrids[i];
		dataProvider->mVolGrids[i] = vol;
	}
	return;
}




