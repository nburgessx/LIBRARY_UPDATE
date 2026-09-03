#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include <vector>
#include "AQLCoreTemplateType.h"
#include "LACalibrate.h"

class AQLObject;
class AQLFunctionBase;

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
    virtual void	setUp(AQLObjectPool &objPool,  const MAScenarioParam &param, MACalibrationFunc *method, int gridPos = -1);
	//=============================================
	//  calibration method
    virtual void    doCalibrate();

};

