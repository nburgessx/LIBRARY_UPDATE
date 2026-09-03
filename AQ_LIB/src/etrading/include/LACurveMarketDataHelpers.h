//
//  LACurveMarketDataHelpers.h
//  This file was previous called YieldCurveMarketUti1s.h and before that AQLMarketData.h
//

#include <algorithm>
#include "AQLDataInstance.h"
#include "AQLCoreTemplateType.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataMultiReference.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLPriceDataCalendar.h"
#include "LADateHelpers.h"
#include "AQLPriceDataDayCount.h"
#include "AQLDefinitions.h"
#include "AQLDefinitionsRisk.h"
#include "AQLCoreDataService.h"
#include "AQLStaticDataManager.h"
#include "AQLFileAccessor.h"

#include "CurveCalibrationData.h"


//===================== Class Declare AQLMarketData==================================
/*! 
    @brief  Market data util class   
*/

class LACurveMarketDataHelpers
{
public:
		
	//==============================================
	// reset marketdata use libor
	static void resetMarketDataUsingLibor(CurveCalibrationData &curve, const AQLString &ccy, const AQLString *pCurveType = 0);

	//==============================================
	// sort yield curve reference
	static void sortMarketData(CurveCalibrationData &ypro);

	//==============================================
	// restore swap rate 
	static void restoreSwapRateFromLibor(CurveCalibrationData &curve, const std::map<AQLString, double> &sRateMap, const AQLString &ccy, const AQLString *pCurveType = 0);

private:

//friend class AQLCoreDataService;

	// constructor
	LACurveMarketDataHelpers(void);
	// destructor
	~LACurveMarketDataHelpers(void);
	// copy constructor
	LACurveMarketDataHelpers(const LACurveMarketDataHelpers &rhs);
	LACurveMarketDataHelpers &operator=(const LACurveMarketDataHelpers &rhs);
	
};


