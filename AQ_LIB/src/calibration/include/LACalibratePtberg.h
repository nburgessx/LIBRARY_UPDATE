#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LACoreTemplateType.h"
#include "LACalibrate.h"

class LAObject;
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
    virtual void	setUp(LAObjectPool &objPool,  const MAScenarioParam &param, MACalibrationFunc *method, int gridPos = -1);
	//=============================================
	//  calibration method
    virtual void    doCalibrate();
protected :
	double mSpotRate;
	bool mIsLJ;
	bool mIsDDL;
	DoubleArray mDFRatios;
	DoubleArray mTimeGrid;
	LAString mFXCurrency;
};

