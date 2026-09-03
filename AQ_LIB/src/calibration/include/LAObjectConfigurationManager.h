/*! @file
    @brief LAObjectConfiguration manager class 
*/
//  2008, AlgoQuantHub.
#ifndef LAObjectConfigurationManager_h
#define LAObjectConfigurationManager_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAObjectConfigurationManager.h
//
//  DESCRIPTION :       LAObjectConfigurationManager 
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif



class LACoreDataService;
class AQLString;
class LAObjectConfiguration;

//===================== Class Declare LAObjectConfigurationManager==================================
/*! 
    @brief LAObjectConfigurationManager Factory class

    This class is singleton
*/
class LAObjectConfigurationManager
{
public:
	//==============================================
	// return singleton
	static LAObjectConfigurationManager     *getInstance();
	//==============================================
	// create LAObjectConfiguration
	LAObjectConfiguration *createEntitySetUpper(const AQLString &model) const;

private:

friend class LACoreDataService;
	// constructor
	LAObjectConfigurationManager(void);
	// destructor
	~LAObjectConfigurationManager(void);
	// copy constructor
	LAObjectConfigurationManager(const LAObjectConfigurationManager &rhs);
	LAObjectConfigurationManager &operator=(const LAObjectConfigurationManager &rhs);

	static LAObjectConfigurationManager *mpInstance; // single instance

};

#endif
