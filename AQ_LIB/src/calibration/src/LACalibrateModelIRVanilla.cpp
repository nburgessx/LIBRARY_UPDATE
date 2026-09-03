#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include <fstream>
#include "AQLDataInstance.h"
#include "AQLFunctionManager.h"
#include "AQLPriceDataManager.h"
#include "AQLDataReference.h"
#include "AQLMathYieldCurve.h"
#include "AQLMathYieldCurvePro.h"

#ifndef VISUAL_STUDIO_2010_ANALYTICS 
#include "AQLMathCorrelation.h"
#include "AQLRatesTermStructureSDE.h"
#include "AQLRatesLJTermStructureSDE.h"
#include "AQLRatesCurveLinearInterpolation.h"
#include "LADealUtils.h"
#endif

#include "LACoreDataService.h"
#include "LAStaticData.h"
#include "LACalibrateModelIRVanilla.h"
#include "AQLDataVector.h"
#include "AQLDataProcedure.h"
#include "AQLMathYieldCurve.h"
#include "AQLPriceYieldGenerator.h"
#include "AQLLinearInterpolation.h"
#include "AQLSplineInterpolation.h"
#include "AQLPriceDataInterpolation.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLPriceDataDayCount.h"
#include "AQLPriceDataFunction.h"
#include "LAMarketData.h"


using namespace std;

// constructor
/*!

*/

LACalibrateModelIRVanilla::LACalibrateModelIRVanilla(const AQLString &baseCurrency)
: LACalibrateModelIR(baseCurrency){
}
// destructor
/*!

*/
LACalibrateModelIRVanilla::~LACalibrateModelIRVanilla(void)
{
}




