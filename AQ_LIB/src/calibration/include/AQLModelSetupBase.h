/*! @file
    @brief Master data regist class
*/
//  2007, AlgoQuantHub.
#ifndef  AQLModelSetup_h
#define  AQLModelSetup_h
#ifdef __GNUG__
#pragma interface
#endif



class AQLDataInstance;
class AQLPriceDataManager;

//===================== Class Declare AQLModelSetupBase==================================
/*! 
    @brief Master data regist base class

    This class is abstract
*/
class AQLModelSetupBase
{
public:
	// constructor
	AQLModelSetupBase();
	// destructor
	virtual ~AQLModelSetupBase(void);
								//======================================
								// regist master data to dataInstance object
	void                          registMaster(AQLDataInstance &dataInstance);
								//======================================
	                            // extra regist method
	                            // sub class override this method
								//======================================
								// regist extra data
	virtual void                 registAttrMasterEx(AQLPriceDataManager &dm) = 0;
								//======================================
								// regist extra object 
	virtual void                 registEntityMasterEx(AQLDataInstance &dataInstance) = 0;
								//======================================
								// regist extra function 
	virtual void                 registFunctionMasterEx(AQLDataInstance &dataInstance) = 0;		

protected:
								//======================================
								// regist data 
	void                          registAttrMaster(AQLPriceDataManager &dm);
								//======================================
								// regist object 
	void                           registEntityMaster(AQLDataInstance &dataInstance);
								//======================================
								// regist function 
	virtual void                          registFunctionMaster(AQLDataInstance &dataInstance);



};

#endif