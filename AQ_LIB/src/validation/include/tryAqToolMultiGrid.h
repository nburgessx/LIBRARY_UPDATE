#pragma once

#include <string>
#include <utility>
#include <tuple>
#include <vector>

#include "AQLCoreTemplateType.h"
#include "AQLDataInstance.h"
#include "Variant.h"
#include "CoreEnumerations.h"
#include "UserUtilities.h"
#include "tryAqToolGrid.h"

namespace validation
{
    std::string tryAqToolObjectMultiGridCreate(	const std::string& objectName,
                                            const std::vector<std::string>& gridNames,
                                            const std::vector<TableInfo>& infoBlocks,
                                            const bool allowJaggedData = false );

    std::pair<const FlexibleData, std::vector<std::string>>  tryAqToolObjectMultiGridDisplay(
                const std::string& objectName,
                const std::string& gridName );

    std::vector<std::string> tryAqToolObjectMultiGridSubNames( const std::string&  objectName );

}