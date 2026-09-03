#pragma once

#ifndef IR_CALIBRATION_DATA_OPTIONMATURITY
#define IR_CALIBRATION_DATA_OPTIONMATURITY				"OptionMaturity"		//  data name of OptionMaturity
#endif

#include "AQLString.h"
#include <map>

class AQLObject;
class AQLObjectPool;
class AQLCalibrationFunc;
class AQLDataInstance;
struct AQLScenarioParam;


class AQLCalibrate
{
public :
//  LIFECYCLE
	// constructor
	explicit AQLCalibrate(void);
	// destructor
	virtual ~AQLCalibrate(void);

	//=============================================
	//  setup
    virtual void	setUp(AQLObjectPool &objPool,  const AQLScenarioParam &param, AQLCalibrationFunc *method, int gridPos = -1) = 0;
	//=============================================
	//  calibration method
    virtual void    doCalibrate() = 0;
	//=============================================
	//  get calib engine
	virtual AQLObject *getCalibEngine() { return mpCaibEngine; }
	//=============================================
	//  get serialized result
	static AQLString getSerializedData(AQLString file) { return mSerializeMap[file]; }
	//=============================================
	//  clear method
	static void clear(const AQLString &fileNum);
	//=============================================
	//  clear method
	static void clear();
	//=============================================

protected :

	//=============================================
	//  deserialize method
	void deserializeStream(const AQLString &key);
	AQLString mCalcType;      // calctype
	int mGridPos;            // calibration grid pos
	AQLCalibrationFunc *mpFunc;  // calibration method
	AQLObject *mpCaibEngine;     // calibration engine
	AQLString mSerializeStatus;  // serialize status
	AQLString mSerializeFile;    // serialize file
	AQLString mCalibIDName;      // calibration ID name
	AQLDataInstance *mpDataInstance;             // dataInstance pointer
	static std::map<AQLString, AQLString> mSerializeMap;  // serialize map
	static std::map<AQLString, std::map<AQLString, AQLString> > mDeserializedEMap; // deserialized object map
	static std::map<AQLString, bool> mIsDeserializedMap; // deserialized flag
	void setmSerializeMap(const AQLString &fileNum);
	void setmIsDeserializedMap(bool isDeserializedMap);
private:
#ifdef __HAS_MIC__
	static common_lib::StaticMutex  mMutex;
#endif
};


