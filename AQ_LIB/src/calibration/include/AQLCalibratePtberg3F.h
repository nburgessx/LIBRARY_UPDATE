#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreTemplateType.h"
#include "AQLCalibratePtberg.h"

class AQLObject;
class AQLMathYieldCurve;

class AQLCalibratePtberg3F : public AQLCalibratePtberg
{
public :
//  LIFECYCLE
	// constructor
	explicit AQLCalibratePtberg3F(void);
	// destructor
	virtual ~AQLCalibratePtberg3F(void);

	//=============================================
	//  setup
    virtual void	setUp(AQLObjectPool &objPool,  const AQLScenarioParam &param, AQLCalibrationFunc *method, int gridPos = -1);
	//=============================================
	//  calibration method
    virtual void    doCalibrate();
};

