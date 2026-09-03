#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include <LADataInstance.h>
#include <LAString.h>
#include <LADate.h>
#include <LACoreTemplateType.h>
#include <ConstantDeclarations.h>
#include "LAMathFXVolatilitySurfaceGenerate.h"
#include "LAMathDisplacedHestonTDP.h"
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

class LAMathFXVolSurfUtility
{
public:
    static double FindStrikeFromDelta( LADataInstance* dataInstance, double delta, double vol, int sgn, 
        LAStringMatrix volSurfaceInfo, const LADate& expiryDate, const LAString& deltaType );

    static void SetDeltaType( const LAString& deltaType_str, FXDeltaType& deltaType );

    static LAString SetUpFXVolatilitySurface( LADataInstance* dataInstance, LAStringMatrix& volSurfaceInfo, 
        LAStringMatrix& fxOptionData, LAStringMatrix& smParams_str, LAStringMatrix& wingFactor );

    static void OutPutVolatilitySurface( LADataInstance* dataInstance, const LAString& volSurfaceID, DoubleVector& ret, 
                                         size_t& maturitySize, size_t& strikeSize );

    static void OutPutFXOptionData( LADataInstance* dataInstance, const LAString& volSurfaceID, DoubleVector& ret,
                                    size_t& dataNum );

    static double GetVolatility( LADataInstance* dataInstance, const LAString& volSurfaceID, double point, const LADate& expiryPoint, 
        const LAString& variable_str, const LAString& maturityMethod_str );

	static double GetVolatility2( LADataInstance* dataInstance, const LAString& volSurfaceID, double point, double expiryPoint, 
        double forwardFX, const LAString& variable_str, const LAString& maturityMethod_str );

    static double GetATMVolatility( LADataInstance* dataInstance, const LAString& volSurfaceID, const LADate& expiryPoint,
        const LAString& maturityMethod_str );

    static double GetFXPremiumFromSurface( LADataInstance* dataInstance, const LAString& volSurfaceID, double point, const LADate& expiryPoint,
        const LAString& variable_str, const LAString& maturityMethod_str, const LAString& callput, const LAString& deltaType_str );

    static void GetWingFactorFromHeston( LADataInstance* dataInstance, LAStringMatrix& volSurfaceInfo, LAStringMatrix& fxOptionData, 
                                         LAStringMatrix& smParams_str, const HestonParams_TDP& hestonParam, DoubleVector& wfs );

    static void GetMarketDataFromSmileData( LADataInstance* dataInstance, const LAString& volSurfaceID, DoubleVector& mkDatas, size_t& dataNum );

private:
	LAMathFXVolSurfUtility(void);
	~LAMathFXVolSurfUtility(void);
	LAMathFXVolSurfUtility(const LAMathFXVolSurfUtility &rhs);
	LAMathFXVolSurfUtility &operator=(const LAMathFXVolSurfUtility &rhs);
};

