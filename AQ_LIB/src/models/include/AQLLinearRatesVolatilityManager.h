/*! @file
    @brief AQLLinearRatesVolatilityManager manager class 
*/
#ifndef AQLLinearRatesVolatilityManager_h
#define AQLLinearRatesVolatilityManager_h
#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma interface
#endif

#include <map>
#include "AQLCoreTemplateType.h"
#include "AQLCoreFunctionBase.h"

class AQLString;
class AQLLinearRatesVolatility;
class AQLObject;
class AQLDataProvider;
//hishida vannavolga
class AQLLinearRatesModel;

using namespace std;

#ifdef __GNUG__
#include <pthread.h>
#endif

//===================== Class Declare AQLLinearRatesVolatilityManager==================================
/*! 
    @brief SDE manager class

    This class is singleton
*/
class AQLLinearRatesVolatilityManager
{
public:
	//==============================================
	// return singleton 
	static AQLLinearRatesVolatilityManager     *getInstance();
	//==============================================
	// create AQLLinearRatesVolatilityManager
	AQLLinearRatesVolatility *createPlainVanillaVolatiltyGenerator(AQLDataProvider* dataProvider, AQLObject& object, function_t producttype, AQLString productname, AQLString modelname) const;
	//==============================================
	// create AQLLinearRatesModel
	AQLLinearRatesModel *createPlainVanillaModelGenerator(AQLDataProvider* dataProvider, AQLObject& object, function_t producttype, AQLString productname, AQLString modelname) const;
	//clear pointer
	static void finalize(void);
	
private:
	// constructor
	AQLLinearRatesVolatilityManager(void);
	// destructor
	~AQLLinearRatesVolatilityManager(void);
	// copy constructor
	AQLLinearRatesVolatilityManager(const AQLLinearRatesVolatilityManager &rhs);
	AQLLinearRatesVolatilityManager &operator=(const AQLLinearRatesVolatilityManager &rhs);
	
	static std::map<AQLString, AQLLinearRatesVolatility*> mVolatilityMap;
	//hishida vannavolga
	static std::map<AQLString, AQLLinearRatesModel*> mModelMap;
	static AQLLinearRatesVolatilityManager *mpInstance; // single instance
#ifdef __HAS_MIC__
	static common_lib::StaticMutex mMutex;
#endif
};

#endif
