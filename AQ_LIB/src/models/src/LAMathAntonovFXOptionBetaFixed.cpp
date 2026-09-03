//  2008, Mizuho International London.
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LAMathAntonovFXOptionBetaFixed.h"
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
#include "LAMathDateCalculations.h"
#include "LAPriceCFGenUtility.h"
#include "LALinearInterpolation.h"
#include "LASplineInterpolation.h"
#include "LAStepInterpolation.h"
#include "LADataMatrix.h"
#include "LAModelDynamicsHW1FCurve.h"

#include "LACoreComponentManager.h"

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

LACoreFunctionBase*
LAMathAntonovFXOptionBetaFixed::clone() const
{
    try 
	{
		return new LAMathAntonovFXOptionBetaFixed(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
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
			throw LACoreInvalidData("Beta 0.0 is not allowed",__FILE__,__LINE__);
		//this needs at getA (where is divided by fxgris[i];
		if(0.0==dataProvider->mFxGrids[i])
			throw LACoreInvalidData("Fx 0.0 is not allowed",__FILE__,__LINE__);
		
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




