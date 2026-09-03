#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include <AQLDataInstance.h>
#include <AQLString.h>
#include <AQLDate.h>
#include <AQLCoreTemplateType.h>
#include <ConstantDeclarations.h>
#include "AQLMathFXVolatilitySurfaceGenerate.h"
#include "AQLMathDisplacedHestonTDP.h"
//////////////////////////

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

class AQLMathFXVolSurfUtility
{
public:
    static double FindStrikeFromDelta( AQLDataInstance* dataInstance, double delta, double vol, int sgn, 
        AQLStringMatrix volSurfaceInfo, const AQLDate& expiryDate, const AQLString& deltaType );

    static void SetDeltaType( const AQLString& deltaType_str, FXDeltaType& deltaType );

    static AQLString SetUpFXVolatilitySurface( AQLDataInstance* dataInstance, AQLStringMatrix& volSurfaceInfo, 
        AQLStringMatrix& fxOptionData, AQLStringMatrix& smParams_str, AQLStringMatrix& wingFactor );

    static void OutPutVolatilitySurface( AQLDataInstance* dataInstance, const AQLString& volSurfaceID, DoubleVector& ret, 
                                         size_t& maturitySize, size_t& strikeSize );

    static void OutPutFXOptionData( AQLDataInstance* dataInstance, const AQLString& volSurfaceID, DoubleVector& ret,
                                    size_t& dataNum );

    static double GetVolatility( AQLDataInstance* dataInstance, const AQLString& volSurfaceID, double point, const AQLDate& expiryPoint, 
        const AQLString& variable_str, const AQLString& maturityMethod_str );

	static double GetVolatility2( AQLDataInstance* dataInstance, const AQLString& volSurfaceID, double point, double expiryPoint, 
        double forwardFX, const AQLString& variable_str, const AQLString& maturityMethod_str );

    static double GetATMVolatility( AQLDataInstance* dataInstance, const AQLString& volSurfaceID, const AQLDate& expiryPoint,
        const AQLString& maturityMethod_str );

    static double GetFXPremiumFromSurface( AQLDataInstance* dataInstance, const AQLString& volSurfaceID, double point, const AQLDate& expiryPoint,
        const AQLString& variable_str, const AQLString& maturityMethod_str, const AQLString& callput, const AQLString& deltaType_str );

    static void GetWingFactorFromHeston( AQLDataInstance* dataInstance, AQLStringMatrix& volSurfaceInfo, AQLStringMatrix& fxOptionData, 
                                         AQLStringMatrix& smParams_str, const HestonParams_TDP& hestonParam, DoubleVector& wfs );

    static void GetMarketDataFromSmileData( AQLDataInstance* dataInstance, const AQLString& volSurfaceID, DoubleVector& mkDatas, size_t& dataNum );

private:
	AQLMathFXVolSurfUtility(void);
	~AQLMathFXVolSurfUtility(void);
	AQLMathFXVolSurfUtility(const AQLMathFXVolSurfUtility &rhs);
	AQLMathFXVolSurfUtility &operator=(const AQLMathFXVolSurfUtility &rhs);
};

