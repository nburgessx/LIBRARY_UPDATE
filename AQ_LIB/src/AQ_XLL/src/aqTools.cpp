#include <aqMain.h>

// Test Function
XLO_FUNC_START(aqToolsEcho(const ExcelObj* arg))
{
    return returnValue(arg->toString());
}
XLO_FUNC_END(aqToolsEcho).threadsafe()
.help(L"Returns the argument provided")
.arg(L"Value", L"Any value");


// Build Version Time Stamp Method
XLO_FUNC_START(aqToolsBuildTime())
{
    return returnValue(L"AQ_ADDIN built " L"" __DATE__ L" " __TIME__);
}
XLO_FUNC_END(aqToolsBuildTime)
.help(L"Build date and time of the loaded add-in.");