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
#include "tryAqObjectsGrid.h"
#include "AQLCoreTemplateType.h"
#include "CoreEnumerations.h"
#include "Environment.h"
#include "SerializationUtilities.h"


namespace validation
{
    const std::vector<std::string> tryAqObjectsCurveList();

    const bool tryAqObjectsCurveDelete( const std::string& curveName );

    const int tryAqObjectsCurveDeleteAll();

    std::string tryAqObjectsCurveSave( const std::string& aqoCurveName,
                                   const std::string& fileNameToWriteTo,
                                   const etrading::FileTypeEnum fileType = etrading::JSON );

    std::pair<const bool, std::string> tryAqObjectsCurveLoad( const std::string& fileName,
                                                          const etrading::FileTypeEnum fileType = etrading::JSON );


}
