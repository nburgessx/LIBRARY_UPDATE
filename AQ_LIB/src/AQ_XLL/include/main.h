#pragma once

#include <xloil/xlOil.h>
using namespace xloil;

namespace aq_xll
{
    // Converts an Excel wide string argument to the narrow strings used by AQ.
    // Safe for the ASCII keywords the API uses (Call, Put, Following, etc).
    inline std::string toNarrowString(const xloil::ExcelObj& obj)
    {
        const std::wstring ws = obj.toString();
        return std::string(ws.begin(), ws.end());
    }
}