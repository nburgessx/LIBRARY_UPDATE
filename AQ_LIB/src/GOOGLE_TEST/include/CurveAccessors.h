#pragma once

#include "LAString.h"
#include "LACoreTemplateType.h"
#include "ReadDataFile.h"

namespace google_test
{
    // implement and expose standard curve accessors
    class CurveAccessors
    {
    public:
        CurveAccessors( const LAString& inputFilename );

        virtual LAString getCurveID() const
        {
            return curveID_;
        }
        virtual LAString getMarketName() const
        {
            return marketName_;
        }
        virtual const LAStringVector& getCurveNames() const
        {
            return curveNames_;
        }

        virtual ~CurveAccessors() {}

    protected:
        etrading::ReadDataFile::Load inputFile_;
        LAString curveID_;
        LAString marketName_;
        LAStringVector curveNames_;
        bool fileLoaded_;
    };
}

