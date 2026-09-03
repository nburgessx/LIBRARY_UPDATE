/*! @file
    @brief  DataInstance setup class for calc PV
*/
//  2007, Mizuho International London.
#ifndef LADataInstanceConfigurationPV_h
#define LADataInstanceConfigurationPV_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LADataInstanceConfigurationPV.h
//
//  DESCRIPTION :        DataInstance setup class for calc PV
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "LACoreTemplateType.h"
#include "LADataInstanceConfiguration.h"


class LADataInstance;
class LAObject;
class LADate;

//===================== Class Declare LADataInstanceConfigurationPV==================================
/*! 
    @brief DataInstance setup class for calc PV
	

*/
class LADataInstanceConfigurationPV : public LADataInstanceConfiguration
{
public:
	// constructor
	explicit LADataInstanceConfigurationPV(void);
	// destructor
	virtual ~LADataInstanceConfigurationPV(void);
	// copy constructor
	LADataInstanceConfigurationPV(const LADataInstanceConfigurationPV &rhs);
	LADataInstanceConfigurationPV &operator=(const LADataInstanceConfigurationPV &rhs);
	//==============================================
	// setup this instance
	virtual void setUp(void);

	//==============================================
	// setup master object
	virtual void setUpMasters(LADataInstance &dataInstance) const;
	//==============================================
	// setup sde
	virtual void setUpSDE(LADataInstance &dataInstance) const;
	//==============================================
	// load entities
	virtual void loadEntities(LADataInstance &dataInstance) const;
	//==============================================
	// load entities
	virtual void loadCSV(LADataInstance &dataInstance) const;
	//==============================================
	// setup entities
	virtual void setUpEntityes(LADataInstance &dataInstance) const;
	//==============================================
	// setup  risk entities
	virtual void setUpRiskEntityes(LADataInstance &dataInstance) const;
	//==============================================
	// create path object
	virtual void createPathEntity(LADataInstance &dataInstance) const;
	//==============================================
	// create fx object
	virtual void createFXEntity(LADataInstance &dataInstance) const;
	//==============================================
	// setup path object
	virtual void setUpPathEntity(LADataInstance &dataInstance) const;
	//==============================================
	// setup fx object
	virtual void setUpFXEntity(LADataInstance &dataInstance) const;
	//==============================================
	// setup risk info
	virtual void setUpRiskInfo(LADataInstance &dataInstance) const;

protected:

	LAStringVector mSDECurrencys; // currency vector
	LAStringVector mSDEModels;    // model vector for currency
	LAString mPathEntityName;	// PATH1 or MARKETPARAM

    void shiftPaymentDate(LAObject& trade, const LADate& asof1, const LADate& asof2) const;
    void shiftPaymentDate(LADataInstance& dataInstance) const;
    LADate getAsOfDate(LADataInstance& dataInstance) const;
    void insertContext(LAStringMatrix& m, LADataInstance& dataInstance) const;
    void setupFundingChangeInfo(LADataInstance& dataInstance) const;
};
#endif
