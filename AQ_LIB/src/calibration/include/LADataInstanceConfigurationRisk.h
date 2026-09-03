/*! @file
    @brief  DataInstance setup class for calc Risk
*/
//  2007, AlgoQuantHub.
#ifndef LADataInstanceConfigurationRisk_h
#define LADataInstanceConfigurationRisk_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LADataInstanceConfigurationRisk.h
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

#include "LADataInstanceConfigurationPV.h"

//===================== Class Declare LADataInstanceConfigurationRisk==================================
/*! 
    @brief DataInstance setup class for calc Risk
	

*/
class LADataInstanceConfigurationRisk : public LADataInstanceConfigurationPV
{
public:
	// constructor
	explicit LADataInstanceConfigurationRisk(void);
	// destructor
	virtual ~LADataInstanceConfigurationRisk(void);
	// copy constructor
	LADataInstanceConfigurationRisk(const LADataInstanceConfigurationRisk &rhs);
	LADataInstanceConfigurationRisk &operator=(const LADataInstanceConfigurationRisk &rhs);
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
