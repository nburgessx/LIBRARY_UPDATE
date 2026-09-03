#ifndef LAMathArbitrageFreeCurve_h
#define LAMathArbitrageFreeCurve_h

#ifdef __GNUG__
#pragma interface
#endif


#include "LAPriceDataConvention.h"
#include "LAFunctionBase.h"
#include "LASplineInterpolation.h"
 
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


class LADataInstance;

using namespace std;

//=================== YIELD CURVE ==================================
/*! 
    @brief Class of YieldCurve
*/
class LAMathArbitrageFreeCurve
{
public:
	// constructor
	LAMathArbitrageFreeCurve(LADataInstance* dataInstance,const LAString& curveName);
	// copy constructor
	LAMathArbitrageFreeCurve(const LAMathArbitrageFreeCurve& curve);
	// destructor
	virtual ~LAMathArbitrageFreeCurve();

    virtual void		setForecastCurve(LADataInstance* dataInstance, const LAString& curveName);

    double getDF(double term);

    double getRate(double term, const LAString curveName);

    double getBasisLegValue(double valueTerm, const DoubleArray& fixTerms, const DoubleArray& payTerms, const DoubleArray& accruTerms,
                             const LAString& forecastID, double basis, bool isPrincipal, double amount, double firstFixingRate);
		
private:
	LAString mName;
    LADate mAsOfDate;
	LASplineInterpolation mDFData;
    map<LAString, LASplineInterpolation > mForecastDataMap;
};
#endif
