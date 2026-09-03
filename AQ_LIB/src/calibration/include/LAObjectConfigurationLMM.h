/*! @file
    @brief  LMM object setup class
*/
//  2007, Mizuho International London.
#ifndef LAObjectConfigurationLMM_h
#define LAObjectConfigurationLMM_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAObjectConfigurationLMM.h
//
//  DESCRIPTION :        LMM object setup class 
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "LAObjectConfiguration.h"
#include "LADefinitions.h"


class LAObjectPool;
class LAStaticData;

//===================== Class Declare LAObjectConfigurationLMM==================================
/*! 
    @brief Object setup class
	
	object setup for LMM

*/
class LAObjectConfigurationLMM : public LAObjectConfiguration
{
public:
	// constructor
	explicit LAObjectConfigurationLMM(void);
	// destructor
	virtual ~LAObjectConfigurationLMM(void);
	// copy constructor
	LAObjectConfigurationLMM(const LAObjectConfigurationLMM &rhs);
	LAObjectConfigurationLMM &operator=(const LAObjectConfigurationLMM &rhs);

protected:
	//==============================================
	// get model name
	virtual LAString getModelName() const { return MODEL_LMM; }
	//==============================================
	// get sde timegrid
	virtual void getSDETimeGrid(DoubleArray &timeGrid) const;
	//==============================================
	// get sde integral timegrid
	virtual void getSDEIntegralTimeGrid(DoubleArray &timeGrid) const;

};
#endif
