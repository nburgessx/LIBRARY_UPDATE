/*! @file
    @brief AQLCalibrateVolatility manager class 
*/
//  2008, AlgoQuantHub.
#ifndef AQLCalibrateVolatilityManager_h
#define AQLCalibrateVolatilityManager_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLCalibrateVolatilityManager.h
//
//  DESCRIPTION :       AQLCalibrateVolatilityManager 
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "AQLString.h"


class AQLCoreDataService;
class AQLCalibrateVolatility;
//===================== Class Declare AQLCalibrateVolatilityManager==================================
/*! 
    @brief AQLCalibrateVolatility manager class

    This class is singleton
*/
class AQLCalibrateVolatilityManager
{
public:
	//==============================================
	// return singleton 
	static AQLCalibrateVolatilityManager     *getInstance();
	//==============================================
	// create AQLCalibrateVolatility
	AQLCalibrateVolatility *createVolCreator(const AQLString &model) const;

private:

friend class AQLCoreDataService;
	// constructor
	AQLCalibrateVolatilityManager(void);
	// destructor
	~AQLCalibrateVolatilityManager(void);
	// copy constructor
	AQLCalibrateVolatilityManager(const AQLCalibrateVolatilityManager &rhs);
	AQLCalibrateVolatilityManager &operator=(const AQLCalibrateVolatilityManager &rhs);

	static AQLCalibrateVolatilityManager *mpInstance; // single instance

};

#endif
