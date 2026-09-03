#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreTemplateType.h"
#include "LACalibrate.h"

class AQLObject;

class LACalibrateHW : public LACalibrate
{
public :
//  LIFECYCLE
	// constructor
	explicit LACalibrateHW(void);
	// destructor
	virtual ~LACalibrateHW(void);

	//=============================================
	//  setup
    virtual void	setUp(AQLObjectPool &objPool,  const MAScenarioParam &param, MACalibrationFunc *method, int gridPos = -1);
	//=============================================
	//  calibration method
    virtual void    doCalibrate();
	

};

