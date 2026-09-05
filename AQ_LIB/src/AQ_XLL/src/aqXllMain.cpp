#include <aqXllMain.h>

// IMPORTANT: Only include this in one source file per addin, otherwise you will get multiple function definitions.
#include <xloil/XllEntryPoint.h> 

// Addin Manager Info
struct AlgoQuantLib
{
    static std::wstring addInManagerInfo() { return L"Algo Quant Lib"; }
};
XLO_DECLARE_ADDIN(AlgoQuantLib);
