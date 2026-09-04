/*! @file
    @brief Ptberg sde generator class
*/
#ifndef AQLCalibrateModelPtberg_h
#define AQLCalibrateModelPtberg_h
#ifdef __GNUG__
#pragma interface
#endif

#include "AQLString.h"
#include "AQLCalibrateModelFX.h"


class AQLDataInstance;
class AQLMathVolatility;
class AQLCalibrationParametersPtberg;

//===================== Class Declare AQLCalibrateModelPtberg==================================
/*! 
    @brief Ptberg SDE generator 
	

*/
class AQLCalibrateModelPtberg : public AQLCalibrateModelFX
{

public:
	// constructor
	explicit AQLCalibrateModelPtberg();
	// destructor
	virtual ~AQLCalibrateModelPtberg(void);
	// copy constructor
	AQLCalibrateModelPtberg(const AQLCalibrateModelPtberg &rhs);
	AQLCalibrateModelPtberg &operator=(const AQLCalibrateModelPtberg &rhs);
protected:
	friend class AQLPtbergUtils;
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
	virtual AQLCalibrationParametersPtberg *createCalibInfoCreator(void) const;

private:
	//==============================================
	// set volatility object
	void setUpVolEntity(const AQLString &fx, AQLMathVolatility &vol) const;

};
#endif
