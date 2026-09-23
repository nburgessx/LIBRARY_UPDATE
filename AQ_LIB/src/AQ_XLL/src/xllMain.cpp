#include <xllMain.h>

// IMPORTANT: Only include this in one source file per addin, otherwise you will get multiple function definitions.
#include <xloil/XllEntryPoint.h>

#include <InitializeETrading.h>   // etrading::InitializeETrading
#include <tryAqToolSetup.h>       // validation::tryAqToolTearDown

// Addin Manager Info
struct AlgoQuantLib
{
    // xlOil constructs this struct once, from RegisterAddin::autoOpen() during
    // xlAutoOpen. It brings the library up: InitializeETrading::instance() loads
    // the holiday calendars, the IR static data and the optional startup config.
    // The date and curve functions assume this has run.
    //
    // Called with no arguments so a config-load failure does not abort the
    // add-in load; aqToolInitialize() re-runs it with the loud checks and
    // reports which config path was used.
    AlgoQuantLib()
    {
        etrading::InitializeETrading::instance();
    }

    // xlOil calls this from xlAutoClose / on unload. Goes through the same
    // centralized teardown as GTEST and AQ_API (validation::tryAqToolTearDown)
    // so the results containers, object pool and volatility manager are cleared
    // too, not just the InitializeETrading singleton.
    ~AlgoQuantLib()
    {
        validation::tryAqToolTearDown();
    }

    static std::wstring addInManagerInfo() { return L"Algo Quant Lib"; }
};
XLO_DECLARE_ADDIN(AlgoQuantLib);
