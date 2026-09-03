#include "TryMeCurveAccessors.h"

#include "YieldCurveUtil.h"

namespace google_test
{
    TryMeCurveAccessors::TryMeCurveAccessors( const AQLString& inputFile )
        : fileLoaded_( false )
    {
        if ( inputFile.size() != 0 )
        {
            inputFile_ = etrading::ReadDataFile::Load( inputFile );
            fileLoaded_ = true;
        }
    };
}


