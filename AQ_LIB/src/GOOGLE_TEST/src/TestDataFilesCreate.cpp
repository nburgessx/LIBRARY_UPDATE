// TestDataFilesCreate.cpp

#include "ResultsProcessor.h"
#include "ReadDataFile.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"

#include <gTest/gTest.h>

#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>

#include <vector>
#include <cstring>
#include <stdexcept>

using etrading::CreateDataFile;
using etrading::ReadDataFile;

namespace google_test
{
    //
    // SetUp Test Fixture
    //
    class TestDataFilesCreate : public testing::Test
    {
    protected:
        TestDataFilesCreate() {}

        // Setup gets invoked immediately before a test starts
        virtual void SetUp()
        {
        }

        // TearDown gets invoked immediately after a test finishes
        virtual void TearDown()
        {
        }

    private:
    };

    //
    // Call Test Fixture
    //

    using boost::filesystem::path;

    namespace
    {
        //
        // helper class for cleaning up temporary files
        //
        using boost::filesystem::remove;
        class CleanUpTempFiles : private path
        {
        public:
            CleanUpTempFiles( const path& p = path() ) : path( p ) {}

            void adopt( const LAString& p )
            {
                path::operator=( path( p.getCString() ) );
            }
            LAString fullpath() const
            {
                return string().c_str();
            }

            ~CleanUpTempFiles()
            {
                if ( !empty() && !remove( *this ) )
                {
                    const LAString hdr( "error removing temporary file: " );
                    const LAString msg = hdr + string().c_str();
                    std::cerr << "exception: " << msg << std::endl;
                    // throw CreateDataFile::Exception(msg + string().c_str());
                }
            }
        private:
            CleanUpTempFiles( const CleanUpTempFiles& );
            CleanUpTempFiles& operator=( const CleanUpTempFiles& );
        };

        class ResetTestCounter
        {
        public:
            ResetTestCounter()
            {
                CreateDataFile::beginTestCount();
            }
            ~ResetTestCounter()
            {
                CreateDataFile::endTestCount();
            }
        };

        class CallCaptureRestorer
        {
        public:
            CallCaptureRestorer()
                : recordEnabled_( CreateDataFile::recordEnabled() )
            {}

            ~CallCaptureRestorer()
            {
                CreateDataFile::setRecordFlag( recordEnabled_ );
            }

        private:
            bool recordEnabled_;
        };
    }

    TEST_F( TestDataFilesCreate, UNIT_ErrorHandling )
    {
        const LAString absent = "/this/folder/does/not/exist";

        EXPECT_THROW( CreateDataFile p0( absent ), CreateDataFile::OutputError );
        EXPECT_THROW( CreateDataFile p1( absent ), CreateDataFile::Exception );
        EXPECT_THROW( CreateDataFile p2( absent ), LACoreAppError );
        EXPECT_THROW( CreateDataFile p3( absent ), LACoreError );  // On Windows LACoreError inherits from std::exception; however on Linux it does not.

        EXPECT_THROW( CreateDataFile::beginTestCount( -1 ), CreateDataFile::Exception );

        try
        {
            CreateDataFile p4( absent );
        }
        catch ( const LACoreError& x )  // On Windows LACoreError inherits from std::exception; however on Linux it does not.
        {
            std::cerr << "x.what(): " << x.what() << std::endl;
        }
    }

    TEST_F( TestDataFilesCreate, UNIT_Basics )
    {
        CleanUpTempFiles g;
        CreateDataFile p( "xxx1.yyy", ".yyy" );
        g.adopt( p.fullpath() );

        EXPECT_EQ( "xxx1.yyy", p.filename() );

        CleanUpTempFiles h;
        CreateDataFile q( "xxx2.yyy", ".zzz" );
        h.adopt( q.fullpath() );

        EXPECT_EQ( "xxx2.yyy.zzz", q.filename() );

        CleanUpTempFiles i;
        CreateDataFile r( "xxx3.yyy", "" );
        i.adopt( r.fullpath() );

        EXPECT_EQ( "xxx3.yyy", r.filename() );
    }

    TEST_F( TestDataFilesCreate, UNIT_RecordFlagEnabled )
    {
        CallCaptureRestorer reset;

        CreateDataFile::setRecordFlag( true );
        EXPECT_EQ( true, CreateDataFile::recordEnabled() );

        CreateDataFile::setRecordFlag( false );
        EXPECT_EQ( false, CreateDataFile::recordEnabled() );
    }

