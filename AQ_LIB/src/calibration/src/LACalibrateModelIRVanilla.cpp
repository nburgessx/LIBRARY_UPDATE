#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include <fstream>
#include "LADataInstance.h"
#include "LAFunctionManager.h"
#include "LAPriceDataManager.h"
#include "LADataReference.h"
#include "LAMathYieldCurve.h"
#include "LAMathYieldCurvePro.h"

#ifndef VISUAL_STUDIO_2010_ANALYTICS 
#include "LAMathCorrelation.h"
#include "LARatesTermStructureSDE.h"
#include "LARatesLJTermStructureSDE.h"
#include "LARatesCurveLinearInterpolation.h"
#include "LADealUtils.h"
#endif

#include "LACoreDataService.h"
#include "LAStaticData.h"
#include "LACalibrateModelIRVanilla.h"
#include "LADataVector.h"
#include "LADataProcedure.h"
#include "LAMathYieldCurve.h"
#include "LAPriceYieldGenerator.h"
#include "LALinearInterpolation.h"
#include "LASplineInterpolation.h"
#include "LAPriceDataInterpolation.h"
#include "LAPriceDataSlidingRule.h"
#include "LAPriceDataDayCount.h"
#include "LAPriceDataFunction.h"
#include "LAMarketData.h"


using namespace std;

// constructor
/*!

*/

LACalibrateModelIRVanilla::LACalibrateModelIRVanilla(const LAString &baseCurrency)
: LACalibrateModelIR(baseCurrency){
}
// destructor
/*!

*/
LACalibrateModelIRVanilla::~LACalibrateModelIRVanilla(void)
{
}




