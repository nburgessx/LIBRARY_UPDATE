#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLDate.h"
#include "AQLCoreAppError.h"
#include <map>
#include "LABlackScholesCalc.h"
#include "ConstantDeclarations.h"

class AQLString;
class AQLObject;
class AQLPriceDataSlidingRule;
class AQLPriceDataCalendar;

class AQLCoreComponentManager
{
public:
	static std::map<AQLString, AQLString>&		getInterpolationMap();
	static std::map<AQLString, AQLString>&		getDayCountMap();
	static std::map<AQLString, AQLString>&		getBasisTypeMap();
	static std::map<AQLString, AQLString>&		getBasisIndexMap();
	static std::map<AQLString, LABlackScholesBase*>&		getBlackComponentMap(); 
	static std::map<AQLString, AnalyticParam*>&	getBlackParamComponentMap(); 
	
	static void deleteBlackComponentMap();
	static void deleteBlackParamComponentMap();

	//====================================================
	// getInterpolation
	static AQLString getInterpolation(const AQLString &key);
	//====================================================
	// getDayCount
	static AQLString getDayCount(const AQLString &key);
	//====================================================
	// getBasisType
	static AQLString getBasisType(const AQLString &key);
	//====================================================
	// getBasisIndex
	static AQLString getBasisIndex(const AQLString &key);
	//====================================================
	// initialize
	static void initialize();
	//====================================================
	// finalize
	static void finalize();

private:
	static std::map<AQLString, AQLString>			interpolationMap;
	static std::map<AQLString, AQLString>			dayCountMap;
	static std::map<AQLString, AQLString>			basisTypeMap;
	static std::map<AQLString, AQLString>			basisIndexMap;
	static std::map<AQLString, LABlackScholesBase*>		blackComponentMap;
	static std::map<AQLString, AnalyticParam*>	blackParamComponentMap;

};