    TEST_F( TestDataFilesCreate, UNIT_PrintLAString )
    {
        const LAString name( "TestString" );
        const LAString value( "just a test string" );

        const LAString empty( "EmptyString" );
        const LAString blank;

        //
        // aside:
        //
        {
            // empty string different from default constructed string!
            const LAString b0;
            const LAString b1( "" );
            EXPECT_NE( b0, b1 );

            // contrast with std::string:
            const std::string c0;
            const std::string c1( "" );
            EXPECT_EQ( c0, c1 );
        }

        CleanUpTempFiles clean;
        if ( true )
        {
            CreateDataFile p( test_info_->name() );
            clean.adopt( p.fullpath() );
            p.write( name, value );
            p.write( empty, blank );
        }

        ReadDataFile::Load x( clean.fullpath() );

        EXPECT_EQ( value, x[name]() );

        LAString e = x[empty];
        EXPECT_EQ( blank, e );
    }

    TEST_F( TestDataFilesCreate, UNIT_PrintStringVector )
    {
        const LAString name( "TestVector" );
        const LAString v0( "v0" );
        const LAString v1( "v1" );

        LAStringVector value;
        value.push_back( v0 );
        value.push_back( v1 );

        const LAString empty( "EmptyVector" );
        LAStringVector novals;

        CleanUpTempFiles clean;
        if ( true )
        {
            CreateDataFile p( test_info_->name() );
            clean.adopt( p.fullpath() );
            p.write( name, value );
            p.write( empty, novals );
        }

        ReadDataFile::Load x( clean.fullpath() );

        const LAStringVector s = x[name];
        for ( std::size_t i = 0; i != value.size(); ++i )
        {
            EXPECT_EQ( value[i], s[i] );
        }

        const LAStringVector e = x[empty];
        EXPECT_EQ( 0, e.size() );
    }

    TEST_F( TestDataFilesCreate, UNIT_PrintStringMatrix )
    {
        const LAString name( "TestMatrix" );
        const LAString v00( "v00" );
        const LAString v01( "v01" );
        const LAString v10( "v10" );
        const LAString v11( "v11" );

        LAStringVector r0;
        r0.push_back( v00 );
        r0.push_back( v01 );
        LAStringVector r1;
        r1.push_back( v10 );
        r1.push_back( v11 );

        LAStringMatrix value;
        value.push_back( r0 );
        value.push_back( r1 );

        CleanUpTempFiles clean;
        if ( true )
        {
            CreateDataFile p( test_info_->name() );
            clean.adopt( p.fullpath() );
            p.write( name, value );
        }

        ReadDataFile::Load x( clean.fullpath() );

        const LAStringMatrix m = x[name];
        for ( std::size_t i = 0; i != value.size(); ++i )
        {
            for ( std::size_t j = 0; j != value[i].size(); ++j )
            {
				LAString str1 = value[i][j];
				LAString str2 = m[i][j];
				if (j == 0)
				{
					// First columm holds keys that are all upper case
					str1.toUpper();
				}
                EXPECT_EQ( str1,  str2);
            }
        }
    }

    TEST_F( TestDataFilesCreate, UNIT_PrintDoubleVector )
    {
        const LAString name( "TestVector" );
        const double v0 = 1.234;
        const double v1 = 5.678;

        std::vector<double> value;
        value.push_back( v0 );
        value.push_back( v1 );

        const LAString empty( "EmptyVector" );
        std::vector<double> novals;
        CleanUpTempFiles clean;
        if ( true )
        {
            CreateDataFile p( test_info_->name() );
            clean.adopt( p.fullpath() );
            p.write( name, value );
            p.write( empty, novals );
        }

        ReadDataFile::Load x( clean.fullpath() );

        const std::vector<double> s = x[name];
        for ( std::size_t i = 0; i != value.size(); ++i )
        {
            EXPECT_EQ( value[i], s[i] );
        }

        const std::vector<double> e = x[empty];
        EXPECT_EQ( 0, e.size() );
    }

    TEST_F( TestDataFilesCreate, UNIT_PrintDateVector )
    {
        const LAString name( "TestVector" );
        const LADate v0( "20151026" );
        const LADate v1( "20151027" );

        std::vector<LADate> value;
        value.push_back( v0 );
        value.push_back( v1 );

        const LAString empty( "EmptyVector" );
        std::vector<LADate> novals;
        CleanUpTempFiles clean;
        if ( true )
        {
            CreateDataFile p( test_info_->name() );
            clean.adopt( p.fullpath() );
            p.write( name, value );
            p.write( empty, novals );
        }

        ReadDataFile::Load x( clean.fullpath() );

        const std::vector<LADate> s = x[name];
        for ( std::size_t i = 0; i != value.size(); ++i )
        {
            EXPECT_EQ( value[i], s[i] );
        }

        const std::vector<double> e = x[empty];
        EXPECT_EQ( 0, e.size() );
    }

