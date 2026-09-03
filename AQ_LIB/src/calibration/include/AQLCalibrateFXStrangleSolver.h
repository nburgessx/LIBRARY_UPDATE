#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreTemplateType.h"
#include "AQLCalibrate.h"
#include "AQLMathFXVolatilitySurfaceGenerate.h"

class AQLObject;
class AQLMathYieldCurve;

class AQLCalibrateFXStrangleSolver : public AQLCalibrate
{
public :
//  LIFECYCLE
	// constructor
	explicit AQLCalibrateFXStrangleSolver(void);
	// destructor
	virtual ~AQLCalibrateFXStrangleSolver(void);

	//=============================================
	//  setup
    virtual void	setUp(AQLObjectPool &objPool,  const AQLScenarioParam &param, AQLCalibrationFunc *method, int gridPos = -1);
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

