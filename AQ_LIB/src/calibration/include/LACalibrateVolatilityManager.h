/*! @file
    @brief LACalibrateVolatility manager class 
*/
//  2008, AlgoQuantHub.
#ifndef LACalibrateVolatilityManager_h
#define LACalibrateVolatilityManager_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrateVolatilityManager.h
//
//  DESCRIPTION :       LACalibrateVolatilityManager 
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "AQLString.h"


class LACoreDataService;
class LACalibrateVolatility;
//===================== Class Declare LACalibrateVolatilityManager==================================
/*! 
    @brief LACalibrateVolatility manager class

    This class is singleton
*/
class LACalibrateVolatilityManager
{
public:
	//==============================================
	// return singleton 
	static LACalibrateVolatilityManager     *getInstance();
	//==============================================
	// create LACalibrateVolatility
	LACalibrateVolatility *createVolCreator(const AQLString &model) const;

private:

friend class LACoreDataService;
	// constructor
	LACalibrateVolatilityManager(void);
	// destructor
	~LACalibrateVolatilityManager(void);
	// copy constructor
	LACalibrateVolatilityManager(const LACalibrateVolatilityManager &rhs);
	LACalibrateVolatilityManager &operator=(const LACalibrateVolatilityManager &rhs);

	static LACalibrateVolatilityManager *mpInstance; // single instance

};

#endif
