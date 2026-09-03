/*! @file
    @brief IR vega setup class
*/
//  2008, Mizuho International London.
#ifndef LARiskConfigurationYieldIRShiftVolIRVega_h
#define LARiskConfigurationYieldIRShiftVolIRVega_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationYieldIRShiftVolIRVega.h
//
//  DESCRIPTION :        IR Vega setup class 
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "LARiskConfigurationVolIRVega.h"



class LAString;
class LADataInstance;
class LARiskConfigurationVolCreatorLMM;
//===================== Class Declare LARiskConfigurationYieldIRShiftVolIRVega==================================
/*! 
    @brief IR Vega setup class
	

*/
class LARiskConfigurationYieldIRShiftVolIRVega : public LARiskConfigurationVolIRVega
{
public:
	// constructor
	explicit LARiskConfigurationYieldIRShiftVolIRVega(void);
	// destructor
	virtual ~LARiskConfigurationYieldIRShiftVolIRVega(void);
	// copy constructor
	LARiskConfigurationYieldIRShiftVolIRVega(const LARiskConfigurationYieldIRShiftVolIRVega &rhs);
	LARiskConfigurationYieldIRShiftVolIRVega &operator=(const LARiskConfigurationYieldIRShiftVolIRVega &rhs);

protected:
friend class LARiskConfigurationVolCreatorLMM;
	//==============================================
	// get outputname1
	virtual  LAString  getOutPutName1(const LAString &ccy) const;
	//==============================================
	// get outputname1
	virtual  LAString  getOutPutName1(const LAString &ccy, int index) const;
	////==============================================
	// get shift type
	virtual  LAString  getShiftType(const LAString &ccy) const;
	//==============================================
	// get bump direction
	virtual  LAString  getBumpDirection(const LAString &ccy) const;
	//==============================================
	// get risk name
	virtual  LAString getRiskName(void) const;
	//==============================================
	// check target currencies
	virtual  LAString getTargetCurrencies() const;
	//==============================================
	// get calibration target currencies
	virtual  LAString getCalibTargetCurrencies() const;
	//==============================================
	// isgridsensitivity
	virtual  bool  isGridSensitivity(const LAString &ccy) const;
	//==============================================
	// isparallelshift
	virtual  bool isParallelShift(const LAString &ccy) const;
	//==============================================
	// get grid term
	virtual std::vector<LAString> getMarketTerm(const LAString &ccy) const;
	//==============================================
	// get bucket grid term
	virtual std::vector<LAString> getBucketGridTerm(const LAString &ccy) const;
	//==============================================
	// isfileuse
	virtual  bool isShiftValFileUse(const LAString &ccy) const;
	//==============================================
	// get scenario1 shift
	virtual double getScenario1ShiftValue(const LAString &ccy) const;
	//==============================================
	// get scenario2 shift
	virtual double getScenario2ShiftValue(const LAString &ccy) const;
	//==============================================
	// get divunit
	virtual double getDivUnit(const LAString &ccy) const;
	//==============================================
	// get double matrix
	virtual DoubleMatrix getCoordinatesMatrix(const LAString &ccy) const;
	//==============================================
	// get fileval matrix
	virtual DoubleMatrix getFileValMatrix(const LAString &ccy) const;
	//==============================================
	// get bump type
	virtual  LAString getBumpType(const LAString &ccy) const;
	//==============================================
	// get grid type
	virtual  LAString getGridType(const LAString &ccy) const;
	//==============================================
	// get grid term
	virtual std::vector<LAString> getCoordinates(const LAString &ccy) const;
	//==============================================
	// get is wave
	virtual  bool  isWave(const LAString &ccy) const;
	//==============================================
	// isRiskCurrencyMode
	virtual bool isRiskCurrencyMode(const LAString& ccy) const;
		// get get FX shift vals
	virtual  DoubleArray  getBaseShiftVals(const LAString &ccy) const;
	//==============================================
	// get shift yield vals
	virtual double getBaseYieldVal(const LAString &ccy, int index) const;
	//==============================================
	// create basescenario object
	virtual std::vector<LAObject *> createBaseScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, int index) const;
	//==============================================
	// create base extraScenario object 
	virtual  std::vector<LAObject *> createBaseExtraScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, int index) const;
	//==============================================
	// get base extra target names 
	virtual  LAStringVector getBaseExtraTargetNames(const LAString &ccy, LADataInstance &dataInstance) const;
	//==============================================
	// get shift grid term
	virtual std::vector<LAString> getShiftGridTerm(const LAString &ccy) const;
	//==============================================
	// getScenarioBaseYieldName
	virtual LAString getScenarioBaseYieldName(const LAString& ccy) const;
	//==============================================
	// get baseoutputname
	virtual  LAString  getBaseOutPutName(const LAString &ccy, int index) const;
	//==============================================
	// get baseoperator
	virtual  LAString  getBaseOperator(void) const;
	//==============================================
	// get basecoefficient
	virtual  LAString  getBaseCoefficient(const LAString &ccy) const;
	//==============================================
	// get base target names 
	virtual  LAString getBaseTargetNames(const LAString &ccy, LADataInstance &dataInstance)  const;
	//==============================================
	// get shift vals(bp)
	virtual DoubleArray getBaseShifts(const LAString &ccy, int index) const;
	//==============================================
	// get IR shift vals
	virtual DoubleArray getIRShiftVals(const LAString &ccy) const;	
	//==============================================
	// get base chift curveType
	virtual LAString getBaseShiftCurveType(const LAString &ccy) const;
	//==============================================
	// get base shift curve suffix
	virtual LAString getBaseShiftCurveSuffix(const LAString &ccy) const;
};


#endif
