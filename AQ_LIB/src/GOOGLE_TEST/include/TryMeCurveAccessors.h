#pragma once

#include "AQLString.h"
#include "AQLCoreTemplateType.h"
#include "ReadDataFile.h"

namespace google_test
{
    // implement and expose standard curve accessors
    class TryMeCurveAccessors
    {
    public:
        TryMeCurveAccessors( const AQLString& inputFilename );

        virtual ~TryMeCurveAccessors() {}

    protected:
        etrading::ReadDataFile::Load inputFile_;
        bool fileLoaded_;
    };
}

