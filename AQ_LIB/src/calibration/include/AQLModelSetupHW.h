/*! @file
    @brief HW Master data regist class
*/
#ifndef  AQLModelSetupHW_h
#define  AQLModelSetupHW_h
#ifdef __GNUG__
#pragma interface
#endif


#include "AQLModelSetupBase.h"

//===================== Class Declare AQLModelSetupHW==================================
/*! 
    @brief Master data regist class for HW

    
*/
class AQLModelSetupHW : public AQLModelSetupBase
{
public:
	// constructor
	AQLModelSetupHW(void);
	// destructor
	virtual ~AQLModelSetupHW(void);
								//======================================
								// regist extra data
	virtual void                registAttrMasterEx(AQLPriceDataManager &dm);
								//======================================
								// regist extra object 
	virtual void                 registEntityMasterEx(AQLDataInstance &dataInstance);
								//======================================
								// regist extra function 
	virtual void                registFunctionMasterEx(AQLDataInstance &dataInstance);

protected:
};


#endif
