/*! @file
    @brief  DataInstance setup class
*/
//  2007, AlgoQuantHub.
#ifndef AQLDataInstanceConfiguration_h
#define AQLDataInstanceConfiguration_h
#ifdef __GNUG__
#pragma interface
#endif



class AQLDataInstance;
//===================== Class Declare AQLDataInstanceConfiguration==================================
/*! 
    @brief DataInstance setup class
	
	this class is abstract and facade

*/
class AQLDataInstanceConfiguration
{
public:
	// constructor
	explicit AQLDataInstanceConfiguration(void);
	// destructor
	virtual ~AQLDataInstanceConfiguration(void);
	// copy constructor
	AQLDataInstanceConfiguration(const AQLDataInstanceConfiguration &rhs);
	AQLDataInstanceConfiguration &operator=(const AQLDataInstanceConfiguration &rhs);

	//==============================================
	// setup dataInstance
	virtual void setUpDataInstance(AQLDataInstance &dataInstance) const;
	//==============================================
	// setup this instance
	virtual void setUp(void) = 0;
	//==============================================
	// setup master object
	virtual void setUpMasters(AQLDataInstance &dataInstance) const = 0;
	//==============================================
	// setup sde
	virtual void setUpSDE(AQLDataInstance &dataInstance) const = 0;
	//==============================================
	// load entities
	virtual void loadEntities(AQLDataInstance &dataInstance) const = 0;
	//==============================================
	// setup  entities
	virtual void setUpEntityes(AQLDataInstance &dataInstance) const = 0;
	//==============================================
	// setup  risk entities
	virtual void setUpRiskEntityes(AQLDataInstance &dataInstance) const = 0;
	//==============================================
	// create path object
	virtual void createPathEntity(AQLDataInstance &dataInstance) const = 0;
	//==============================================
	// create fx object
	virtual void createFXEntity(AQLDataInstance &dataInstance) const = 0;
	//==============================================
	// setup path object
	virtual void setUpPathEntity(AQLDataInstance &dataInstance) const = 0;
	//==============================================
	// setup fx object
	virtual void setUpFXEntity(AQLDataInstance &dataInstance) const = 0;
	//==============================================
	// setup risk info 
	virtual void setUpRiskInfo(AQLDataInstance &dataInstance) const = 0;

protected:

};
#endif
