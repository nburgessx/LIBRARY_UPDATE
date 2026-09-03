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

#include "AQLString.h"
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


class AQLDataInstance;
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
	virtual LARatesSDEBase *createSDEInstance(const AQLString &fx, AQLDataInstance &dataInstance) const;
	//==============================================
	// get sde Type
	virtual  SDE_TYPE getSDEType(const AQLString &fx) const;
	//==============================================
	// check LJ
	virtual bool isLJ(const AQLString &fx) const;
	//==============================================
	// set volatility 
	virtual  void setVolatility(const AQLString &fx, LARatesSDEBase &sde)  const;	
	//==============================================
	// set drift 
	virtual  void setDrift(const AQLString &fx, LARatesSDEBase &sde)  const;
	//==============================================
	// set integralfunction 
	virtual  void setIntegralFunction(const AQLString &fx, LARatesSDEBase &sde)  const;
	//==============================================
	virtual  void setOutputTemplate(const AQLString &fx, LARatesSDEBase &sde)  const;
	//==============================================
	// set interpolation method 
	virtual  void setInterpolationMethod(const AQLString &fx, LARatesSDEBase &sde) const;
	//==============================================
	// get function master regist name 
	virtual AQLString getFunctionMasterResistName(const AQLString &fx) const;

	//==============================================
	// get vol type
	virtual AQLString getVolType(const AQLString &fx) const;
	//==============================================
	// setup vol data
	virtual void setUpVolData(const AQLString &fx, LAMathVolatility &vol, AQLDataInstance &dataInstance) const;
	//==============================================
	// setup vol method
	virtual void setUpVolFunc(const AQLString &fx, LAMathVolatility &vol, AQLDataInstance &dataInstance) const;

private:
	//==============================================
	// set volatility object
	void setUpVolEntity(const AQLString &fx, LAMathVolatility &vol) const;

};
#endif
