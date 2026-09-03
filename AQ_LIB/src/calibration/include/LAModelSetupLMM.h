/*! @file
    @brief LMM Master data regist class
*/
//  2007, Mizuho International London.
#ifndef  LAModelSetupLMM_h
#define  LAModelSetupLMM_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAModelSetupLMM.h
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


#include "LAModelSetupBase.h"


//===================== Class Declare LAModelSetupLMM==================================
/*! 
    @brief Master data regist class for LMM

    
*/
class LAModelSetupLMM : public LAModelSetupBase
{
public:
	// constructor
	LAModelSetupLMM(void);
	// destructor
	virtual ~LAModelSetupLMM(void);
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
