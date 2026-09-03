#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreTemplateType.h"
#include "AQLCalibrate.h"

class AQLObject;

class AQLCalibrateIRSABR : public AQLCalibrate
{
public :
//  LIFECYCLE
	// constructor
	explicit AQLCalibrateIRSABR(void);
	// destructor
	virtual ~AQLCalibrateIRSABR(void);

	//=============================================
	//  setup
    virtual void	setUp(AQLObjectPool &objPool,  const AQLScenarioParam &param, AQLCalibrationFunc *method, int gridPos = -1);
	//=============================================
	//  calibration method
    virtual void    doCalibrate();

	AQLStringVector mUnderlyings;
	std::map<AQLString, AQLString> mcurvesetid;
	std::map<AQLString, AQLString> malphaid;
	std::map<AQLString, AQLString> mbetaid;
	std::map<AQLString, AQLString> mrhoid;
	std::map<AQLString, AQLString> mnuid;
	std::map<AQLString, AQLString> matmvolid;
	std::map<AQLString, AQLString> mswapconvid;
	std::map<AQLString, AQLString> mcapconvid;
	std::map<AQLString, AQLStringVector> mswapvolid;
	std::map<AQLString, AQLStringMatrix> msabrLimiter;
	std::map<AQLString, AQLString> mtarget;
	std::map<AQLString, DoubleVector> mweight;
	std::map<AQLString, AQLStringMatrix> mcurvesetmat;
	std::map<AQLString, bool> mIsCalibSkip;
	std::map<AQLString, AQLString> mcalibmethod;
	std::map<AQLString, AQLString> mapproxmethod;
	std::map<AQLString, BoolVector> mcalibflag;
	std::map<AQLString, IntVector> moptionsign;
	std::map<AQLString, BoolMatrix> mcalibflgmat;
	std::map<AQLString, bool> mishaganatmvolfixed;
	std::map<AQLString, double> mforwardshiftvalue;
};

