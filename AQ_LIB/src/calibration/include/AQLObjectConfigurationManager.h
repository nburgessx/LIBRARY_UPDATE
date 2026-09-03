/*! @file
    @brief AQLObjectConfiguration manager class 
*/
//  2008, AlgoQuantHub.
#ifndef AQLObjectConfigurationManager_h
#define AQLObjectConfigurationManager_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLObjectConfigurationManager.h
//
//  DESCRIPTION :       AQLObjectConfigurationManager 
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif



class AQLCoreDataService;
class AQLString;
class AQLObjectConfiguration;

//===================== Class Declare AQLObjectConfigurationManager==================================
/*! 
    @brief AQLObjectConfigurationManager Factory class

    This class is singleton
*/
class AQLObjectConfigurationManager
{
public:
	//==============================================
	// return singleton
	static AQLObjectConfigurationManager     *getInstance();
	//==============================================
	// create AQLObjectConfiguration
	AQLObjectConfiguration *createEntitySetUpper(const AQLString &model) const;

private:

friend class AQLCoreDataService;
	// constructor
	AQLObjectConfigurationManager(void);
	// destructor
	~AQLObjectConfigurationManager(void);
	// copy constructor
	AQLObjectConfigurationManager(const AQLObjectConfigurationManager &rhs);
	AQLObjectConfigurationManager &operator=(const AQLObjectConfigurationManager &rhs);

	static AQLObjectConfigurationManager *mpInstance; // single instance

};

#endif
