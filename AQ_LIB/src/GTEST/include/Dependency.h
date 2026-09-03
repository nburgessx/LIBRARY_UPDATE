#pragma once

#include <gTest/gTest.h>
#include "InitializeAQGoogleTest.h"
#include "BindFileToClassConstructor.h"
#include "CurveResultsContainer.h"

namespace google_test
{
    // Vanilla 'Mir' Curve Constructors
    class CurveOis;
    class CurveStd;
    class CurveTenorBasis;

    // Etrading 'Me' Curve Constructors
    class TryMeCurveOis;
    class TryMeCurveStd;
    class TryMeCurveTenorBasis;
    class TryMeCurveXccyBasis;
    class TryMeCurveFwdFxConst;

    //
    // CLASS
    //   google_test::Dependency
    //
    // PURPOSE
    //   Declare dependencies between curves.
    //
    // SYNPOSIS
    //   #include "Dependency.h"
    //
    //   using google_test::Dependency0;
    //   using google_test::Dependency1;
    //
    //   typedef Dependency0< SourceEURYC_OIS >                 CurveEURYC_OIS;
    //   typedef Dependency1< SourceEURYC_STD, CurveEURYC_OIS > CurveEURYC_STD;
    //
    //   class TestFixture : public testing::Test, public CurveEURYC_STD {};
    //

    //
    // IMPORTANT NOTE:
    // The dependency functions below are using a mixture of non-virtual and virtual inheritance.
    // As such all virtual constructors are called first and in order of declarations followed by
    // all non-virtual constructors in order of declaration
    //

    // HOW DO THESE MACROS WORK?
    //   The dependency template just calls the BindFileToClassConstructor routines in the correct order using virtual inheritance
    //   The BindFileToClassConstructor template calls the curve constructor and takes two arguments the first is the curve class constructor
    //   and the second is the filepath to the curve data required by the curve constructor.

    // Build Order: Src
    template< typename Src >
    class Dependency0 : public Src {};

    // Build Order: D1, Src
    template< typename Src, typename D1 >
    class Dependency1 : public Src, protected virtual D1 {};

    // Build Order: D1, D2, Src
    template< typename Src, typename D1, typename D2 >
    class Dependency2 : public Src, protected virtual D1, protected virtual D2 {};

    // Build Order: D1, D2, D3, Src
    template< typename Src, typename D1, typename D2, typename D3 >
    class Dependency3 : public Src, protected virtual D1, protected virtual D2, protected virtual D3 {};

    // Build Order: D1, D2, D3, D4, Src
    template< typename Src, typename D1, typename D2, typename D3, typename D4 >
    class Dependency4 : public Src, protected virtual D1, protected virtual D2, protected virtual D3, protected virtual D4 {};

    // Build Order: D1, D2, D3, D4, D5, Src
    template< typename Src, typename D1, typename D2, typename D3, typename D4, typename D5 >
    class Dependency5 : public Src, protected virtual D1, protected virtual D2, protected virtual D3, protected virtual D4, protected virtual D5 {};

    // Build Order: D1, D2, D3, D4, D5, Src
    template< typename Src, typename D1, typename D2, typename D3, typename D4, typename D5, typename D6, typename D7, typename D8, typename D9, typename D10 >
    class Dependency10 : public Src, protected virtual D1, protected virtual D2, protected virtual D3, protected virtual D4, protected virtual D5
                                  , protected virtual D6, protected virtual D7, protected virtual D8, protected virtual D9, protected virtual D10 {};

    //
    // Curve Macros for the Vanilla Addin
    // Use when building curves using the 'Mir' Functions
    // --------------------------------------------------------------------------------------------------------------
    //

    // HOW DO THESE MACROS WORK?
    //   The dependency template just calls the BindFileToClassConstructor routines in the correct order using virtual inheritance
    //   The BindFileToClassConstructor template calls the curve constructor and takes two arguments the first is the curve class constructor
    //   and the second is the filepath to the curve data required by the curve constructor.

    // Macro that builds all EUR curves in order, using dependency4 above, as long as curve source files are given
    // Build Order: OIS, 6ML, 3ML, 12ML, 1ML
#define ADD_EUR_CURVE_DEPENDENCIES(test_name, source_ois, source_1m, source_3m, source_6m, source_12m)	\
    class test_name : public testing::Test, public virtual google_test::InitializeAQGoogleTest, public Dependency4<  BindFileToClassConstructor< CurveTenorBasis, source_1m >,	\
        BindFileToClassConstructor< CurveOis,		  source_ois >,	\
        BindFileToClassConstructor< CurveStd,         source_6m >,	\
        BindFileToClassConstructor< CurveTenorBasis,  source_3m >,	\
        BindFileToClassConstructor< CurveTenorBasis,  source_12m > > {};

