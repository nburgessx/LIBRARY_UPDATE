/*!  
	@file	
    @brief Source code for class to represent ArbitrageFreeCurve.
*/
//  2009, AlgoQuantHub.

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
#include "AQLMathDefine.h"
#include "AQLBasic.h"
#include "AQLDataReference.h"
#include "AQLDataInstance.h"
#include <AQLDataBasics.h>
#include <AQLDataVector.h>
#include <AQLPriceDataManager.h>
#include "AQLSplineInterpolation.h"

#include <cmath>

//====================================================================
// YIELD CURVE
/*!
    @brief constructor

	@param[in] dataInstance pointer of AQLDataInstance

*/
LAMathArbitrageFreeCurve::LAMathArbitrageFreeCurve(AQLDataInstance* dataInstance, const AQLString& curveID)
{
    mName = curveID;

    const AQLObject& baseCurve = dataInstance->getObjectPool().getObject(curveID,ENCHKTYPE_ISDEFINED).get();
    mAsOfDate = dynamic_cast<const AQLDataDate&> ((baseCurve.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL)).get()).get();

    const DoubleArray& dfTerms = dynamic_cast<const AQLDataDoubles& >(baseCurve.getData("Terms",ISDEFINED).get()).get();
    const DoubleArray& dfs = dynamic_cast<const AQLDataDoubles& >(baseCurve.getData("DiscountFactors",ISDEFINED).get()).get();
    mDFData.set(dfTerms,dfs);
	
    AQLString threeMCurveName = curveID + "_ThreeMLibor";
    const AQLObject& threeMCurve = dataInstance->getObjectPool().getObject(threeMCurveName,ENCHKTYPE_ISDEFINED).get();
    const DoubleArray& threeLTerms = dynamic_cast<const AQLDataDoubles& >(threeMCurve.getData("Terms_Rate",ISDEFINED).get()).get();
    const DoubleArray& threeLRates = dynamic_cast<const AQLDataDoubles& >(threeMCurve.getData("ForecastRates",ISDEFINED).get()).get();
    AQLSplineInterpolation theeLData; theeLData.set(threeLTerms,threeLRates);
    mForecastDataMap.insert( make_pair( "ThreeMLibor", theeLData ) );

    AQLString sixMCurveName = curveID + "_SixMLibor";
    const AQLObject& sixMCurve = dataInstance->getObjectPool().getObject(sixMCurveName,ENCHKTYPE_ISDEFINED).get();
    const DoubleArray& sixLTerms = dynamic_cast<const AQLDataDoubles& >(sixMCurve.getData("Terms_Rate",ISDEFINED).get()).get();
    const DoubleArray& sixLRates = dynamic_cast<const AQLDataDoubles& >(sixMCurve.getData("ForecastRates",ISDEFINED).get()).get();
    AQLSplineInterpolation sixLData; sixLData.set(sixLTerms,sixLRates);
    mForecastDataMap.insert( make_pair( "SixMLibor", sixLData ) );	
}

/*!
    @brief destructor	
*/
LAMathArbitrageFreeCurve::~LAMathArbitrageFreeCurve()
{
}

void 
LAMathArbitrageFreeCurve::setForecastCurve(AQLDataInstance* dataInstance, const AQLString& curveName)
{
    AQLString forecastCurName = mName + "_" + curveName;
    const AQLObject& forecastCurve =  dataInstance->getObjectPool().getObject(forecastCurName,ENCHKTYPE_ISDEFINED).get();
    AQLString arbFreeCurName_for = dynamic_cast<const AQLDataString&> ((forecastCurve.getData("ArbFreeCurve", ISNOTNULL)).get()).get();
    if( arbFreeCurName_for != mName ) throw AQLCoreInvalidData("arv free curves are inconsistent !", __FILE__, __LINE__);

    const DoubleArray& terms = dynamic_cast<const AQLDataDoubles& >(forecastCurve.getData("Terms_Rate",ISDEFINED).get()).get();
    const DoubleArray& rates = dynamic_cast<const AQLDataDoubles& >(forecastCurve.getData("ForecastRates",ISDEFINED).get()).get();
    AQLSplineInterpolation data; data.set(terms,rates);

    mForecastDataMap.insert( make_pair( curveName, data ) );
}

double LAMathArbitrageFreeCurve::getDF(double term)
{
    return mDFData.value(term);
}

double LAMathArbitrageFreeCurve::getRate(double term, const AQLString curveName)
{
    return mForecastDataMap[curveName].value(term);
}

double 
LAMathArbitrageFreeCurve::getBasisLegValue
(double valueTerm, const DoubleArray& fixTerms, const DoubleArray& payTerms, const DoubleArray& accruTerms,
 const AQLString& forecastID, double basis, bool isPrincipal, double amount, double firstFixingRate)
{
    size_t legSize = payTerms.size();
    if( legSize != fixTerms.size() || legSize != accruTerms.size() ) throw AQLCoreInvalidData("fixing,payment and accrual times are not same!",__FILE__,__LINE__);
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
