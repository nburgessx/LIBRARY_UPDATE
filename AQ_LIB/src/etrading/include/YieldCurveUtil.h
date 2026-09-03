#pragma once

#include "AQLString.h"
#include "AQLCoreTemplateType.h"
#include "ReadDataFile.h"

namespace etrading
{
    // extract curve id from ReadTestData input object; example: "USDYC"
    AQLString getCurveID( const etrading::ReadDataFile::Load& );

    // extract market name from ReadTestData input object; example: "OIS"
    AQLString getMarketName( const etrading::ReadDataFile::Load& );

    // extract curve names from ReadTestData input object; example: ["STD", "EUR6ML"]
    AQLStringVector getCurveNames( const etrading::ReadDataFile::Load& );
}
