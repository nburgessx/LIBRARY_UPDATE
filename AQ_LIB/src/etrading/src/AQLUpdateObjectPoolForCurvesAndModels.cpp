/*
 * @brief			Implementation body of vanilla Yield Curve analytics
 */

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLUpdateObjectPoolForCurvesAndModels.h"
#include <fstream>
#include "AQLDataInstance.h"
#include "AQLFunctionManager.h"
#include "AQLPriceDataManager.h"
#include "AQLDataReference.h"
#include "AQLCurvePricingObject.h"

#ifndef VISUAL_STUDIO_2010_ANALYTICS 
#include "AQLMathCorrelation.h"
#include "AQLRatesTermStructureSDE.h"
#include "AQLRatesLJTermStructureSDE.h"
#include "AQLRatesCurveLinearInterpolation.h"
#include "AQLDealUtils.h"
#endif

#include "AQLCoreDataService.h"
#include "AQLStaticData.h"
#include "AQLDataVector.h"
#include "AQLDataProcedure.h"
#include "AQLLinearInterpolation.h"
#include "AQLSplineInterpolation.h"
#include "AQLPriceDataInterpolation.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLPriceDataDayCount.h"
#include "AQLPriceDataFunction.h"


using namespace std;

//! constructor
/*!

*/

UpdateObjectPoolForSDEsAndCurves::UpdateObjectPoolForSDEsAndCurves(const AQLString &baseCurrency)
: AQLUpdateObjectPoolForCurves(baseCurrency){
}
//! destructor
/*!

*/
UpdateObjectPoolForSDEsAndCurves::~UpdateObjectPoolForSDEsAndCurves(void)
{
}




