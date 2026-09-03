/*! @file
    @brief LALinearRatesVolatilityManager manager class 
*/
//  2007, Mizuho International London.
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
#include "LACoreTemplateType.h"
#include "LACoreFunctionBase.h"

class LAString;
class LALinearRatesVolatility;
class LAObject;
class LADataProvider;
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
	LALinearRatesVolatility *createPlainVanillaVolatiltyGenerator(LADataProvider* dataProvider, LAObject& object, function_t producttype, LAString productname, LAString modelname) const;
	//==============================================
	// create LALinearRatesModel
	LALinearRatesModel *createPlainVanillaModelGenerator(LADataProvider* dataProvider, LAObject& object, function_t producttype, LAString productname, LAString modelname) const;
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
	
	static std::map<LAString, LALinearRatesVolatility*> mVolatilityMap;
	//hishida vannavolga
	static std::map<LAString, LALinearRatesModel*> mModelMap;
	static LALinearRatesVolatilityManager *mpInstance; // single instance
#ifdef __HAS_MIC__
	static common_lib::StaticMutex mMutex;
#endif
};

#endif
