#pragma once

#include <string>
#include <utility>
#include <tuple>
#include <vector>

#include "LACoreTemplateType.h"
#include "LADataInstance.h"
#include "Variant.h"
#include "CoreEnumerations.h"
#include "UserUtilities.h"
#include "tryMeLWOGrid.h"

namespace validation
{
    std::string tryMeLWOMultiGridCreate(	const std::string& objectName,
                                            const std::vector<std::string>& gridNames,
                                            const std::vector<TableInfo>& infoBlocks,
                                            const bool allowJaggedData = false );

    std::pair<const FlexibleData, std::vector<std::string>>  tryMeLWOMultiGridDisplay(
                const std::string& objectName,
                const std::string& gridName );

    std::vector<std::string> tryMeLWOMultiGridSubNames( const std::string&  objectName );

}