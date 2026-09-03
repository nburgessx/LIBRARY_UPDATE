/*! @file
    @brief Master data regist class
*/
//  2007, AlgoQuantHub.
#ifndef  LAModelSetup_h
#define  LAModelSetup_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAModelSetupBase.h
//
//  DESCRIPTION :       Master data regist class
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif



class LADataInstance;
class LAPriceDataManager;

//===================== Class Declare LAModelSetupBase==================================
/*! 
    @brief Master data regist base class

    This class is abstract
*/
class LAModelSetupBase
{
public:
	// constructor
	LAModelSetupBase();
	// destructor
	virtual ~LAModelSetupBase(void);
								//======================================
								// regist master data to dataInstance object
	void                          registMaster(LADataInstance &dataInstance);
								//======================================
	                            // extra regist method
	                            // sub class override this method
								//======================================
								// regist extra data
	virtual void                 registAttrMasterEx(LAPriceDataManager &dm) = 0;
								//======================================
								// regist extra object 
	virtual void                 registEntityMasterEx(LADataInstance &dataInstance) = 0;
								//======================================
								// regist extra function 
	virtual void                 registFunctionMasterEx(LADataInstance &dataInstance) = 0;		

protected:
								//======================================
								// regist data 
	void                          registAttrMaster(LAPriceDataManager &dm);
								//======================================
								// regist object 
	void                           registEntityMaster(LADataInstance &dataInstance);
								//======================================
								// regist function 
	virtual void                          registFunctionMaster(LADataInstance &dataInstance);



};

#endif