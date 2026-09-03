//
//  LACurveMarketDataHelpers.h
//  This file was previous called YieldCurveMarketUti1s.h and before that LAMarketData.h
//

#include <algorithm>
#include "LADataInstance.h"
#include "LACoreTemplateType.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataMultiReference.h"
#include "LAPriceDataSlidingRule.h"
#include "LAPriceDataCalendar.h"
#include "LADateHelpers.h"
#include "LAPriceDataDayCount.h"
#include "LADefinitions.h"
#include "LADefinitionsRisk.h"
#include "LACoreDataService.h"
#include "LAStaticDataManager.h"
#include "LAFileAccessor.h"

#include "CurveCalibrationData.h"


//===================== Class Declare LAMarketData==================================
/*! 
    @brief  Market data util class   
*/

class LACurveMarketDataHelpers
{
public:
		
	//==============================================
	// reset marketdata use libor
	static void resetMarketDataUsingLibor(CurveCalibrationData &curve, const LAString &ccy, const LAString *pCurveType = 0);

	//==============================================
	// sort yield curve reference
	static void sortMarketData(CurveCalibrationData &ypro);

	//==============================================
	// restore swap rate 
	static void restoreSwapRateFromLibor(CurveCalibrationData &curve, const std::map<LAString, double> &sRateMap, const LAString &ccy, const LAString *pCurveType = 0);

private:

//friend class LACoreDataService;

	// constructor
	LACurveMarketDataHelpers(void);
	// destructor
	~LACurveMarketDataHelpers(void);
	// copy constructor
	LACurveMarketDataHelpers(const LACurveMarketDataHelpers &rhs);
	LACurveMarketDataHelpers &operator=(const LACurveMarketDataHelpers &rhs);
	
};


