/*!  
	@file	
    @brief Source code for class to represent ArbitrageFreeCurve.
*/
//  2009, Mizuho International London.

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#define SIMPLE		"SIMPLE"
#define ANNUAL		"ANNUAL"
#define SEMI_ANNUAL	"SEMI-ANNUAL"
#define QUARTERLY	"QUARTERLY"
#define MONTHLY		"MONTHLY"
#define CONTINUOUS	"CONTINUOUS"

#include "LAMathArbFreeCurve.h"
#include "LAMathDefine.h"
#include "LABasic.h"
#include "LADataReference.h"
#include "LADataInstance.h"
#include <LADataBasics.h>
#include <LADataVector.h>
#include <LAPriceDataManager.h>
#include "LASplineInterpolation.h"

#include <cmath>

//====================================================================
// YIELD CURVE
/*!
    @brief constructor

	@param[in] dataInstance pointer of LADataInstance

*/
LAMathArbitrageFreeCurve::LAMathArbitrageFreeCurve(LADataInstance* dataInstance, const LAString& curveID)
{
    mName = curveID;

    const LAObject& baseCurve = dataInstance->getObjectPool().getObject(curveID,ENCHKTYPE_ISDEFINED).get();
    mAsOfDate = dynamic_cast<const LADataDate&> ((baseCurve.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL)).get()).get();

    const DoubleArray& dfTerms = dynamic_cast<const LADataDoubles& >(baseCurve.getData("Terms",ISDEFINED).get()).get();
    const DoubleArray& dfs = dynamic_cast<const LADataDoubles& >(baseCurve.getData("DiscountFactors",ISDEFINED).get()).get();
    mDFData.set(dfTerms,dfs);
	
    LAString threeMCurveName = curveID + "_ThreeMLibor";
    const LAObject& threeMCurve = dataInstance->getObjectPool().getObject(threeMCurveName,ENCHKTYPE_ISDEFINED).get();
    const DoubleArray& threeLTerms = dynamic_cast<const LADataDoubles& >(threeMCurve.getData("Terms_Rate",ISDEFINED).get()).get();
    const DoubleArray& threeLRates = dynamic_cast<const LADataDoubles& >(threeMCurve.getData("ForecastRates",ISDEFINED).get()).get();
    LASplineInterpolation theeLData; theeLData.set(threeLTerms,threeLRates);
    mForecastDataMap.insert( make_pair( "ThreeMLibor", theeLData ) );

    LAString sixMCurveName = curveID + "_SixMLibor";
    const LAObject& sixMCurve = dataInstance->getObjectPool().getObject(sixMCurveName,ENCHKTYPE_ISDEFINED).get();
    const DoubleArray& sixLTerms = dynamic_cast<const LADataDoubles& >(sixMCurve.getData("Terms_Rate",ISDEFINED).get()).get();
    const DoubleArray& sixLRates = dynamic_cast<const LADataDoubles& >(sixMCurve.getData("ForecastRates",ISDEFINED).get()).get();
    LASplineInterpolation sixLData; sixLData.set(sixLTerms,sixLRates);
    mForecastDataMap.insert( make_pair( "SixMLibor", sixLData ) );	
}

/*!
    @brief destructor	
*/
LAMathArbitrageFreeCurve::~LAMathArbitrageFreeCurve()
{
}

void 
LAMathArbitrageFreeCurve::setForecastCurve(LADataInstance* dataInstance, const LAString& curveName)
{
    LAString forecastCurName = mName + "_" + curveName;
    const LAObject& forecastCurve =  dataInstance->getObjectPool().getObject(forecastCurName,ENCHKTYPE_ISDEFINED).get();
    LAString arbFreeCurName_for = dynamic_cast<const LADataString&> ((forecastCurve.getData("ArbFreeCurve", ISNOTNULL)).get()).get();
    if( arbFreeCurName_for != mName ) throw LACoreInvalidData("arv free curves are inconsistent !", __FILE__, __LINE__);

    const DoubleArray& terms = dynamic_cast<const LADataDoubles& >(forecastCurve.getData("Terms_Rate",ISDEFINED).get()).get();
    const DoubleArray& rates = dynamic_cast<const LADataDoubles& >(forecastCurve.getData("ForecastRates",ISDEFINED).get()).get();
    LASplineInterpolation data; data.set(terms,rates);

    mForecastDataMap.insert( make_pair( curveName, data ) );
}

double LAMathArbitrageFreeCurve::getDF(double term)
{
    return mDFData.value(term);
}

double LAMathArbitrageFreeCurve::getRate(double term, const LAString curveName)
{
    return mForecastDataMap[curveName].value(term);
}

double 
LAMathArbitrageFreeCurve::getBasisLegValue
(double valueTerm, const DoubleArray& fixTerms, const DoubleArray& payTerms, const DoubleArray& accruTerms,
 const LAString& forecastID, double basis, bool isPrincipal, double amount, double firstFixingRate)
{
    size_t legSize = payTerms.size();
    if( legSize != fixTerms.size() || legSize != accruTerms.size() ) throw LACoreInvalidData("fixing,payment and accrual times are not same!",__FILE__,__LINE__);
    //set DF data
    double valueDateDF = getDF(valueTerm);
    DoubleArray dfs;
    for(size_t i=0; i<legSize; i++)
    {
        dfs.push_back( getDF(payTerms[i]) );   
    }

    //set curve data
    DoubleArray rates(legSize, 0.);
    if( forecastID != "" )
    {
        if( fixTerms[0] < 0. ) rates[0] = firstFixingRate;
        else rates[0] = mForecastDataMap[forecastID].value(fixTerms[0]); 
        for(size_t i=1; i<legSize; i++)
        {
            rates[i] = mForecastDataMap[forecastID].value(fixTerms[i]);   
        }
    }

    double asOfValue=0.;
    for(size_t i=0; i<legSize; i++)
    {
        asOfValue += ( rates[i] + basis ) * accruTerms[i] * dfs[i];
    }

    if( isPrincipal ) asOfValue += dfs[legSize-1];
    return asOfValue / valueDateDF * amount;
}
