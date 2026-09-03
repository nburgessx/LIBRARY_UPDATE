/*! @file
    @brief LMM Master data regist class
*/
//  2007, AlgoQuantHub.
#ifndef  AQLModelSetupLMM_h
#define  AQLModelSetupLMM_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLModelSetupLMM.h
//
//  DESCRIPTION :       LMM Master data regist class
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
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
