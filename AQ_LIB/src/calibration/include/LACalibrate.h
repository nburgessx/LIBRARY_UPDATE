#pragma once

#ifndef IR_CALIBRATION_DATA_OPTIONMATURITY
#define IR_CALIBRATION_DATA_OPTIONMATURITY				"OptionMaturity"		//  data name of OptionMaturity
#endif

#include "LAString.h"
#include <map>

class LAObject;
class LAObjectPool;
class MACalibrationFunc;
class LADataInstance;
struct MAScenarioParam;


class LACalibrate
{
public :
//  LIFECYCLE
	// constructor
	explicit LACalibrate(void);
	// destructor
	virtual ~LACalibrate(void);

	//=============================================
	//  setup
    virtual void	setUp(LAObjectPool &objPool,  const MAScenarioParam &param, MACalibrationFunc *method, int gridPos = -1) = 0;
	//=============================================
	//  calibration method
    virtual void    doCalibrate() = 0;
	//=============================================
	//  get calib engine
	virtual LAObject *getCalibEngine() { return mpCaibEngine; }
	//=============================================
	//  get serialized result
	static LAString getSerializedData(LAString file) { return mSerializeMap[file]; }
	//=============================================
	//  clear method
	static void clear(const LAString &fileNum);
	//=============================================
	//  clear method
	static void clear();
	//=============================================

protected :

	//=============================================
	//  deserialize method
	void deserializeStream(const LAString &key);
	LAString mCalcType;      // calctype
	int mGridPos;            // calibration grid pos
	MACalibrationFunc *mpFunc;  // calibration method
	LAObject *mpCaibEngine;     // calibration engine
	LAString mSerializeStatus;  // serialize status
	LAString mSerializeFile;    // serialize file
	LAString mCalibIDName;      // calibration ID name
	LADataInstance *mpDataInstance;             // dataInstance pointer
	static std::map<LAString, LAString> mSerializeMap;  // serialize map
	static std::map<LAString, std::map<LAString, LAString> > mDeserializedEMap; // deserialized object map
	static std::map<LAString, bool> mIsDeserializedMap; // deserialized flag
	void setmSerializeMap(const LAString &fileNum);
	void setmIsDeserializedMap(bool isDeserializedMap);
private:
#ifdef __HAS_MIC__
	static common_lib::StaticMutex  mMutex;
#endif
};


