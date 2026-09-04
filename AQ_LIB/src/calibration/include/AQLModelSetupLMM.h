/*! @file
    @brief LMM Master data regist class
*/
#ifndef  AQLModelSetupLMM_h
#define  AQLModelSetupLMM_h
#ifdef __GNUG__
#pragma interface
#endif


#include "AQLModelSetupBase.h"


//===================== Class Declare AQLModelSetupLMM==================================
/*! 
    @brief Master data regist class for LMM

    
*/
class AQLModelSetupLMM : public AQLModelSetupBase
{
public:
	// constructor
	AQLModelSetupLMM(void);
	// destructor
	virtual ~AQLModelSetupLMM(void);
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
