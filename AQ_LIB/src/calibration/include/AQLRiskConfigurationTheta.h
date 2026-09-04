/*! @file
    @brief Theta risk setup class
*/
//  2007, AlgoQuantHub.
#ifndef AQLRiskConfigurationTheta_h
#define AQLRiskConfigurationTheta_h
#ifdef __GNUG__
#pragma interface
#endif

#include "AQLRiskConfiguration.h"

//===================== Class Declare AQLRiskConfigurationTheta==================================
/*! 
    @brief Theta risk setup class
	
	this class is abstract

*/
class AQLRiskConfigurationTheta : public AQLRiskConfiguration
{
public:
	// constructor
	explicit AQLRiskConfigurationTheta(void);
	// destructor
	virtual ~AQLRiskConfigurationTheta(void);
	// copy constructor
	AQLRiskConfigurationTheta(const AQLRiskConfigurationTheta &rhs);
	AQLRiskConfigurationTheta &operator=(const AQLRiskConfigurationTheta &rhs);

protected:
	//==============================================
	// create risk object
	virtual  std::vector<std::pair<AQLString, std::vector<AQLObject *> > > createRiskEntity(AQLObjectPool &objPool) const;
	//==============================================
	// setup target names 
	virtual  void setUpTargetNames(const AQLString &ccy, AQLObject &e, AQLDataInstance &dataInstance) const;
	//==============================================
	// create scenario1 object 
	virtual  std::vector<AQLObject *> createScenario1Entity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const;
	//==============================================
	// create scenario2 object
	virtual  std::vector<AQLObject *> createScenario2Entity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const;
	//==============================================
	// create scenario yield object
	virtual std::vector<AQLObject *> createThetaEntity(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;
	//==============================================
	// return risk name
	virtual  AQLString  getRiskName(void) const;
	//==============================================
	// get bump direction
	virtual  AQLString getBumpDirection(const AQLString &ccy) const;
	//==============================================
	// get shift value
	virtual double getShiftVal(const AQLString &ccy, SCENARIONUM scenarioNum) const;
	//==============================================
	// get divunit
	virtual double getDivUnit(const AQLString &fx) const;
	//==============================================
	// get outputname1
	virtual  AQLString  getOutPutName1(const AQLString &ccy) const;
	//==============================================
	// get target names 
	virtual  AQLString getTargetNames(const AQLString &ccy, AQLDataInstance &dataInstance)  const {ccy; return AQ_NO_DATA;};
	//==============================================
	// isgridsensitivity
	virtual  bool isGridSensitivity(const AQLString &ccy) const  {ccy; return false;};
	//==============================================
	// isparallelshift
	virtual  bool isParallelShift(const AQLString &ccy) const {ccy; return false;};
	//==============================================
	// get grid term
	virtual std::vector<AQLString> getGridTerm(const AQLString &ccy) const { ccy;  std::vector<AQLString> ret; return ret; };
	//==============================================
	// get bucket grid term
	virtual std::vector<AQLString> getBucketGridTerm(const AQLString &ccy) const { ccy; std::vector<AQLString> ret; ret.push_back(AQ_NO_DATA); return ret; };
	// isSavePastFixing
	virtual bool isSavePastFixing() const { return true; };
	//==============================================
	// isRiskCurrencyMode
	virtual bool isRiskCurrencyMode(const AQLString& fx) const;
	//==============================================
	// isPLChangeMode
	virtual bool isPLChangeMode(const AQLString &fx) const;

};
#endif
