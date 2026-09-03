/*! @file
    @brief Theta risk setup class
*/
//  2007, Mizuho International London.
#ifndef LARiskConfigurationTheta_h
#define LARiskConfigurationTheta_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationTheta.h
//
//  DESCRIPTION :       Theta risk setup class
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "LARiskConfiguration.h"

//===================== Class Declare LARiskConfigurationTheta==================================
/*! 
    @brief Theta risk setup class
	
	this class is abstract

*/
class LARiskConfigurationTheta : public LARiskConfiguration
{
public:
	// constructor
	explicit LARiskConfigurationTheta(void);
	// destructor
	virtual ~LARiskConfigurationTheta(void);
	// copy constructor
	LARiskConfigurationTheta(const LARiskConfigurationTheta &rhs);
	LARiskConfigurationTheta &operator=(const LARiskConfigurationTheta &rhs);

protected:
	//==============================================
	// create risk object
	virtual  std::vector<std::pair<LAString, std::vector<LAObject *> > > createRiskEntity(LAObjectPool &objPool) const;
	//==============================================
	// setup target names 
	virtual  void setUpTargetNames(const LAString &ccy, LAObject &e, LADataInstance &dataInstance) const;
	//==============================================
	// create scenario1 object 
	virtual  std::vector<LAObject *> createScenario1Entity(const LAString &ccy, LADataInstance &dataInstance, int index) const;
	//==============================================
	// create scenario2 object
	virtual  std::vector<LAObject *> createScenario2Entity(const LAString &ccy, LADataInstance &dataInstance, int index) const;
	//==============================================
	// create scenario yield object
	virtual std::vector<LAObject *> createThetaEntity(const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;
	//==============================================
	// return risk name
	virtual  LAString  getRiskName(void) const;
	//==============================================
	// get bump direction
	virtual  LAString getBumpDirection(const LAString &ccy) const;
	//==============================================
	// get shift value
	virtual double getShiftVal(const LAString &ccy, SCENARIONUM scenarioNum) const;
	//==============================================
	// get divunit
	virtual double getDivUnit(const LAString &fx) const;
	//==============================================
	// get outputname1
	virtual  LAString  getOutPutName1(const LAString &ccy) const;
	//==============================================
	// get target names 
	virtual  LAString getTargetNames(const LAString &ccy, LADataInstance &dataInstance)  const {ccy; return MLIB_NO_DATA;};
	//==============================================
	// isgridsensitivity
	virtual  bool isGridSensitivity(const LAString &ccy) const  {ccy; return false;};
	//==============================================
	// isparallelshift
	virtual  bool isParallelShift(const LAString &ccy) const {ccy; return false;};
	//==============================================
	// get grid term
	virtual std::vector<LAString> getGridTerm(const LAString &ccy) const { ccy;  std::vector<LAString> ret; return ret; };
	//==============================================
	// get bucket grid term
	virtual std::vector<LAString> getBucketGridTerm(const LAString &ccy) const { ccy; std::vector<LAString> ret; ret.push_back(MLIB_NO_DATA); return ret; };
	// isSavePastFixing
	virtual bool isSavePastFixing() const { return true; };
	//==============================================
	// isRiskCurrencyMode
	virtual bool isRiskCurrencyMode(const LAString& fx) const;
	//==============================================
	// isPLChangeMode
	virtual bool isPLChangeMode(const LAString &fx) const;

};
#endif
