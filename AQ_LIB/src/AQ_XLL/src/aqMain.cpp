#include <aqMain.h>

// IMPORTANT: Only include this in one source file per addin, otherwise you will get multiple function definitions.
#include <xloil/XllEntryPoint.h>

#include "InitializeETrading.h"   // etrading::InitializeETrading

// Addin Manager Info
struct AlgoQuantLib
{
    // xlOil constructs this struct once, from RegisterAddin::autoOpen() during
    // xlAutoOpen. Use it to bring the library up: InitializeETrading::instance()
    // loads the holiday calendars, the IR static data and the optional startup
    // config. The date and curve functions assume this has run - it is the same
    // setup GTEST performs in InitializeGoogleTest, and the equivalent of the
    // legacy add-in's OnXllOpenEx / MLIB_START_AND_CHECK_LICENCE path.
    //
    // Called with no arguments so a config-load failure does not abort the
    // add-in load; aqToolsInitialize() re-runs it with the loud checks and
    // reports which config path was used.
    AlgoQuantLib()
    {
        etrading::InitializeETrading::instance();
    }

    ~AlgoQuantLib()
    {
        etrading::InitializeETrading::destroyInstance();
    }

    static std::wstring addInManagerInfo() { return L"Algo Quant Lib"; }
};
XLO_DECLARE_ADDIN(AlgoQuantLib);
