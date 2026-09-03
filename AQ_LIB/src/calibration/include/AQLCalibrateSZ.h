#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreTemplateType.h"
#include "AQLCalibrate.h"

class AQLObject;
class AQLMathYieldCurve;

class AQLCalibrateSZ : public AQLCalibrate
{
public:
	//  LIFECYCLE
	// constructor
	explicit AQLCalibrateSZ(void);
	// destructor
	virtual ~AQLCalibrateSZ(void);

	//=============================================
	//  setup
	virtual void	setUp(AQLObjectPool &objPool, const AQLScenarioParam &param, AQLCalibrationFunc *method, int gridPos = -1);
	virtual void	setUp2(AQLObjectPool &objPool, const AQLScenarioParam &param, AQLCalibrationFunc *method, int gridPos = -1);
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

