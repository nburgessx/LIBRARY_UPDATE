/*! @file
    @brief  HW object setup class
*/
//  2008, Mizuho International London.
#ifndef LAObjectConfigurationHW_h
#define LAObjectConfigurationHW_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAObjectConfigurationHW.h
//
//  DESCRIPTION :        HW object setup class 
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
//===================== Class Declare LAObjectConfigurationHW==================================
/*! 
    @brief Object setup class
	
	object setup for HW

*/
class LAObjectConfigurationHW : public LAObjectConfiguration
{
public:
	// constructor
	explicit LAObjectConfigurationHW(void);
	// destructor
	virtual ~LAObjectConfigurationHW(void);
	// copy constructor
	LAObjectConfigurationHW(const LAObjectConfigurationHW &rhs);
	LAObjectConfigurationHW &operator=(const LAObjectConfigurationHW &rhs);

protected:
	// get model name
	virtual LAString getModelName() const { return MODEL_HW; }
	//==============================================
	// get sde timegrid
	virtual void getSDETimeGrid(DoubleArray &timeGrid) const;
	//==============================================
	// get sde integral timegrid
	virtual void getSDEIntegralTimeGrid(DoubleArray &timeGrid) const;

};
#endif
