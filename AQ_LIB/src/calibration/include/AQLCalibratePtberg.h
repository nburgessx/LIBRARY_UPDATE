#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreTemplateType.h"
#include "AQLCalibrate.h"

class AQLObject;
class AQLMathYieldCurve;

class AQLCalibratePtberg : public AQLCalibrate
{
public :
//  LIFECYCLE
	// constructor
	explicit AQLCalibratePtberg(void);
	// destructor
	virtual ~AQLCalibratePtberg(void);

	//=============================================
	//  setup
    virtual void	setUp(AQLObjectPool &objPool,  const AQLScenarioParam &param, AQLCalibrationFunc *method, int gridPos = -1);
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

