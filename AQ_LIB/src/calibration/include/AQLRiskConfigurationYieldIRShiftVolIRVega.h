/*! @file
    @brief IR vega setup class
*/
//  2008, AlgoQuantHub.
#ifndef AQLRiskConfigurationYieldIRShiftVolIRVega_h
#define AQLRiskConfigurationYieldIRShiftVolIRVega_h
#ifdef __GNUG__
#pragma interface
#endif

#include "AQLRiskConfigurationVolIRVega.h"



class AQLString;
class AQLDataInstance;
class AQLRiskConfigurationVolCreatorLMM;
//===================== Class Declare AQLRiskConfigurationYieldIRShiftVolIRVega==================================
/*! 
    @brief IR Vega setup class
	

*/
class AQLRiskConfigurationYieldIRShiftVolIRVega : public AQLRiskConfigurationVolIRVega
{
public:
	// constructor
	explicit AQLRiskConfigurationYieldIRShiftVolIRVega(void);
	// destructor
	virtual ~AQLRiskConfigurationYieldIRShiftVolIRVega(void);
	// copy constructor
	AQLRiskConfigurationYieldIRShiftVolIRVega(const AQLRiskConfigurationYieldIRShiftVolIRVega &rhs);
	AQLRiskConfigurationYieldIRShiftVolIRVega &operator=(const AQLRiskConfigurationYieldIRShiftVolIRVega &rhs);

protected:
friend class AQLRiskConfigurationVolCreatorLMM;
	//==============================================
	// get outputname1
	virtual  AQLString  getOutPutName1(const AQLString &ccy) const;
	//==============================================
	// get outputname1
	virtual  AQLString  getOutPutName1(const AQLString &ccy, int index) const;
	////==============================================
	// get shift type
	virtual  AQLString  getShiftType(const AQLString &ccy) const;
	//==============================================
	// get bump direction
	virtual  AQLString  getBumpDirection(const AQLString &ccy) const;
	//==============================================
	// get risk name
	virtual  AQLString getRiskName(void) const;
	//==============================================
	// check target currencies
	virtual  AQLString getTargetCurrencies() const;
	//==============================================
	// get calibration target currencies
	virtual  AQLString getCalibTargetCurrencies() const;
	//==============================================
	// isgridsensitivity
	virtual  bool  isGridSensitivity(const AQLString &ccy) const;
	//==============================================
	// isparallelshift
	virtual  bool isParallelShift(const AQLString &ccy) const;
	//==============================================
	// get grid term
	virtual std::vector<AQLString> getMarketTerm(const AQLString &ccy) const;
	//==============================================
	// get bucket grid term
	virtual std::vector<AQLString> getBucketGridTerm(const AQLString &ccy) const;
	//==============================================
	// isfileuse
	virtual  bool isShiftValFileUse(const AQLString &ccy) const;
	//==============================================
	// get scenario1 shift
	virtual double getScenario1ShiftValue(const AQLString &ccy) const;
	//==============================================
	// get scenario2 shift
	virtual double getScenario2ShiftValue(const AQLString &ccy) const;
	//==============================================
	// get divunit
	virtual double getDivUnit(const AQLString &ccy) const;
	//==============================================
	// get double matrix
	virtual DoubleMatrix getCoordinatesMatrix(const AQLString &ccy) const;
	//==============================================
	// get fileval matrix
	virtual DoubleMatrix getFileValMatrix(const AQLString &ccy) const;
	//==============================================
	// get bump type
	virtual  AQLString getBumpType(const AQLString &ccy) const;
	//==============================================
	// get grid type
	virtual  AQLString getGridType(const AQLString &ccy) const;
	//==============================================
	// get grid term
	virtual std::vector<AQLString> getCoordinates(const AQLString &ccy) const;
	//==============================================
	// get is wave
	virtual  bool  isWave(const AQLString &ccy) const;
	//==============================================
	// isRiskCurrencyMode
	virtual bool isRiskCurrencyMode(const AQLString& ccy) const;
		// get get FX shift vals
	virtual  DoubleArray  getBaseShiftVals(const AQLString &ccy) const;
	//==============================================
	// get shift yield vals
	virtual double getBaseYieldVal(const AQLString &ccy, int index) const;
	//==============================================
	// create basescenario object
	virtual std::vector<AQLObject *> createBaseScenarioEntity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const;
	//==============================================
	// create base extraScenario object 
	virtual  std::vector<AQLObject *> createBaseExtraScenarioEntity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const;
	//==============================================
	// get base extra target names 
	virtual  AQLStringVector getBaseExtraTargetNames(const AQLString &ccy, AQLDataInstance &dataInstance) const;
	//==============================================
	// get shift grid term
	virtual std::vector<AQLString> getShiftGridTerm(const AQLString &ccy) const;
	//==============================================
	// getScenarioBaseYieldName
	virtual AQLString getScenarioBaseYieldName(const AQLString& ccy) const;
	//==============================================
	// get baseoutputname
	virtual  AQLString  getBaseOutPutName(const AQLString &ccy, int index) const;
	//==============================================
	// get baseoperator
	virtual  AQLString  getBaseOperator(void) const;
	//==============================================
	// get basecoefficient
	virtual  AQLString  getBaseCoefficient(const AQLString &ccy) const;
	//==============================================
	// get base target names 
	virtual  AQLString getBaseTargetNames(const AQLString &ccy, AQLDataInstance &dataInstance)  const;
	//==============================================
	// get shift vals(bp)
	virtual DoubleArray getBaseShifts(const AQLString &ccy, int index) const;
	//==============================================
	// get IR shift vals
	virtual DoubleArray getIRShiftVals(const AQLString &ccy) const;	
	//==============================================
	// get base chift curveType
	virtual AQLString getBaseShiftCurveType(const AQLString &ccy) const;
	//==============================================
	// get base shift curve suffix
	virtual AQLString getBaseShiftCurveSuffix(const AQLString &ccy) const;
};


#endif
