#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include <vector>
#include "LACoreTemplateType.h"
#include "LACalibrate.h"

class LAObject;
class LAFunctionBase;

class LACalibrateLMM : public LACalibrate
{
public :
//  LIFECYCLE
	// constructor
	explicit LACalibrateLMM(void);
	// destructor
	virtual ~LACalibrateLMM(void);

	//=============================================
	//  setup
    virtual void	setUp(LAObjectPool &objPool,  const MAScenarioParam &param, MACalibrationFunc *method, int gridPos = -1);
	//=============================================
	//  calibration method
    virtual void    doCalibrate();

};

