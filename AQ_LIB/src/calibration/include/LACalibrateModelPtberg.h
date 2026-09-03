/*! @file
    @brief Ptberg sde generator class
*/
//  2007, AlgoQuantHub.
#ifndef LACalibrateModelPtberg_h
#define LACalibrateModelPtberg_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrateModelPtberg.h
//
//  DESCRIPTION :       Ptberg SDE generator 
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "LAString.h"
#include "LACalibrateModelFX.h"


class LADataInstance;
class LAMathVolatility;
class LACalibrationParametersPtberg;

//===================== Class Declare LACalibrateModelPtberg==================================
/*! 
    @brief Ptberg SDE generator 
	

*/
class LACalibrateModelPtberg : public LACalibrateModelFX
{

public:
	// constructor
	explicit LACalibrateModelPtberg();
	// destructor
	virtual ~LACalibrateModelPtberg(void);
	// copy constructor
	LACalibrateModelPtberg(const LACalibrateModelPtberg &rhs);
	LACalibrateModelPtberg &operator=(const LACalibrateModelPtberg &rhs);
protected:
	friend class MAPtbergUtils;
	//==============================================
	// create sde instance 
	virtual LARatesSDEBase *createSDEInstance(const LAString &fx, LADataInstance &dataInstance) const;
	//==============================================
	// get sde Type
	virtual  SDE_TYPE getSDEType(const LAString &fx) const;
	//==============================================
	// check LJ
	virtual bool isLJ(const LAString &fx) const;
	//==============================================
	// set volatility 
	virtual  void setVolatility(const LAString &fx, LARatesSDEBase &sde)  const;	
	//==============================================
	// set drift 
	virtual  void setDrift(const LAString &fx, LARatesSDEBase &sde)  const;
	//==============================================
	// set integralfunction 
	virtual  void setIntegralFunction(const LAString &fx, LARatesSDEBase &sde)  const;
	//==============================================
	// get function master regist name 
	virtual LAString getFunctionMasterResistName(const LAString &fx) const;

	//==============================================
	// get vol type
	virtual LAString getVolType(const LAString &fx) const;
	//==============================================
	// setup vol data
	virtual void setUpVolData(const LAString &fx, LAMathVolatility &vol, LADataInstance &dataInstance) const;
	//==============================================
	// setup vol method
	virtual void setUpVolFunc(const LAString &fx, LAMathVolatility &vol, LADataInstance &dataInstance) const;
	//==============================================
	// create calibration info
	virtual LACalibrationParametersPtberg *createCalibInfoCreator(void) const;

private:
	//==============================================
	// set volatility object
	void setUpVolEntity(const LAString &fx, LAMathVolatility &vol) const;

};
#endif
