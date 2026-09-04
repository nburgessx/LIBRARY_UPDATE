/*! @file
    @brief  DataInstance setup class for calc PV
*/
//  2007, AlgoQuantHub.
#ifndef AQLDataInstanceConfigurationPV_h
#define AQLDataInstanceConfigurationPV_h
#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreTemplateType.h"
#include "AQLDataInstanceConfiguration.h"


class AQLDataInstance;
class AQLObject;
class AQLDate;

//===================== Class Declare AQLDataInstanceConfigurationPV==================================
/*! 
    @brief DataInstance setup class for calc PV
	

*/
class AQLDataInstanceConfigurationPV : public AQLDataInstanceConfiguration
{
public:
	// constructor
	explicit AQLDataInstanceConfigurationPV(void);
	// destructor
	virtual ~AQLDataInstanceConfigurationPV(void);
	// copy constructor
	AQLDataInstanceConfigurationPV(const AQLDataInstanceConfigurationPV &rhs);
	AQLDataInstanceConfigurationPV &operator=(const AQLDataInstanceConfigurationPV &rhs);
	//==============================================
	// setup this instance
	virtual void setUp(void);

	//==============================================
	// setup master object
	virtual void setUpMasters(AQLDataInstance &dataInstance) const;
	//==============================================
	// setup sde
	virtual void setUpSDE(AQLDataInstance &dataInstance) const;
	//==============================================
	// load entities
	virtual void loadEntities(AQLDataInstance &dataInstance) const;
	//==============================================
	// load entities
	virtual void loadCSV(AQLDataInstance &dataInstance) const;
	//==============================================
	// setup entities
	virtual void setUpEntityes(AQLDataInstance &dataInstance) const;
	//==============================================
	// setup  risk entities
	virtual void setUpRiskEntityes(AQLDataInstance &dataInstance) const;
	//==============================================
	// create path object
	virtual void createPathEntity(AQLDataInstance &dataInstance) const;
	//==============================================
	// create fx object
	virtual void createFXEntity(AQLDataInstance &dataInstance) const;
	//==============================================
	// setup path object
	virtual void setUpPathEntity(AQLDataInstance &dataInstance) const;
	//==============================================
	// setup fx object
	virtual void setUpFXEntity(AQLDataInstance &dataInstance) const;
	//==============================================
	// setup risk info
	virtual void setUpRiskInfo(AQLDataInstance &dataInstance) const;

protected:

	AQLStringVector mSDECurrencys; // currency vector
	AQLStringVector mSDEModels;    // model vector for currency
	AQLString mPathEntityName;	// PATH1 or MARKETPARAM

    void shiftPaymentDate(AQLObject& trade, const AQLDate& asof1, const AQLDate& asof2) const;
    void shiftPaymentDate(AQLDataInstance& dataInstance) const;
    AQLDate getAsOfDate(AQLDataInstance& dataInstance) const;
    void insertContext(AQLStringMatrix& m, AQLDataInstance& dataInstance) const;
    void setupFundingChangeInfo(AQLDataInstance& dataInstance) const;
};
#endif
