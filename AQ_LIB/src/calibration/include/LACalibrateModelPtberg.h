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

#include "AQLString.h"
#include "LACalibrateModelFX.h"


class AQLDataInstance;
class AQLMathVolatility;
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
	virtual AQLRatesSDEBase *createSDEInstance(const AQLString &fx, AQLDataInstance &dataInstance) const;
	//==============================================
	// get sde Type
	virtual  SDE_TYPE getSDEType(const AQLString &fx) const;
	//==============================================
	// check LJ
	virtual bool isLJ(const AQLString &fx) const;
	//==============================================
	// set volatility 
	virtual  void setVolatility(const AQLString &fx, AQLRatesSDEBase &sde)  const;	
	//==============================================
	// set drift 
	virtual  void setDrift(const AQLString &fx, AQLRatesSDEBase &sde)  const;
	//==============================================
	// set integralfunction 
	virtual  void setIntegralFunction(const AQLString &fx, AQLRatesSDEBase &sde)  const;
	//==============================================
	// get function master regist name 
	virtual AQLString getFunctionMasterResistName(const AQLString &fx) const;

	//==============================================
	// get vol type
	virtual AQLString getVolType(const AQLString &fx) const;
	//==============================================
	// setup vol data
	virtual void setUpVolData(const AQLString &fx, AQLMathVolatility &vol, AQLDataInstance &dataInstance) const;
	//==============================================
	// setup vol method
	virtual void setUpVolFunc(const AQLString &fx, AQLMathVolatility &vol, AQLDataInstance &dataInstance) const;
	//==============================================
	// create calibration info
	virtual LACalibrationParametersPtberg *createCalibInfoCreator(void) const;

private:
	//==============================================
	// set volatility object
	void setUpVolEntity(const AQLString &fx, AQLMathVolatility &vol) const;

};
#endif