    // Macro that builds all JPY curves in order as long as curve source files are given
    // Build Order: OIS, 6ML, 3ML, 1ML, 12ML
#define ADD_JPY_CURVE_DEPENDENCIES(test_name, source_ois, source_1m, source_3m, source_6m, source_12m)	\
    class test_name : public testing::Test, public virtual google_test::InitializeAQGoogleTest, public Dependency4<  BindFileToClassConstructor< CurveTenorBasis, source_12m >,	\
        BindFileToClassConstructor< CurveOis,		   source_ois >,	\
        BindFileToClassConstructor< CurveStd,          source_6m >,	\
        BindFileToClassConstructor< CurveTenorBasis,   source_3m >,	\
        BindFileToClassConstructor< CurveTenorBasis,   source_1m > > {};

    // Macro that builds all USD curves in order as long as curve source files are given
    // Build Order: OIS, 3ML, 6ML, 12ML, 1ML
#define ADD_USD_CURVE_DEPENDENCIES(test_name, source_ois, source_1m, source_3m, source_6m, source_12m)	\
    class test_name : public testing::Test, public virtual google_test::InitializeAQGoogleTest, public Dependency4<  BindFileToClassConstructor< CurveTenorBasis, source_1m >,	\
        BindFileToClassConstructor< CurveOis,		  source_ois >,	\
        BindFileToClassConstructor< CurveStd,         source_3m >,	\
        BindFileToClassConstructor< CurveTenorBasis,  source_6m >,	\
        BindFileToClassConstructor< CurveTenorBasis,  source_12m > > {};

    // Macro that builds all GBP curves in order as long as curve source files are given
    // Build Order: OIS, 6ML, 3ML, 12ML, 1ML
#define ADD_GBP_CURVE_DEPENDENCIES(test_name, source_ois, source_1m, source_3m, source_6m, source_12m)	\
    class test_name : public testing::Test, public virtual google_test::InitializeAQGoogleTest, public Dependency4<  BindFileToClassConstructor< CurveTenorBasis, source_1m >,	\
        BindFileToClassConstructor< CurveOis,		  source_ois >,	\
        BindFileToClassConstructor< CurveStd,         source_6m >,	\
        BindFileToClassConstructor< CurveTenorBasis,  source_3m >,	\
        BindFileToClassConstructor< CurveTenorBasis,  source_12m > >	{};


    //
    // Curve Macros for the ETrading Addin
    // Use when building curves using the 'Me' Functions
    // --------------------------------------------------------------------------------------------------------------
    //

    // HOW DO THESE MACROS WORK?
    //   The dependency template just calls the BindFileToClassConstructor routines in the correct order using virtual inheritance
    //   The BindFileToClassConstructor template calls the curve constructor and takes two arguments the first is the curve class constructor
    //   and the second is the filepath to the curve data required by the curve constructor.


    // Macro that builds all EUR curves in order, using dependency4 above, as long as curve source files are given
    // Build Order: OIS, 6ML, 3ML, 12ML, 1ML
#define AQL_BUILD_EUR_CURVE(test_name, source_ois, source_1m, source_3m, source_6m, source_12m)	\
    class test_name : public testing::Test, public virtual google_test::InitializeAQGoogleTest, public Dependency4<  BindFileToClassConstructor< TryMeCurveTenorBasis, source_1m >,	\
        BindFileToClassConstructor< TryMeCurveOis,		  source_ois >,	\
        BindFileToClassConstructor< TryMeCurveStd,        source_6m >,	\
        BindFileToClassConstructor< TryMeCurveTenorBasis, source_3m >,	\
        BindFileToClassConstructor< TryMeCurveTenorBasis, source_12m > > {};

