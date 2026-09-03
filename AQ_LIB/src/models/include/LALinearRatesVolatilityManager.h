/*! @file
    @brief LALinearRatesVolatilityManager manager class 
*/
//  2007, AlgoQuantHub.
#ifndef LALinearRatesVolatilityManager_h
#define LALinearRatesVolatilityManager_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LALinearRatesVolatilityManager.h
//
//  DESCRIPTION :       LALinearRatesVolatilityManager 
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma interface
#endif

#include <map>
#include "AQLCoreTemplateType.h"
#include "AQLCoreFunctionBase.h"

class AQLString;
class LALinearRatesVolatility;
class AQLObject;
class AQLDataProvider;
//hishida vannavolga
class LALinearRatesModel;

using namespace std;

#ifdef __GNUG__
#include <pthread.h>
#endif

//===================== Class Declare LALinearRatesVolatilityManager==================================
/*! 
    @brief SDE manager class

    This class is singleton
*/
class LALinearRatesVolatilityManager
{
public:
	//==============================================
	// return singleton 
	static LALinearRatesVolatilityManager     *getInstance();
	//==============================================
	// create LALinearRatesVolatilityManager
	LALinearRatesVolatility *createPlainVanillaVolatiltyGenerator(AQLDataProvider* dataProvider, AQLObject& object, function_t producttype, AQLString productname, AQLString modelname) const;
	//==============================================
	// create LALinearRatesModel
	LALinearRatesModel *createPlainVanillaModelGenerator(AQLDataProvider* dataProvider, AQLObject& object, function_t producttype, AQLString productname, AQLString modelname) const;
	//clear pointer
	static void finalize(void);
	
private:
	// constructor
	LALinearRatesVolatilityManager(void);
	// destructor
	~LALinearRatesVolatilityManager(void);
	// copy constructor
	LALinearRatesVolatilityManager(const LALinearRatesVolatilityManager &rhs);
	LALinearRatesVolatilityManager &operator=(const LALinearRatesVolatilityManager &rhs);
	
	static std::map<AQLString, LALinearRatesVolatility*> mVolatilityMap;
	//hishida vannavolga
	static std::map<AQLString, LALinearRatesModel*> mModelMap;
	static LALinearRatesVolatilityManager *mpInstance; // single instance
#ifdef __HAS_MIC__
	static common_lib::StaticMutex mMutex;
#endif
};

#endif
