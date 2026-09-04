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
#include "tryAqObjToolsGrid.h"
#include "AQLCoreTemplateType.h"
#include "CoreEnumerations.h"
#include "Environment.h"
#include "SerializationUtilities.h"


namespace validation
{
    const std::vector<std::string> tryAqObjCurvesList();

    const bool tryAqObjCurvesDelete( const std::string& curveName );

    const int tryAqObjCurvesDeleteAll();

    std::string tryAqObjCurvesSave( const std::string& aqObjCurveName,
                                   const std::string& fileNameToWriteTo,
                                   const etrading::FileTypeEnum fileType = etrading::JSON );

    std::pair<const bool, std::string> tryAqObjCurvesLoad( const std::string& fileName,
                                                          const etrading::FileTypeEnum fileType = etrading::JSON );


}