    // Macro that builds all JPY curves in order as long as curve source files are given
    // Build Order: OIS, 6ML, 3ML, 1ML, 12ML
#define AQL_BUILD_JPY_CURVE(test_name, source_ois, source_1m, source_3m, source_6m, source_12m)	\
    class test_name : public testing::Test, public virtual google_test::InitializeAQGoogleTest, public Dependency4<  BindFileToClassConstructor< TryMeCurveTenorBasis, source_12m >,	\
        BindFileToClassConstructor< TryMeCurveOis,		  source_ois >,	\
        BindFileToClassConstructor< TryMeCurveStd,         source_6m >,	\
        BindFileToClassConstructor< TryMeCurveTenorBasis,  source_3m >,	\
        BindFileToClassConstructor< TryMeCurveTenorBasis,  source_1m > > {};

    // Macro that builds all USD curves in order as long as curve source files are given
    // Build Order: OIS, 3ML, 6ML, 12ML, 1ML
#define AQL_BUILD_USD_CURVE(test_name, source_ois, source_1m, source_3m, source_6m, source_12m)	\
    class test_name : public testing::Test, public virtual google_test::InitializeAQGoogleTest, public Dependency4<  BindFileToClassConstructor< TryMeCurveTenorBasis, source_1m >,	\
        BindFileToClassConstructor< TryMeCurveOis,		  source_ois >,	\
        BindFileToClassConstructor< TryMeCurveStd,         source_3m >,	\
        BindFileToClassConstructor< TryMeCurveTenorBasis,  source_6m >,	\
        BindFileToClassConstructor< TryMeCurveTenorBasis,  source_12m > >	{};

    // Macro that builds all GBP curves in order as long as curve source files are given
    // Build Order: OIS, 6ML, 3ML, 12ML, 1ML
#define AQL_BUILD_GBP_CURVE(test_name, source_ois, source_1m, source_3m, source_6m, source_12m)	\
    class test_name : public testing::Test, public virtual google_test::InitializeAQGoogleTest, public Dependency4<  BindFileToClassConstructor< TryMeCurveTenorBasis, source_1m >,	\
        BindFileToClassConstructor< TryMeCurveOis,		  source_ois >,	\
        BindFileToClassConstructor< TryMeCurveStd,         source_6m >,	\
        BindFileToClassConstructor< TryMeCurveTenorBasis,  source_3m >,	\
        BindFileToClassConstructor< TryMeCurveTenorBasis,  source_12m > >	{};


    //
    // XCCY CURVE MACRO(S)
    //

    // HOW DO THESE MACROS WORK?
    //   The dependency template just calls the BindFileToClassConstructor routines in the correct order using virtual inheritance
    //   The BindFileToClassConstructor template calls the curve constructor and takes two arguments the first is the curve class constructor
    //   and the second is the filepath to the curve data required by the curve constructor.


