/*
 * @brief			validation interface for meCurve - Access and retrieve methods
 * @Created:		19 April 2016
 * @Author:			Hans Roggeman
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include <vector>
#include <string>
#include <utility>
#include <tuple>
#include <boost/date_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>


#include "LADataInstance.h"
#include "Variant.h"
#include "Environment.h"
#include "FileUtilities.h"
#include "CoreEnumerations.h"
#include "UserUtilities.h"
#include "tryMeLWOGrid.h"
#include "LACoreTemplateType.h"
#include "CoreEnumerations.h"
#include "Environment.h"
#include "SerializationUtilities.h"


namespace validation_api
{
    const std::vector<std::string> tryMeLWOCurveList();

    const bool tryMeLWOCurveDelete( const std::string& curveName );

    const int tryMeLWOCurveDeleteAll();

    std::string tryMeLWOCurveSave( const std::string& lwoCurveName,
                                   const std::string& fileNameToWriteTo,
                                   const etrading::FileTypeEnum fileType = etrading::JSON );

    std::pair<const bool, std::string> tryMeLWOCurveLoad( const std::string& fileName,
                                                          const etrading::FileTypeEnum fileType = etrading::JSON );


}
