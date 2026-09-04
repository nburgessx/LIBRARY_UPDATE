#pragma once

#include "AQLString.h"
#include "AQLCoreTemplateType.h"
#include "ReadDataFile.h"

namespace google_test
{
    // implement and expose standard curve accessors
    class TryAqCurvesAccessors
    {
    public:
        TryAqCurvesAccessors( const AQLString& inputFilename );

        virtual ~TryAqCurvesAccessors() {}

    protected:
        etrading::ReadDataFile::Load inputFile_;
        bool fileLoaded_;
    };
}