    // Macro that builds all EURUSD Xccy curves in order, using dependency5 above, as long as curve source files are given
    // Build Order: USDOIS, USD3M, EUROIS, EUR6M, EUR3M, EURUSDXCCY
#define AQL_BUILD_EURUSD_XCCY_CURVE(test_name, usd_source_ois, usd_source_3m, eur_source_ois, eur_source_6m, eur_source_3m, eur_source_xccy ) \
    class test_name : public testing::Test, public virtual google_test::InitializeAQGoogleTest, public Dependency5<  BindFileToClassConstructor<  TryMeCurveXccyBasis, eur_source_xccy  >,   \
        BindFileToClassConstructor< TryMeCurveOis,	        usd_source_ois >,	\
        BindFileToClassConstructor< TryMeCurveStd,          usd_source_3m >,	\
        BindFileToClassConstructor< TryMeCurveOis,	        eur_source_ois >,	\
        BindFileToClassConstructor< TryMeCurveStd,          eur_source_6m >,	\
        BindFileToClassConstructor< TryMeCurveTenorBasis,   eur_source_3m  > > {};



#define AQL_BUILD_EURUSD_JPYUSD_XCCY_CURVE(test_name, usd_source_ois, usd_source_3m, eur_source_ois, eur_source_6m, eur_source_3m, eur_source_xccy, jpy_source_ois, jpy_source_6m_jscc, jpy_source_6m_lch, jpy_source_3m_lch, jpy_source_xccy ) \
    class test_name : public testing::Test, public virtual google_test::InitializeAQGoogleTest, public Dependency10<  BindFileToClassConstructor<  TryMeCurveXccyBasis, jpy_source_xccy  >,   \
        BindFileToClassConstructor< TryMeCurveOis,	        usd_source_ois >,	\
        BindFileToClassConstructor< TryMeCurveStd,          usd_source_3m >,	\
		BindFileToClassConstructor< TryMeCurveOis,	        eur_source_ois >,	\
        BindFileToClassConstructor< TryMeCurveStd,          eur_source_6m >,	\
        BindFileToClassConstructor< TryMeCurveTenorBasis,   eur_source_3m  >,	\
		BindFileToClassConstructor< TryMeCurveXccyBasis,    eur_source_xccy  >, \
        BindFileToClassConstructor< TryMeCurveOis,	        jpy_source_ois >,	\
        BindFileToClassConstructor< TryMeCurveStd,          jpy_source_6m_jscc >, \
		BindFileToClassConstructor< TryMeCurveTenorBasis,   jpy_source_6m_lch >,  \
		BindFileToClassConstructor< TryMeCurveTenorBasis,   jpy_source_3m_lch > >  {};
 

#define AQL_BUILD_GBPEUR_FWD_CURVE(test_name, usd_source_ois, usd_source_3m, eur_source_ois, eur_source_6m, eur_source_3m, eur_source_xccy, gbp_source_ois, gbp_source_6m, gbp_source_3m, gbp_source_xccy, gbp_eur_csa ) \
    class test_name : public testing::Test, public virtual google_test::InitializeAQGoogleTest, public Dependency10<  BindFileToClassConstructor<  TryMeCurveFwdFxConst, gbp_eur_csa  >,   \
        BindFileToClassConstructor< TryMeCurveOis,	        usd_source_ois >,	\
        BindFileToClassConstructor< TryMeCurveStd,          usd_source_3m >,	\
        BindFileToClassConstructor< TryMeCurveOis,	        eur_source_ois >,	\
        BindFileToClassConstructor< TryMeCurveStd,          eur_source_6m >,	\
        BindFileToClassConstructor< TryMeCurveTenorBasis,   eur_source_3m  >,   \
        BindFileToClassConstructor< TryMeCurveXccyBasis,    eur_source_xccy  >,   \
        BindFileToClassConstructor< TryMeCurveOis,	        gbp_source_ois >,	\
        BindFileToClassConstructor< TryMeCurveStd,          gbp_source_6m >,	\
        BindFileToClassConstructor< TryMeCurveTenorBasis,   gbp_source_3m  >,   \
        BindFileToClassConstructor< TryMeCurveXccyBasis,    gbp_source_xccy  >> {};


	//
    // The following macros also instantiate yield curves
    // Compare the macros above, these macros are not tied to a particular test name.
    // --------------------------------------------------------------------------------------------------------------
    //
	
	// Macro that sets up a single OIS curve
    // LWO Single Curves Populate Curve Results Objects that Conflict with Other Curve Calibration Methods, so we clear the curve results container here
    #define SET_UP_OIS_CURVE(ois) \
	etrading::CurveResultsContainer::getInstance().deleteAllCurveResults(); \
    setUpOISCurve(ois);

	// Macro that sets up a single STD curve and all curves it depends on
    // LWO Single Curves Populate Curve Results Objects that Conflict with Other Curve Calibration Methods, so we clear the curve results container here
    #define SET_UP_STD_CURVE(ois, std) \
	etrading::CurveResultsContainer::getInstance().deleteAllCurveResults(); \
    setUpOISCurve(ois);	\
	setUpSTDCurve(std);

	// Macro that sets up a single tenor basis curve and all curves it depends on.
	// It assumes the 'against' leg forecast curve is a STD swap curve
    // LWO Single Curves Populate Curve Results Objects that Conflict with Other Curve Calibration Methods, so we clear the curve results container here
    #define SET_UP_TENOR_BASIS_CURVE_1(ois_against, std_against, tenorbasis) \
    etrading::CurveResultsContainer::getInstance().deleteAllCurveResults(); \
    setUpOISCurve(ois_against);	\
    setUpSTDCurve(std_against); \
	setUpTenorBasisCurve(tenorbasis);

	// Macro that sets up a single tenor basis curve and all curves it depends on.
	// It assumes the 'against' leg forecast curve is a tenor basis curve
    // LWO Single Curves Populate Curve Results Objects that Conflict with Other Curve Calibration Methods, so we clear the curve results container here
    #define SET_UP_TENOR_BASIS_CURVE_2(ois_against, tenorbasis_against, tenorbasis) \
	etrading::CurveResultsContainer::getInstance().deleteAllCurveResults(); \
    setUpOISCurve(ois_against);	\
	setUpTenorBasisCurve(tenorbasis_against); \
	setUpTenorBasisCurve(tenorbasis);

	

}