    TEST_F( TestDataFilesCreate, UNIT_VectorisedOutput )
    {
        typedef boost::shared_ptr<CreateDataFile> pPrinter;
        typedef boost::shared_ptr<CleanUpTempFiles> pCleaner;

        std::vector<pCleaner> cs;

        if ( true )
        {
            std::vector<pPrinter> ps;
            ResetTestCounter reset;
            for ( int i = 0; i != 10; ++i )
            {
                ps.push_back( pPrinter( new CreateDataFile( test_info_->name(), ".xxx" ) ) );
                cs.push_back( pCleaner( new CleanUpTempFiles( ps.back()->fullpath().getCString() ) ) );
            }
        }
    }

    TEST_F( TestDataFilesCreate, UNIT_decorateCurvename )
    {
        using etrading::decorateCurvename;

        const LAString f0 = "mycurve_inputs";
        const LAString c0 = "mycurveid";
        const LAString m0 = "mymarketname";

        EXPECT_EQ( c0 + "_" + m0 + "_" + f0, decorateCurvename( f0, c0, m0 ) );

        const LAString f1 = "setUpOISCurve_inputs";
        const LAString c1 = c0;
        const LAString m1 = m0;

        EXPECT_EQ( c1 + "_" + m1, decorateCurvename( f1, c1, m1 ) );

        const LAString f2 = "setUpSwapCurve_inputs";
        const LAString c2 = c1;
        const LAString m2 = m1;

        EXPECT_EQ( c2 + "_" + m2, decorateCurvename( f2, c2, m2 ) );

        const LAString f3 = "setUpBasisCurve_inputs";
        const LAString c3 = c2;
        const LAString m3 = m2;

        EXPECT_EQ( c3 + "_" + m3, decorateCurvename( f3, c3, m3 ) );

        const LAString f4 = "setUpFwdFXConstantCurve_inputs";
        const LAString c4 = c3;
        const LAString m4 = m3;

        EXPECT_EQ( c4 + "_" + m4, decorateCurvename( f4, c4, m4 ) );

        const LAString f5 = "setUpBasisCurve_inputs";
        const LAString c5 = c4;
        const LAString m5 = "xyzwBasis";

        EXPECT_EQ( c5 + "_" + "xyzw", decorateCurvename( f5, c5, m5 ) );

        const LAString f6 = "setUpBasisCurve_inputs";
        const LAString c6 = c5;
        const LAString m6 = "XccyBasis";

        EXPECT_EQ( c6 + "_" + "XCCY", decorateCurvename( f6, c6, m6 ) );

        const LAString f7 = "setUpFwdFXConstantCurve_inputs";
        const LAString c7 = c6;
        const LAString m7 = "FwdFXConstJPY";

        EXPECT_EQ( c7 + "_" + "FWDJPY", decorateCurvename( f7, c7, m7 ) );

        const LAString f8 = "xxxx_outputs.csv";
        const LAString c8 = c7;
        const LAString m8 = "";

        EXPECT_EQ( c8 + "_" + f8, decorateCurvename( f8, c8, m8 ) );
    }

    TEST_F( TestDataFilesCreate, UNIT_decorateFilenameWithPrefix )
    {
        using etrading::decorateFilename;

        const LAString f = "filename";
        const LAString p = "prefix";

        EXPECT_EQ( p + "_" + f, decorateFilename( f, p ) );
    }

    TEST_F( TestDataFilesCreate, UNIT_decorateFilenameWithPrefixAndSuffix )
    {
        using etrading::decorateFilename;

        const LAString f    = "filename";
        const LAString p    = "prefix";
        const LAString s    = "suffix";

        // Suffix & Prefix Supplied
        EXPECT_EQ( p + "_" + f + "_" + s, decorateFilename( f, p, s ) );

        // Prefix Only Supplied
        EXPECT_EQ( p + "_" + f, decorateFilename( f, p, "" ) );

        // Suffix Only Supplied
        EXPECT_EQ( f + "_" + s, decorateFilename( f, "", s ) );
    }
}

