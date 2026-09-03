#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreTemplateType.h"
#include "AQLCalibrate.h"

class AQLObject;

class AQLCalibrateHW : public AQLCalibrate
{
public :
//  LIFECYCLE
	// constructor
	explicit AQLCalibrateHW(void);
	// destructor
	virtual ~AQLCalibrateHW(void);

	//=============================================
	//  setup
    virtual void	setUp(AQLObjectPool &objPool,  const AQLScenarioParam &param, AQLCalibrationFunc *method, int gridPos = -1);
	//=============================================
	//  calibration method
    virtual void    doCalibrate();
	

};

