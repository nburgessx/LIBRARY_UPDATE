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
#include "tryAqObjToolsGrid.h"

namespace validation
{
    std::string tryAqObjToolsMultiGridCreate(	const std::string& objectName,
                                            const std::vector<std::string>& gridNames,
                                            const std::vector<TableInfo>& infoBlocks,
                                            const bool allowJaggedData = false );

    std::pair<const FlexibleData, std::vector<std::string>>  tryAqObjToolsMultiGridDisplay(
                const std::string& objectName,
                const std::string& gridName );

    std::vector<std::string> tryAqObjToolsMultiGridSubNames( const std::string&  objectName );

}