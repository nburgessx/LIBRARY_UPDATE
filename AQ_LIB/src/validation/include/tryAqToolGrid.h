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
#include "FreeObject.h"


namespace validation
{
    typedef std::vector<std::vector<etrading::Variant>> FlexibleData;
    typedef std::tuple<std::vector<std::string>, std::vector<etrading::ContainedTypeEnum>, FlexibleData>  TableInfo;

    /* @brief			validation interface for the tryAqObjectCreateRange method
    *  @param [in]		objectName			object name that will be stored as a FreeObject
    *  @param [in]		tableInfo		A full description with data of the data in the range
    *  @param [in]		allowJaggedData		A  boolean indicating whether all columns have the same amount of data
    *							(coming from excel they will (because ranges are rectangular) but from C++ this is not necessarily the case)
    */
    std::string tryAqGridObjectCreate(
        const std::string& objectName,
        const TableInfo& tableInfo,
        const bool allowJaggedData = false );

    std::string tryAqGridObjectSave(
        const std::string& objectName,
        const std::string& fileNameToWriteTo
    );

    /* @brief			validation interface for the tryAqObjectCreateRange method
    *  @param [in]		objectName			object name that will be stored as a FreeObject
    *  @param [in]		showColNames		A boolean indicating whether the column names need to be displayed
    */
    std::pair<const FlexibleData, std::vector<std::string>>  tryAqGridObjectDisplay(
                const std::string& objectName );

    std::pair<const bool, std::string> tryAqGridObjectLoad(
        const std::string& fileName );

    std::vector<std::string> tryAqGridObjectNames();
    const bool tryAqGridObjectClearOne( const std::string& objectName );
    const bool tryAqGridObjectClearAll();

};

