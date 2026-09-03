#include "CurveAccessors.h"

#include "YieldCurveUtil.h"

namespace google_test
{
    CurveAccessors::CurveAccessors( const LAString& inputFile )
        : fileLoaded_( false )
    {
        if ( inputFile.size() != 0 )
        {
            inputFile_ = etrading::ReadDataFile::Load( inputFile );
            curveID_ = etrading::getCurveID( inputFile_ );
            marketName_ = etrading::getMarketName( inputFile_ );
            curveNames_ = etrading::getCurveNames( inputFile_ );
            fileLoaded_ = true;
        }
    };
	
}


