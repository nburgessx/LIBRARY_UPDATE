/*! @file
    @brief  LMM object setup class
*/
#ifndef AQLObjectConfigurationLMM_h
#define AQLObjectConfigurationLMM_h
#ifdef __GNUG__
#pragma interface
#endif

#include "AQLObjectConfiguration.h"
#include "AQLDefinitions.h"


class AQLObjectPool;
class AQLStaticData;

//===================== Class Declare AQLObjectConfigurationLMM==================================
/*! 
    @brief Object setup class
	
	object setup for LMM

*/
class AQLObjectConfigurationLMM : public AQLObjectConfiguration
{
public:
	// constructor
	explicit AQLObjectConfigurationLMM(void);
	// destructor
	virtual ~AQLObjectConfigurationLMM(void);
	// copy constructor
	AQLObjectConfigurationLMM(const AQLObjectConfigurationLMM &rhs);
	AQLObjectConfigurationLMM &operator=(const AQLObjectConfigurationLMM &rhs);

protected:
	//==============================================
	// get model name
	virtual AQLString getModelName() const { return MODEL_LMM; }
	//==============================================
	// get sde timegrid
	virtual void getSDETimeGrid(DoubleArray &timeGrid) const;
	//==============================================
	// get sde integral timegrid
	virtual void getSDEIntegralTimeGrid(DoubleArray &timeGrid) const;

};
#endif
