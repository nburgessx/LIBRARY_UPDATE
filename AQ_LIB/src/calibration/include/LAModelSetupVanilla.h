/*! @file
    @brief Vanilla Master data regist class
*/
//  2008, AlgoQuantHub.
#ifndef  LAModelSetupVanilla_h
#define  LAModelSetupVanilla_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAModelSetupVanilla.h
//
//  DESCRIPTION :       Vanilla Master data regist class
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif


#include "LAModelSetupBase.h"

//===================== Class Declare LAModelSetupVanilla==================================
/*! 
    @brief Master data regist class for Vanilla

    
*/
class LAModelSetupVanilla : public LAModelSetupBase
{
public:
	// constructor
	LAModelSetupVanilla(void);
	// destructor
	virtual ~LAModelSetupVanilla(void);
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
								//======================================
								// regist function 
	virtual void                registFunctionMaster(AQLDataInstance &dataInstance);
};


#endif
