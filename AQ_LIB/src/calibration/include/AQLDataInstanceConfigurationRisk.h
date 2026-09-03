/*! @file
    @brief  DataInstance setup class for calc Risk
*/
//  2007, AlgoQuantHub.
#ifndef AQLDataInstanceConfigurationRisk_h
#define AQLDataInstanceConfigurationRisk_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLDataInstanceConfigurationRisk.h
//
//  DESCRIPTION :        DataInstance setup class for calc Risk
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "AQLDataInstanceConfigurationPV.h"

//===================== Class Declare AQLDataInstanceConfigurationRisk==================================
/*! 
    @brief DataInstance setup class for calc Risk
	

*/
class AQLDataInstanceConfigurationRisk : public AQLDataInstanceConfigurationPV
{
public:
	// constructor
	explicit AQLDataInstanceConfigurationRisk(void);
	// destructor
	virtual ~AQLDataInstanceConfigurationRisk(void);
	// copy constructor
	AQLDataInstanceConfigurationRisk(const AQLDataInstanceConfigurationRisk &rhs);
	AQLDataInstanceConfigurationRisk &operator=(const AQLDataInstanceConfigurationRisk &rhs);
	//==============================================
	// setup this instance
	virtual void setUp(void);
	//==============================================
	// setup entities
	virtual void setUpEntityes(AQLDataInstance &dataInstance) const;
	//==============================================
	// setup  risk entities
	virtual void setUpRiskEntityes(AQLDataInstance &dataInstance) const;

protected:


};
#endif
