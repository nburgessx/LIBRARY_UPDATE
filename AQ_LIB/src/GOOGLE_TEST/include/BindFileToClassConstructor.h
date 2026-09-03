#pragma once

namespace google_test
{
    //
    // CLASS
    //   google_test::BindFileToClassConstructor
    //
    // PURPOSE
    //   Encode AlgoQuantLib classConstructuctorName(s) input file name in a class
    //   for use in compile-time dependency graphs.
    //
    // SYNPOSIS
    //   #include "BindFileToClassConstructor.h"
    //   #include "CurveOIS.h"
    //
    //   using google_test::BindFileToClassConstructor;
    //   using google_test::CurveOIS;
    //
    //   extern const char EURYC_OIS[] = "EURYC_OIS_tryMirSetUpOISCurve_inputs";	// "extern" to make it suitable as a template parameter
    //
    //   typedef BindFileToClassConstructor<CurveOIS, EURYC_OIS> SourceEURYC_OIS;
    //
    //
    // EXTRA COMMENT
    //   The BindFileToClassConstructor template is a helper template, which calls a given class constructor 'classConstructuctorName' passing
    //   it the inputFileName as an input. The routine is used to pass a yield curve constructor the input data file to be used
    //   for the yield curve inputs. This helps to simplify the curve loading process for google tests.
    //
    // EXAMPLE
    //   BindFileToClassConstructor<CurveOIS, EURYC_OIS>
    //   This will call the CurveOIS constructor and pass it the inputFileName as defined by the variable EURYC_OIS='C:\Temp\EurOisCurveInputs.csv' for example


    template<typename classConstructuctorName, const char* inputFileName>
    class BindFileToClassConstructor : public classConstructuctorName
    {
    public:
        BindFileToClassConstructor() : classConstructuctorName( inputFileName ) {}
    };
}
