/*
 * @brief			validation interface for the meLWOStoreTable method
 * @Created:		11 April 2016
 * @Author:			Hans Roggeman
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

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

namespace validation_api
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