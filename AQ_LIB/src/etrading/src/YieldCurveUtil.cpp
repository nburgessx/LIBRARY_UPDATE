#include "YieldCurveUtil.h"

using etrading::ReadDataFile;

namespace etrading
{
    LAString getCurveID( const ReadDataFile::Load& input )
    {
        return input["curveID"]();
    }

    LAString getMarketName( const ReadDataFile::Load& input )
    {
        return input["marketName"]();
    }

    LAStringVector getCurveNames( const ReadDataFile::Load& input )
    {
        LAString curveNames = input["curveNames"]();
        return curveNames.toToken( ':' );
    }
}