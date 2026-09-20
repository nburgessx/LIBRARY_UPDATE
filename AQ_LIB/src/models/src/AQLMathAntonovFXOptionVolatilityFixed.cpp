#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "AQLMathAntonovFXOptionVolatilityFixed.h"
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
#include "AQLDateCalculations.h"
#include "AQLPriceCFGenUtility.h"
#include "AQLLinearInterpolation.h"
#include "AQLSplineInterpolation.h"
#include "AQLStepInterpolation.h"
#include "AQLDataMatrix.h"
#include "AQLModelDynamicsHW1FCurve.h"

#include "AQLCoreComponentManager.h"

using namespace std;

AQLMathAntonovFXOptionVolatilityFixed::AQLMathAntonovFXOptionVolatilityFixed()
: AQLMathAntonovFXOption()
{}

AQLMathAntonovFXOptionVolatilityFixed::AQLMathAntonovFXOptionVolatilityFixed(const AQLMathAntonovFXOptionVolatilityFixed& v)
: AQLMathAntonovFXOption(v)
{}

AQLMathAntonovFXOptionVolatilityFixed::~AQLMathAntonovFXOptionVolatilityFixed()
{}


/*!
    @brief Return this function type
    @return function type
*/

function_t
AQLMathAntonovFXOptionVolatilityFixed::getType() const
{
	return FN_ANTONOVFXOPTIOINVOLATILITYFIXED;
}

AQLCoreFunctionBase*
AQLMathAntonovFXOptionVolatilityFixed::clone() const
{
    try 
	{
		return new AQLMathAntonovFXOptionVolatilityFixed(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

void			
AQLMathAntonovFXOptionVolatilityFixed::setCalibParamFirst(AQLMathAntonovFXOptionDataProvider* dataProvider, double vol, double beta) const
{
unsigned int size = dataProvider->mTimeGrids.size();
	DoubleVector v(size),s(size);
	
	for(unsigned int i=dataProvider->mSpos; i<=dataProvider->mEpos;i++)
		dataProvider->mBetaGrids[i] = beta;
	
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
		v[i] = beta*dataProvider->mVolGrids[i];

	dataProvider->mVGrids = v;
	dataProvider->mSGrids = s;
	return;
}

void			
AQLMathAntonovFXOptionVolatilityFixed::setCalibParam(AQLMathAntonovFXOptionDataProvider* dataProvider, double vol, double beta) const
{
for(unsigned int i=dataProvider->mSpos ;i<= dataProvider->mEpos;i++)
	{
		dataProvider->mVGrids[i] = dataProvider->mVolGrids[i]*beta;
		dataProvider->mBetaGrids[i] = beta;
	}
	return;
}




