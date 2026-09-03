/*! @file
    @brief FXVanilla sde generator class
*/
//  2007, AlgoQuantHub.
#ifndef LACalibrateModelFXStrangleSolver_h
#define LACalibrateModelFXStrangleSolver_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrateModelFXStrangleSolver.h
//
//  DESCRIPTION :       FXVanilla SDE generator 
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

//// DEFINES ////
#ifndef IR_CALIBRATION_DATA_INTERPOLATIONMETHOD					
#define IR_CALIBRATION_DATA_INTERPOLATIONMETHOD	"INTERPOLATIONMETHOD"	// Data Name of
#endif
#ifndef IR_CALIBRATION_DATA_INTERPOLATIONTARGET					
#define IR_CALIBRATION_DATA_INTERPOLATIONTARGET	"INTERPOLATIONTARGET"	// Data Name of
#endif
#ifndef IR_CALIBRATION_DATA_INTERPOLATIONVARIABLE					
#define IR_CALIBRATION_DATA_INTERPOLATIONVARIABLE	"INTERPOLATIONVARIABLE"	// Data Name of
#endif
#ifndef IR_CALIBRATION_DATA_ATMINTERPOLATIONMETHOD					
#define IR_CALIBRATION_DATA_ATMINTERPOLATIONMETHOD	"ATMINTERPOLATIONMETHOD"				// Data Name of
#endif
#ifndef IR_CALIBRATION_DATA_ISWINGFLAG					
#define IR_CALIBRATION_DATA_ISWINGFLAG		"ISWINGFLAG"				// Data Name of
#endif
#ifndef IR_CALIBRATION_DATA_SMILEDATA_DELTAPUT					
#define IR_CALIBRATION_DATA_SMILEDATA_DELTAPUT	"SMILEDATA_DELTAPUT"	// Data Name of
#endif
#ifndef IR_CALIBRATION_DATA_SMILEDATA_LOGSTRIKE					
#define IR_CALIBRATION_DATA_SMILEDATA_LOGSTRIKE	"SMILEDATA_LOGSTRIKE"	// Data Name of
#endif
#ifndef IR_CALIBRATION_DATA_SMILEDATA_STRIKE					
#define IR_CALIBRATION_DATA_SMILEDATA_STRIKE		"SMILEDATA_STRIKE"		// Data Name of
#endif
#ifndef IR_CALIBRATION_DATA_SMILEDATA_VOL					
#define IR_CALIBRATION_DATA_SMILEDATA_VOL		"SMILEDATA_VOL"			// Data Name of
#endif
#ifndef IR_CALIBRATION_DATA_FXOPTDATA_T					
#define IR_CALIBRATION_DATA_FXOPTDATA_T		"FXOPTDATA_T"				// Data Name of
#endif
#ifndef IR_CALIBRATION_DATA_FXOPTDATA_PD					
#define IR_CALIBRATION_DATA_FXOPTDATA_PD		"FXOPTDATA_PD"				// Data Name of
#endif
#ifndef IR_CALIBRATION_DATA_FXOPTDATA_PF					
#define IR_CALIBRATION_DATA_FXOPTDATA_PF		"FXOPTDATA_PF"				// Data Name of
#endif
#ifndef IR_CALIBRATION_DATA_FXOPTDATA_F					
#define IR_CALIBRATION_DATA_FXOPTDATA_F		"FXOPTDATA_F"				// Data Name of
#endif
#ifndef IR_CALIBRATION_DATA_FXOPTDATA_SPOTFX					
#define IR_CALIBRATION_DATA_FXOPTDATA_SPOTFX	"FXOPTDATA_SPOTFX"			// Data Name of
#endif
#ifndef IR_CALIBRATION_DATA_FXOPTDATA_DAYS					
#define IR_CALIBRATION_DATA_FXOPTDATA_DAYS	"FXOPTDATA_DAYS"			// Data Name of
#endif
#ifndef IR_CALIBRATION_DATA_FXOPTDATA_DELTATYPE					
#define IR_CALIBRATION_DATA_FXOPTDATA_DELTATYPE	"FXOPTDATA_DELTATYPE"	// Data Name of
#endif
#ifndef IR_CALIBRATION_DATA_FXOPTDATA_ATMTYPE					
#define IR_CALIBRATION_DATA_FXOPTDATA_ATMTYPE		"FXOPTDATA_ATMTYPE"			// Data Name of
#endif


class LADataInstance;
class LAMathVolatility;

//===================== Class Declare LACalibrateModelFXStrangleSolver==================================
/*! 
    @brief FXVanilla SDE generator 
	

*/
class LACalibrateModelFXStrangleSolver : public LACalibrateModelFX
{

public:
	// constructor
	explicit LACalibrateModelFXStrangleSolver();
	// destructor
	virtual ~LACalibrateModelFXStrangleSolver(void);
	// copy constructor
	LACalibrateModelFXStrangleSolver(const LACalibrateModelFXStrangleSolver &rhs);
	LACalibrateModelFXStrangleSolver &operator=(const LACalibrateModelFXStrangleSolver &rhs);
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
	virtual  void setOutputTemplate(const LAString &fx, LARatesSDEBase &sde)  const;
	//==============================================
	// set interpolation method 
	virtual  void setInterpolationMethod(const LAString &fx, LARatesSDEBase &sde) const;
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

private:
	//==============================================
	// set volatility object
	void setUpVolEntity(const LAString &fx, LAMathVolatility &vol) const;

};
#endif
