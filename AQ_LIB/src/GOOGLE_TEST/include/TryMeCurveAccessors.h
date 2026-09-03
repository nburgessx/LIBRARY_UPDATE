#pragma once

#include "LAString.h"
#include "LACoreTemplateType.h"
#include "ReadDataFile.h"

namespace google_test
{
    // implement and expose standard curve accessors
    class TryMeCurveAccessors
    {
    public:
        TryMeCurveAccessors( const LAString& inputFilename );

        virtual ~TryMeCurveAccessors() {}

    protected:
        etrading::ReadDataFile::Load inputFile_;
        bool fileLoaded_;
    };
}

