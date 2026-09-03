// TestCurveXccyBasisGBP.cpp

#include "CurveOis.h"
#include "CurveStd.h"
#include "CurveTenorBasis.h"
#include "CurveXccyBasis.h"

#include "BindFileToClassConstructor.h"
#include "Dependency.h"
#include "ReadDataFile.h"
#include "ResultsProcessor.h"
#include "tryMirOutputCurve.h"

#include <gTest/gTest.h>

using google_test::CurveOis;
using google_test::CurveStd;
using google_test::CurveTenorBasis;
using google_test::CurveXccyBasis;

using google_test::BindFileToClassConstructor;
using google_test::Dependency0;
using google_test::Dependency1;
using google_test::Dependency2;
using google_test::Dependency3;
using google_test::Dependency4;

using etrading::ReadDataFile;

#define TEST_DIR "Vanilla/Curves/TestCurveXccyBasisGBP/"

namespace
{

    // test tolerance
    // --------------

#ifdef GTEST32
    // This Test was recorded in WIN32 so higher precision here i.e. better match against test results
    const double tolerance = 1e-8;
#else
    // The test should still pass in WIN64, but since the test was recorded using WIN32, results won't
    // match the expected hard-coded results too well
    const double tolerance = 1e-3;
#endif



    //
    // curve input files
    //
    extern const char USDYC_OIS[]			= TEST_DIR "USDYC_OIS_tryMirSetUpOISCurve_inputs";
    extern const char USDYC_STD[]			= TEST_DIR "USDYC_STD_tryMirSetUpSwapCurve_inputs";
    extern const char GBPYC_OIS[]			= TEST_DIR "GBPYC_OIS_tryMirSetUpOISCurve_inputs";
    extern const char GBPYC_STD[]			= TEST_DIR "GBPYC_STD_tryMirSetUpSwapCurve_inputs";
    extern const char GBPYC_3M6M[]			= TEST_DIR "GBPYC_3M6M_tryMirSetUpBasisSwapCurve_inputs";
    extern const char GBPYC_XCCY[]			= TEST_DIR "GBPYC_XCCY_tryMirSetUpBasisSwapCurve_inputs";

    //
    // test call input and reference files
    //
    extern const char OutPutDFReference_32bit[]	= TEST_DIR "GBPYC_XCCY_tryMirOutputCurve_outputs_32bit";
	extern const char OutPutDFReference_64bit[]	= TEST_DIR "GBPYC_XCCY_tryMirOutputCurve_outputs_64bit";
}

namespace google_test
{

    //
    // Bind Inputs to Sources
    //

    typedef BindFileToClassConstructor<CurveOis,			USDYC_OIS>		SourceUSDYC_OIS;
    typedef BindFileToClassConstructor<CurveStd,			USDYC_STD>		SourceUSDYC_STD;
    typedef BindFileToClassConstructor<CurveOis,			GBPYC_OIS>		SourceGBPYC_OIS;
    typedef BindFileToClassConstructor<CurveStd,			GBPYC_STD>		SourceGBPYC_STD;
    typedef BindFileToClassConstructor<CurveTenorBasis,	GBPYC_3M6M>		SourceGBPYC_3M6M;
    typedef BindFileToClassConstructor<CurveXccyBasis,	GBPYC_XCCY>		SourceGBPYC_XCCY;

    //
    // Declare Dependencies
    //

    typedef Dependency0<SourceUSDYC_OIS>
    CurveUSDYC_OIS;
    typedef Dependency1<SourceUSDYC_STD, CurveUSDYC_OIS>
    CurveUSDYC_STD;
    typedef Dependency0<SourceGBPYC_OIS>
    CurveGBPYC_OIS;
    typedef Dependency1<SourceGBPYC_STD, CurveGBPYC_OIS>
    CurveGBPYC_STD;
    typedef Dependency2<SourceGBPYC_3M6M, CurveGBPYC_STD, CurveGBPYC_OIS>
    CurveGBPYC_3M6M;
    typedef Dependency4<SourceGBPYC_XCCY, CurveUSDYC_STD, CurveUSDYC_OIS, CurveGBPYC_OIS, CurveGBPYC_3M6M>
    CurveGBPYC_XCCY;

    //
    // SetUp Test Fixture
    //

    class TestCurveXccyBasisGBP: public testing::Test, public CurveGBPYC_XCCY
    {
        void testGraphAtCompileTime()
        {
            // make sure that conversion to test base class is unambiguous
            auto& g0 = static_cast<google_test::InitializeAQGoogleTest&>( *this );

            // make sure that conversion to source nodes is unambiguous
            auto& s0 = static_cast<SourceUSDYC_OIS&>( *this );
            auto& s1 = static_cast<SourceUSDYC_STD&>( *this );
            auto& s2 = static_cast<SourceGBPYC_OIS&>( *this );
            auto& s3 = static_cast<SourceGBPYC_STD&>( *this );
            auto& s4 = static_cast<SourceGBPYC_3M6M&>( *this );
            auto& s5 = static_cast<SourceGBPYC_XCCY&>( *this );

            // make sure that conversion to curve nodes is unambiguous
            auto& c0 = static_cast<CurveUSDYC_OIS&>( *this );
            auto& c1 = static_cast<CurveUSDYC_STD&>( *this );
            auto& c2 = static_cast<CurveGBPYC_OIS&>( *this );
            auto& c3 = static_cast<CurveGBPYC_STD&>( *this );
            auto& c4 = static_cast<CurveGBPYC_3M6M&>( *this );
            auto& c5 = static_cast<CurveGBPYC_XCCY&>( *this );
        }

    public:
        //
        // disambiguate accessors
        //
        virtual LAString getCurveID() const
        {
            return SourceGBPYC_XCCY::getCurveID();
        }
        virtual const LAStringVector& getCurveNames() const
        {
            return SourceGBPYC_XCCY::getCurveNames();
        }
    };

    //
    // Call Test Fixture
    //

    TEST_F( TestCurveXccyBasisGBP, SNAPSHOT_OutPutDF )
    {
        const DoubleArray results
            = validation_api::tryMirOutputCurve(
                  getDataInstance(),
                  getCurveID(),
                  getCurveNames().at( 0 ) );

        #if defined(GTEST32)
        CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, OutPutDFReference_32bit, tolerance );
		#else
		CheckTestResultsAndRebaseOnRequest( results, TEST_DIR, OutPutDFReference_64bit, tolerance );
		#endif
    }
}
