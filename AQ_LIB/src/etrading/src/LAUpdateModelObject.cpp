/*
 * @brief			Implementation body of vanilla Yield Curve analytics
 */

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LAUpdateModelObject.h"
#include <fstream>
#include "LADataInstance.h"
#include "LAFunctionManager.h"
#include "LAPriceDataManager.h"
#include "LADataReference.h"
#include "LACurvePricingObject.h"

#include "LAMathCorrelation.h"
#include "LARatesTermStructureSDE.h"
#include "LARatesLJTermStructureSDE.h"
#include "LARatesCurveLinearInterpolation.h"
#include "LADealUtils.h"

#include "LACoreDataService.h"
#include "LAStaticData.h"
#include "LADataVector.h"
#include "LADataProcedure.h"
#include "LALinearInterpolation.h"
#include "LASplineInterpolation.h"
#include "LAPriceDataInterpolation.h"
#include "LAPriceDataSlidingRule.h"
#include "LAPriceDataDayCount.h"
#include "LAPriceDataFunction.h"


using namespace std;

// constructor
/*!

*/

UpdateObjectPoolForSDEsAndCurves::UpdateObjectPoolForSDEsAndCurves(const LAString &baseCurrency)
: LAUpdateCurveObject(baseCurrency){
}
// destructor
/*!

*/
UpdateObjectPoolForSDEsAndCurves::~UpdateObjectPoolForSDEsAndCurves(void)
{
}




