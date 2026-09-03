#ifndef LAMathArbitrageFreeCurve_h
#define LAMathArbitrageFreeCurve_h

#ifdef __GNUG__
#pragma interface
#endif


#include "AQLPriceDataConvention.h"
#include "AQLFunctionBase.h"
#include "AQLSplineInterpolation.h"
 
//// DEFINES ////
#ifndef IR_CALIBRATION_DATA_DAYCOUNT_FIX
#define IR_CALIBRATION_DATA_DAYCOUNT_FIX			"DAYCOUNTFIX"			// Data Name of DayCount
#endif
#ifndef IR_CALIBRATION_DATA_DAYCOUNT_FLOAT
#define IR_CALIBRATION_DATA_DAYCOUNT_FLOAT		"DAYCOUNTFLOAT"			// Data Name of DayCount
#endif
#ifndef IR_CALIBRATION_DATA_DAYCOUNT_THREE
#define IR_CALIBRATION_DATA_DAYCOUNT_THREE		"DAYCOUNTTHREE"			// Data Name of DayCount
#endif
#ifndef IR_CALIBRATION_DATA_DAYCOUNT_SIX
#define IR_CALIBRATION_DATA_DAYCOUNT_SIX  		"DAYCOUNTSIX"			// Data Name of DayCount
#endif


class AQLDataInstance;

using namespace std;

//=================== YIELD CURVE ==================================
/*! 
    @brief Class of YieldCurve
*/
class LAMathArbitrageFreeCurve
{
public:
	// constructor
	LAMathArbitrageFreeCurve(AQLDataInstance* dataInstance,const AQLString& curveName);
	// copy constructor
	LAMathArbitrageFreeCurve(const LAMathArbitrageFreeCurve& curve);
	// destructor
	virtual ~LAMathArbitrageFreeCurve();

    virtual void		setForecastCurve(AQLDataInstance* dataInstance, const AQLString& curveName);

    double getDF(double term);

    double getRate(double term, const AQLString curveName);

    double getBasisLegValue(double valueTerm, const DoubleArray& fixTerms, const DoubleArray& payTerms, const DoubleArray& accruTerms,
                             const AQLString& forecastID, double basis, bool isPrincipal, double amount, double firstFixingRate);
		
private:
	AQLString mName;
    AQLDate mAsOfDate;
	AQLSplineInterpolation mDFData;
    map<AQLString, AQLSplineInterpolation > mForecastDataMap;
};
#endif
