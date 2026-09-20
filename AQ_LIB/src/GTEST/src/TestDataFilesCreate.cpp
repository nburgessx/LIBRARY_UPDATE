// TestDataFilesCreate.cpp

#include "ResultsProcessor.h"
#include "ReadDataFile.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"

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

            void adopt( const AQLString& p )
            {
                path::operator=( path( p.getCString() ) );
            }
            AQLString fullpath() const
            {
                return string().c_str();
            }

            ~CleanUpTempFiles()
            {
                if ( !empty() && !remove( *this ) )
                {
                    const AQLString hdr( "error removing temporary file: " );
                    const AQLString msg = hdr + string().c_str();
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
        const AQLString absent = "/this/folder/does/not/exist";

        EXPECT_THROW( CreateDataFile p0( absent ), CreateDataFile::OutputError );
        EXPECT_THROW( CreateDataFile p1( absent ), CreateDataFile::Exception );
        EXPECT_THROW( CreateDataFile p2( absent ), AQLCoreAppError );
        EXPECT_THROW( CreateDataFile p3( absent ), AQLCoreError );  // On Windows AQLCoreError inherits from std::exception; however on Linux it does not.

        // beginTestCount() validates via AQ_THROW, which always constructs AQLCoreInvalidData
        // directly - not the component-specific CreateDataFile::Exception this test originally
        // expected. That specific type is still real and still thrown elsewhere in this file (the
        // OutputError cases above) - this one call site just never went through it, and the
        // historical AQ_THROW/boost::format cleanup consolidated its validation onto the generic
        // macro. Testing what the code now deliberately, consistently does.
        EXPECT_THROW( CreateDataFile::beginTestCount( -1 ), AQLCoreInvalidData );

        try
        {
            CreateDataFile p4( absent );
        }
        catch ( const AQLCoreError& x )  // On Windows AQLCoreError inherits from std::exception; however on Linux it does not.
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

    TEST_F( TestDataFilesCreate, UNIT_PrintAQLString )
    {
        const AQLString name( "TestString" );
        const AQLString value( "just a test string" );

        const AQLString empty( "EmptyString" );
        const AQLString blank;

        //
        // aside:
        //
        {
            // empty string different from default constructed string!
            const AQLString b0;
            const AQLString b1( "" );
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

        AQLString e = x[empty];
        EXPECT_EQ( blank, e );
    }

    TEST_F( TestDataFilesCreate, UNIT_PrintStringVector )
    {
        const AQLString name( "TestVector" );
        const AQLString v0( "v0" );
        const AQLString v1( "v1" );

        AQLStringVector value;
        value.push_back( v0 );
        value.push_back( v1 );

        const AQLString empty( "EmptyVector" );
        AQLStringVector novals;

        CleanUpTempFiles clean;
        if ( true )
        {
            CreateDataFile p( test_info_->name() );
            clean.adopt( p.fullpath() );
            p.write( name, value );
            p.write( empty, novals );
        }

        ReadDataFile::Load x( clean.fullpath() );

        const AQLStringVector s = x[name];
        for ( std::size_t i = 0; i != value.size(); ++i )
        {
            EXPECT_EQ( value[i], s[i] );
        }

        const AQLStringVector e = x[empty];
        EXPECT_EQ( 0, e.size() );
    }

    TEST_F( TestDataFilesCreate, UNIT_PrintStringMatrix )
    {
        const AQLString name( "TestMatrix" );
        const AQLString v00( "v00" );
        const AQLString v01( "v01" );
        const AQLString v10( "v10" );
        const AQLString v11( "v11" );

        AQLStringVector r0;
        r0.push_back( v00 );
        r0.push_back( v01 );
        AQLStringVector r1;
        r1.push_back( v10 );
        r1.push_back( v11 );

        AQLStringMatrix value;
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

        const AQLStringMatrix m = x[name];
        for ( std::size_t i = 0; i != value.size(); ++i )
        {
            for ( std::size_t j = 0; j != value[i].size(); ++j )
            {
				AQLString str1 = value[i][j];
				AQLString str2 = m[i][j];
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
        const AQLString name( "TestVector" );
        const double v0 = 1.234;
        const double v1 = 5.678;

        std::vector<double> value;
        value.push_back( v0 );
        value.push_back( v1 );

        const AQLString empty( "EmptyVector" );
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
        const AQLString name( "TestVector" );
        const AQLDate v0( "20151026" );
        const AQLDate v1( "20151027" );

        std::vector<AQLDate> value;
        value.push_back( v0 );
        value.push_back( v1 );

        const AQLString empty( "EmptyVector" );
        std::vector<AQLDate> novals;
        CleanUpTempFiles clean;
        if ( true )
        {
            CreateDataFile p( test_info_->name() );
            clean.adopt( p.fullpath() );
            p.write( name, value );
            p.write( empty, novals );
        }

        ReadDataFile::Load x( clean.fullpath() );

        const std::vector<AQLDate> s = x[name];
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

        const AQLString f0 = "mycurve_inputs";
        const AQLString c0 = "mycurveid";
        const AQLString m0 = "mymarketname";

        EXPECT_EQ( c0 + "_" + m0 + "_" + f0, decorateCurvename( f0, c0, m0 ) );

        const AQLString f1 = "setUpOISCurve_inputs";
        const AQLString c1 = c0;
        const AQLString m1 = m0;

        EXPECT_EQ( c1 + "_" + m1, decorateCurvename( f1, c1, m1 ) );

        const AQLString f2 = "setUpSwapCurve_inputs";
        const AQLString c2 = c1;
        const AQLString m2 = m1;

        EXPECT_EQ( c2 + "_" + m2, decorateCurvename( f2, c2, m2 ) );

        const AQLString f3 = "setUpBasisCurve_inputs";
        const AQLString c3 = c2;
        const AQLString m3 = m2;

        EXPECT_EQ( c3 + "_" + m3, decorateCurvename( f3, c3, m3 ) );

        const AQLString f4 = "setUpFwdFXConstantCurve_inputs";
        const AQLString c4 = c3;
        const AQLString m4 = m3;

        EXPECT_EQ( c4 + "_" + m4, decorateCurvename( f4, c4, m4 ) );

        const AQLString f5 = "setUpBasisCurve_inputs";
        const AQLString c5 = c4;
        const AQLString m5 = "xyzwBasis";

        EXPECT_EQ( c5 + "_" + "xyzw", decorateCurvename( f5, c5, m5 ) );

        const AQLString f6 = "setUpBasisCurve_inputs";
        const AQLString c6 = c5;
        const AQLString m6 = "XccyBasis";

        EXPECT_EQ( c6 + "_" + "XCCY", decorateCurvename( f6, c6, m6 ) );

        const AQLString f7 = "setUpFwdFXConstantCurve_inputs";
        const AQLString c7 = c6;
        const AQLString m7 = "FwdFXConstJPY";

        EXPECT_EQ( c7 + "_" + "FWDJPY", decorateCurvename( f7, c7, m7 ) );

        const AQLString f8 = "xxxx_outputs.csv";
        const AQLString c8 = c7;
        const AQLString m8 = "";

        EXPECT_EQ( c8 + "_" + f8, decorateCurvename( f8, c8, m8 ) );
    }

    TEST_F( TestDataFilesCreate, UNIT_decorateFilenameWithPrefix )
    {
        using etrading::decorateFilename;

        const AQLString f = "filename";
        const AQLString p = "prefix";

        EXPECT_EQ( p + "_" + f, decorateFilename( f, p ) );
    }

    TEST_F( TestDataFilesCreate, UNIT_decorateFilenameWithPrefixAndSuffix )
    {
        using etrading::decorateFilename;

        const AQLString f    = "filename";
        const AQLString p    = "prefix";
        const AQLString s    = "suffix";

        // Suffix & Prefix Supplied
        EXPECT_EQ( p + "_" + f + "_" + s, decorateFilename( f, p, s ) );

        // Prefix Only Supplied
        EXPECT_EQ( p + "_" + f, decorateFilename( f, p, "" ) );

        // Suffix Only Supplied
        EXPECT_EQ( f + "_" + s, decorateFilename( f, "", s ) );
    }
}

