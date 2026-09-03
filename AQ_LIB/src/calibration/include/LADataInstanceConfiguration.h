/*! @file
    @brief  DataInstance setup class
*/
//  2007, AlgoQuantHub.
#ifndef LADataInstanceConfiguration_h
#define LADataInstanceConfiguration_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LADataInstanceConfiguration.h
//
//  DESCRIPTION :        DataInstance setup class 
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif



class LADataInstance;
//===================== Class Declare LADataInstanceConfiguration==================================
/*! 
    @brief DataInstance setup class
	
	this class is abstract and facade

*/
class LADataInstanceConfiguration
{
public:
	// constructor
	explicit LADataInstanceConfiguration(void);
	// destructor
	virtual ~LADataInstanceConfiguration(void);
	// copy constructor
	LADataInstanceConfiguration(const LADataInstanceConfiguration &rhs);
	LADataInstanceConfiguration &operator=(const LADataInstanceConfiguration &rhs);

	//==============================================
	// setup dataInstance
	virtual void setUpDataInstance(LADataInstance &dataInstance) const;
	//==============================================
	// setup this instance
	virtual void setUp(void) = 0;
	//==============================================
	// setup master object
	virtual void setUpMasters(LADataInstance &dataInstance) const = 0;
	//==============================================
	// setup sde
	virtual void setUpSDE(LADataInstance &dataInstance) const = 0;
	//==============================================
	// load entities
	virtual void loadEntities(LADataInstance &dataInstance) const = 0;
	//==============================================
	// setup  entities
	virtual void setUpEntityes(LADataInstance &dataInstance) const = 0;
	//==============================================
	// setup  risk entities
	virtual void setUpRiskEntityes(LADataInstance &dataInstance) const = 0;
	//==============================================
	// create path object
	virtual void createPathEntity(LADataInstance &dataInstance) const = 0;
	//==============================================
	// create fx object
	virtual void createFXEntity(LADataInstance &dataInstance) const = 0;
	//==============================================
	// setup path object
	virtual void setUpPathEntity(LADataInstance &dataInstance) const = 0;
	//==============================================
	// setup fx object
	virtual void setUpFXEntity(LADataInstance &dataInstance) const = 0;
	//==============================================
	// setup risk info 
	virtual void setUpRiskInfo(LADataInstance &dataInstance) const = 0;

protected:

};
#endif
