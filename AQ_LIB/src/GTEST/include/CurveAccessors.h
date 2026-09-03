#pragma once

#include "AQLString.h"
#include "AQLCoreTemplateType.h"
#include "ReadDataFile.h"

namespace google_test
{
    // implement and expose standard curve accessors
    class CurveAccessors
    {
    public:
        CurveAccessors( const AQLString& inputFilename );

        virtual AQLString getCurveID() const
        {
            return curveID_;
        }
        virtual AQLString getMarketName() const
        {
            return marketName_;
        }
        virtual const AQLStringVector& getCurveNames() const
        {
            return curveNames_;
        }

        virtual ~CurveAccessors() {}

    protected:
        etrading::ReadDataFile::Load inputFile_;
        AQLString curveID_;
        AQLString marketName_;
        AQLStringVector curveNames_;
        bool fileLoaded_;
    };
}

