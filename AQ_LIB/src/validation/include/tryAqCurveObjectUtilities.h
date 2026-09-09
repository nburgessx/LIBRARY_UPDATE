#pragma once

#include <vector>
#include <string>
#include <utility>
#include <tuple>
#include <boost/date_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>


#include "AQLDataInstance.h"
#include "Variant.h"
#include "Environment.h"
#include "FileUtilities.h"
#include "CoreEnumerations.h"
#include "UserUtilities.h"
#include "tryAqToolGrid.h"
#include "AQLCoreTemplateType.h"
#include "CoreEnumerations.h"
#include "Environment.h"
#include "SerializationUtilities.h"


namespace validation
{
    const std::vector<std::string> tryAqCurveObjectList();

    const bool tryAqCurveObjectDelete( const std::string& curveName );

    const int tryAqCurveObjectDeleteAll();

    std::string tryAqCurveObjectSave( const std::string& aqObjCurveName,
                                   const std::string& fileNameToWriteTo,
                                   const etrading::FileTypeEnum fileType = etrading::JSON );

    std::pair<const bool, std::string> tryAqCurveObjectLoad( const std::string& fileName,
                                                          const etrading::FileTypeEnum fileType = etrading::JSON );


}
