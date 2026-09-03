#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include <vector>
#include "AQLCoreTemplateType.h"
#include "AQLCalibrate.h"

class AQLObject;
class AQLFunctionBase;

class AQLCalibrateLMM : public AQLCalibrate
{
public :
//  LIFECYCLE
	// constructor
	explicit AQLCalibrateLMM(void);
	// destructor
	virtual ~AQLCalibrateLMM(void);

	//=============================================
	//  setup
    virtual void	setUp(AQLObjectPool &objPool,  const AQLScenarioParam &param, AQLCalibrationFunc *method, int gridPos = -1);
	//=============================================
	//  calibration method
    virtual void    doCalibrate();

};

