/*! @file
    @brief Vanilla Master data regist class
*/
//  2008, AlgoQuantHub.
#ifndef  AQLModelSetupVanilla_h
#define  AQLModelSetupVanilla_h
#ifdef __GNUG__
#pragma interface
#endif


#include "AQLModelSetupBase.h"

//===================== Class Declare AQLModelSetupVanilla==================================
/*! 
    @brief Master data regist class for Vanilla

    
*/
class AQLModelSetupVanilla : public AQLModelSetupBase
{
public:
	// constructor
	AQLModelSetupVanilla(void);
	// destructor
	virtual ~AQLModelSetupVanilla(void);
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
