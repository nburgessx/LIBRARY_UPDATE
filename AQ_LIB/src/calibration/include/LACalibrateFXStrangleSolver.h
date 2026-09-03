#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreTemplateType.h"
#include "LACalibrate.h"
#include "AQLMathFXVolatilitySurfaceGenerate.h"

class AQLObject;
class AQLMathYieldCurve;

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
    virtual void	setUp(AQLObjectPool &objPool,  const MAScenarioParam &param, MACalibrationFunc *method, int gridPos = -1);
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

