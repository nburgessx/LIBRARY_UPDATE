/*! @file
    @brief HW Master data regist class
*/
//  2008, AlgoQuantHub.
#ifndef  LAModelSetupHW_h
#define  LAModelSetupHW_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAModelSetupHW.h
//
//  DESCRIPTION :       HW Master data regist class
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif


#include "LAModelSetupBase.h"

//===================== Class Declare LAModelSetupHW==================================
/*! 
    @brief Master data regist class for HW

    
*/
class LAModelSetupHW : public LAModelSetupBase
{
public:
	// constructor
	LAModelSetupHW(void);
	// destructor
	virtual ~LAModelSetupHW(void);
								//======================================
								// regist extra data
	virtual void                registAttrMasterEx(LAPriceDataManager &dm);
								//======================================
								// regist extra object 
	virtual void                 registEntityMasterEx(LADataInstance &dataInstance);
								//======================================
								// regist extra function 
	virtual void                registFunctionMasterEx(LADataInstance &dataInstance);

protected:
};


#endif
