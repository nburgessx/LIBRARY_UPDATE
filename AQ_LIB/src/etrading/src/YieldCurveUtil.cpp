#include "YieldCurveUtil.h"

using etrading::ReadDataFile;

namespace etrading
{
    AQLString getCurveID( const ReadDataFile::Load& input )
    {
        return input["curveID"]();
    }

    AQLString getMarketName( const ReadDataFile::Load& input )
    {
        return input["marketName"]();
    }

    AQLStringVector getCurveNames( const ReadDataFile::Load& input )
    {
        AQLString curveNames = input["curveNames"]();
        return curveNames.toToken( ':' );
    }
}