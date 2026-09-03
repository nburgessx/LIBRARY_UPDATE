#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LACoreTemplateType.h"
#include "LACalibrate.h"
#include "LAMathFXVolatilitySurfaceGenerate.h"

class LAObject;
class LAMathYieldCurve;

class LACalibrateFXStrangleSolver : public LACalibrate
{
public :
//  LIFECYCLE
	// constructor
	explicit LACalibrateFXStrangleSolver(void);
	// destructor
	virtual ~LACalibrateFXStrangleSolver(void);

	//=============================================
	//  setup
    virtual void	setUp(LAObjectPool &objPool,  const MAScenarioParam &param, MACalibrationFunc *method, int gridPos = -1);
	//=============================================
	//  calibration method
    virtual void    doCalibrate();
private :
	double mSpotRate;
	InterpolationMethod mMethod;
	InterpolationTarget mTarget;
	InterpolationVariable mVariable;
	ATMInterpolationMethod mAtmMethod;
	DoubleVector mWingFactor;

	
	mutable	std::vector<SmileParam> mSmileParams;
	mutable	std::vector<SmileData> mSmileData;
	mutable std::vector<FXOptionData > mFxParams;
	bool mIsApproximation;
	bool mIsWing;


};

