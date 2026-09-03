#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LACoreTemplateType.h"
#include "LACalibratePtberg.h"

class LAObject;
class LAMathYieldCurve;

class LACalibratePtberg3F : public LACalibratePtberg
{
public :
//  LIFECYCLE
	// constructor
	explicit LACalibratePtberg3F(void);
	// destructor
	virtual ~LACalibratePtberg3F(void);

	//=============================================
	//  setup
    virtual void	setUp(LAObjectPool &objPool,  const MAScenarioParam &param, MACalibrationFunc *method, int gridPos = -1);
	//=============================================
	//  calibration method
    virtual void    doCalibrate();
};

