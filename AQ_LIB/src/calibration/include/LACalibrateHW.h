#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LACoreTemplateType.h"
#include "LACalibrate.h"

class LAObject;

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
    virtual void	setUp(LAObjectPool &objPool,  const MAScenarioParam &param, MACalibrationFunc *method, int gridPos = -1);
	//=============================================
	//  calibration method
    virtual void    doCalibrate();
	

};

