#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LACoreTemplateType.h"
#include "LACalibrate.h"

class LAObject;

class LACalibrateIRSABR : public LACalibrate
{
public :
//  LIFECYCLE
	// constructor
	explicit LACalibrateIRSABR(void);
	// destructor
	virtual ~LACalibrateIRSABR(void);

	//=============================================
	//  setup
    virtual void	setUp(LAObjectPool &objPool,  const MAScenarioParam &param, MACalibrationFunc *method, int gridPos = -1);
	//=============================================
	//  calibration method
    virtual void    doCalibrate();

	LAStringVector mUnderlyings;
	std::map<LAString, LAString> mcurvesetid;
	std::map<LAString, LAString> malphaid;
	std::map<LAString, LAString> mbetaid;
	std::map<LAString, LAString> mrhoid;
	std::map<LAString, LAString> mnuid;
	std::map<LAString, LAString> matmvolid;
	std::map<LAString, LAString> mswapconvid;
	std::map<LAString, LAString> mcapconvid;
	std::map<LAString, LAStringVector> mswapvolid;
	std::map<LAString, LAStringMatrix> msabrLimiter;
	std::map<LAString, LAString> mtarget;
	std::map<LAString, DoubleVector> mweight;
	std::map<LAString, LAStringMatrix> mcurvesetmat;
	std::map<LAString, bool> mIsCalibSkip;
	std::map<LAString, LAString> mcalibmethod;
	std::map<LAString, LAString> mapproxmethod;
	std::map<LAString, BoolVector> mcalibflag;
	std::map<LAString, IntVector> moptionsign;
	std::map<LAString, BoolMatrix> mcalibflgmat;
	std::map<LAString, bool> mishaganatmvolfixed;
	std::map<LAString, double> mforwardshiftvalue;
};

