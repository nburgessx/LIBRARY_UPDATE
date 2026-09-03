#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreTemplateType.h"
#include "LACalibrate.h"

class AQLObject;
class LAMathYieldCurve;

class LACalibrateSZ : public LACalibrate
{
public:
	//  LIFECYCLE
	// constructor
	explicit LACalibrateSZ(void);
	// destructor
	virtual ~LACalibrateSZ(void);

	//=============================================
	//  setup
	virtual void	setUp(AQLObjectPool &objPool, const MAScenarioParam &param, MACalibrationFunc *method, int gridPos = -1);
	virtual void	setUp2(AQLObjectPool &objPool, const MAScenarioParam &param, MACalibrationFunc *method, int gridPos = -1);
	//=============================================
	//  calibration method
	virtual void    doCalibrate();
protected:
	double mSpotRate;
	bool mIsLJ;
	bool mIsDDL;
	DoubleArray mDFRatios;
	DoubleArray mTimeGrid;
	AQLString mFXCurrency;
};

