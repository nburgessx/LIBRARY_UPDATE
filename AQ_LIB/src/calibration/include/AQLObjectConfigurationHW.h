/*! @file
    @brief  HW object setup class
*/
//  2008, AlgoQuantHub.
#ifndef AQLObjectConfigurationHW_h
#define AQLObjectConfigurationHW_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLObjectConfigurationHW.h
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


#include "AQLObjectConfiguration.h"
#include "AQLDefinitions.h"


class AQLObjectPool;
class AQLStaticData;
//===================== Class Declare AQLObjectConfigurationHW==================================
/*! 
    @brief Object setup class
	
	object setup for HW

*/
class AQLObjectConfigurationHW : public AQLObjectConfiguration
{
public:
	// constructor
	explicit AQLObjectConfigurationHW(void);
	// destructor
	virtual ~AQLObjectConfigurationHW(void);
	// copy constructor
	AQLObjectConfigurationHW(const AQLObjectConfigurationHW &rhs);
	AQLObjectConfigurationHW &operator=(const AQLObjectConfigurationHW &rhs);

protected:
	// get model name
	virtual AQLString getModelName() const { return MODEL_HW; }
	//==============================================
	// get sde timegrid
	virtual void getSDETimeGrid(DoubleArray &timeGrid) const;
	//==============================================
	// get sde integral timegrid
	virtual void getSDEIntegralTimeGrid(DoubleArray &timeGrid) const;

};
#endif
