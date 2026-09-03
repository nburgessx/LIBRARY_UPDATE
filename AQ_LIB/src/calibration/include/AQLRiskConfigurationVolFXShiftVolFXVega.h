/*! @file
    @brief FX vega setup class
*/
//  2008, AlgoQuantHub.
#ifndef AQLRiskConfigurationVolFXShiftVolFXVega_h
#define AQLRiskConfigurationVolFXShiftVolFXVega_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLRiskConfigurationVolFXShiftVolFXVega.h
//
//  DESCRIPTION :        FX Vega setup class 
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "AQLRiskConfigurationVolFXVega.h"



class AQLString;
class AQLDataInstance;
class AQLRiskConfigurationVolCreatorLMM;
//===================== Class Declare AQLRiskConfigurationVolFXShiftVolFXVega==================================
/*! 
    @brief FX Vega setup class
	

*/
class AQLRiskConfigurationVolFXShiftVolFXVega : public AQLRiskConfigurationVolFXVega
{
public:
	// constructor
	explicit AQLRiskConfigurationVolFXShiftVolFXVega(void);
	// destructor
	virtual ~AQLRiskConfigurationVolFXShiftVolFXVega(void);
	// copy constructor
	AQLRiskConfigurationVolFXShiftVolFXVega(const AQLRiskConfigurationVolFXShiftVolFXVega &rhs);
	AQLRiskConfigurationVolFXShiftVolFXVega &operator=(const AQLRiskConfigurationVolFXShiftVolFXVega &rhs);

protected:
friend class AQLRiskConfigurationVolCreatorLMM;
	//==============================================
	// get outputname1
	virtual  AQLString  getOutPutName1(const AQLString &fx) const;
	//==============================================
	// get outputname1
	virtual  AQLString  getOutPutName1(const AQLString &ccy, int index) const;
	//==============================================
	// get shift type
	virtual  AQLString  getShiftType(const AQLString &fx) const;
	//==============================================
	// get bump direction
	virtual  AQLString  getBumpDirection(const AQLString &fx) const;
	//==============================================
	// get risk name
	virtual  AQLString getRiskName(void) const;
	//==============================================
	// check target currencies
	virtual  AQLString getTargetCurrencies() const;
	//==============================================
	// isgridsensitivity
	virtual  bool  isGridSensitivity(const AQLString &fx) const;
	//==============================================
	// isparallelshift
	virtual  bool isParallelShift(const AQLString &fx) const;
	//==============================================
	// get deltatype
	virtual AQLString getDeltaType(const AQLString &ccy) const;
	//==============================================
	// get grid term
	virtual std::vector<AQLString> getGridTerm(const AQLString &fx) const;
	//==============================================
	// get bucket grid term
	virtual std::vector<AQLString> getBucketGridTerm(const AQLString &fx) const;
	//==============================================
	// get property bucket grid term
	virtual AQLString getPropertyBucketGridTerm(const AQLString &fx) const;
	//==============================================
	// get divunit
	virtual double getDivUnit(const AQLString &fx) const;
	//==============================================
	// get is wave
	virtual  bool  isWave(const AQLString &fx) const;
	//==============================================
	// get scenario1 shift
	virtual double getScenario1ShiftValue(const AQLString &fx) const;
	//==============================================
	// get get shift vals
	virtual  DoubleArray  getBaseShiftVals(const AQLString &ccy) const;
	//==============================================
	// get base shift val
	virtual  double getBaseVolVal(const AQLString &ccy, int index) const;
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
	// create basescenario object
	virtual std::vector<AQLObject *> createBaseScenarioEntity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const;
	//==============================================
	// isRiskCurrencyMode
	virtual bool isRiskCurrencyMode(const AQLString& fx) const;
	


};


#endif
