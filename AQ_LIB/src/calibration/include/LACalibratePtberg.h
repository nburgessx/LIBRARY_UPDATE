#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreTemplateType.h"
#include "LACalibrate.h"

class AQLObject;
class LAMathYieldCurve;

class LACalibratePtberg : public LACalibrate
{
public :
//  LIFECYCLE
	// constructor
	explicit LACalibratePtberg(void);
	// destructor
	virtual ~LACalibratePtberg(void);

	//=============================================
	//  setup
    virtual void	setUp(AQLObjectPool &objPool,  const MAScenarioParam &param, MACalibrationFunc *method, int gridPos = -1);
	//=============================================
	//  calibration method
    virtual void    doCalibrate();
protected :
	double mSpotRate;
	bool mIsLJ;
	bool mIsDDL;
	DoubleArray mDFRatios;
	DoubleArray mTimeGrid;
	AQLString mFXCurrency;
};

