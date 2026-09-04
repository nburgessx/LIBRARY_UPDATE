/*! @file
    @brief IR Shift Delta setup class
*/
//  2008, AlgoQuantHub.
#ifndef AQLRiskConfigurationYieldIRShiftDelta_h
#define AQLRiskConfigurationYieldIRShiftDelta_h
#ifdef __GNUG__
#pragma interface
#endif


#include "AQLRiskConfigurationYieldIRDelta.h"

//===================== Class Declare AQLRiskConfigurationYieldIRShiftDelta==================================
/*! 
    @brief IR Shift PV setup class
	

*/
class AQLRiskConfigurationYieldIRShiftDelta : public AQLRiskConfigurationYieldIRDelta
{
public:
	// constructor
	explicit AQLRiskConfigurationYieldIRShiftDelta(void);
	// destructor
	virtual ~AQLRiskConfigurationYieldIRShiftDelta(void);
	// copy constructor
	AQLRiskConfigurationYieldIRShiftDelta(const AQLRiskConfigurationYieldIRShiftDelta &rhs);
	AQLRiskConfigurationYieldIRShiftDelta &operator=(const AQLRiskConfigurationYieldIRShiftDelta &rhs);

protected:
	//==============================================
	// create basescenario object
	virtual std::vector<AQLObject *> createBaseScenarioEntity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const; 
	//==============================================
	// get baseoperator
	virtual  AQLString  getBaseOperator(void) const;	
	//==============================================
	// get basecoefficient
	virtual  AQLString  getBaseCoefficient(const AQLString &ccy) const;
	//==============================================
	// get baseoutputname
	virtual  AQLString  getBaseOutPutName(const AQLString &ccy, int index) const;
	//==============================================
	// get baseshift string
	virtual  AQLString  getBaseShiftStr(const AQLString &ccy, int index) const;
	//==============================================
	// get outputname1
	virtual  AQLString  getOutPutName1(const AQLString &ccy) const;
	//==============================================
	// get outputname1
	virtual  AQLString  getOutPutName1(const AQLString &ccy, int index) const;
	//==============================================
	// isgridsensitivity
	virtual  bool isGridSensitivity(const AQLString &ccy) const;
	//==============================================
	// isparallelshift
	virtual  bool isParallelShift(const AQLString &ccy) const;
	//==============================================
	// create risk object
	virtual  std::vector<std::pair<AQLString, std::vector<AQLObject *> > > createRiskEntity(AQLObjectPool &objPool) const;
	//==============================================
	// return risk name
	virtual  AQLString  getRiskName(void) const;
	//==============================================
	// get shift vals(bp)
	virtual DoubleArray getBaseShifts(const AQLString &ccy, int index) const;
	//==============================================
	// get shift grid term
	virtual std::vector<AQLString> getShiftGridTerm(const AQLString &ccy) const;
	//==============================================
	// get scenario1 parallel shift str
	virtual AQLString getScenario1ParallelShiftStr(const AQLString &ccy) const;
	//==============================================
	// get scenario1 grid shift
	virtual AQLString getScenario1GridShiftStr(const AQLString &ccy) const;
	//==============================================
	// get shift type
	virtual  AQLString getShiftType(const AQLString &ccy) const;
	//==============================================
	// get bump direction
	virtual  AQLString getBumpDirection(const AQLString &ccy) const;
	//==============================================
	// get is wave
	virtual  bool  isWave(const AQLString &ccy) const;
	//==============================================
	// get target currency
	virtual  AQLString getTargetCurrencies() const;
	//==============================================
	// get calibration target currencies
	virtual  AQLString getCalibTargetCurrencies() const;
	//==============================================
	// get divid one unit
	virtual  double getDivUnit(const AQLString &ccy) const;
	//==============================================
	// get isirshiftscenario
	virtual  bool  isIRShiftScenario(const AQLString &ccy) const;
	//==============================================
	// get get IR shift vals
	virtual  DoubleArray  getIRShiftVals(const AQLString &ccy) const;
	//==============================================
	// get get IR shift scenario vals
	virtual  DoubleArray  getIRShiftScenarioVals(const AQLString &ccy) const;
	//==============================================
	// get grid calc buffer
	virtual int getGridCalcBuffer() const;
	//==============================================
	// get base extra target names 
	virtual  AQLStringVector getBaseExtraTargetNames(const AQLString &ccy, AQLDataInstance &dataInstance) const;
	//==============================================
	// create base extraScenario object 
	virtual  std::vector<AQLObject *> createBaseExtraScenarioEntity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const;
	//==============================================
	// get bucket grid term
	virtual std::vector<AQLString> getBucketGridTerm(const AQLString &ccy) const;
	//==============================================
	// get property bucket grid term
	virtual AQLString getPropertyBucketGridTerm(const AQLString &ccy) const;
	//==============================================
	// isRiskCurrencyMode
	virtual bool isRiskCurrencyMode(const AQLString& ccy) const;
	//==============================================
	// is zero rate bump
	virtual bool isZeroBump(const AQLString& ccy) const;

};
#endif
