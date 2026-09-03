#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LADate.h"
#include "LACoreAppError.h"
#include <map>
#include "LABlackScholesCalc.h"
#include "ConstantDeclarations.h"

class LAString;
class LAObject;
class LAPriceDataSlidingRule;
class LAPriceDataCalendar;

class LACoreComponentManager
{
public:
	static std::map<LAString, LAString>&		getInterpolationMap();
	static std::map<LAString, LAString>&		getDayCountMap();
	static std::map<LAString, LAString>&		getBasisTypeMap();
	static std::map<LAString, LAString>&		getBasisIndexMap();
	static std::map<LAString, LABlackScholesBase*>&		getBlackComponentMap(); 
	static std::map<LAString, AnalyticParam*>&	getBlackParamComponentMap(); 
	
	static void deleteBlackComponentMap();
	static void deleteBlackParamComponentMap();

	//====================================================
	// getInterpolation
	static LAString getInterpolation(const LAString &key);
	//====================================================
	// getDayCount
	static LAString getDayCount(const LAString &key);
	//====================================================
	// getBasisType
	static LAString getBasisType(const LAString &key);
	//====================================================
	// getBasisIndex
	static LAString getBasisIndex(const LAString &key);
	//====================================================
	// initialize
	static void initialize();
	//====================================================
	// finalize
	static void finalize();

private:
	static std::map<LAString, LAString>			interpolationMap;
	static std::map<LAString, LAString>			dayCountMap;
	static std::map<LAString, LAString>			basisTypeMap;
	static std::map<LAString, LAString>			basisIndexMap;
	static std::map<LAString, LABlackScholesBase*>		blackComponentMap;
	static std::map<LAString, AnalyticParam*>	blackParamComponentMap;

};
