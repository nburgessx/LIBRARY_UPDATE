#pragma once

#include "LAString.h"
#include "LACoreTemplateType.h"
#include "ReadDataFile.h"

namespace etrading
{
    // extract curve id from ReadTestData input object; example: "USDYC"
    LAString getCurveID( const etrading::ReadDataFile::Load& );

    // extract market name from ReadTestData input object; example: "OIS"
    LAString getMarketName( const etrading::ReadDataFile::Load& );

    // extract curve names from ReadTestData input object; example: ["STD", "EUR6ML"]
    LAStringVector getCurveNames( const etrading::ReadDataFile::Load& );
}
