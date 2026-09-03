#include "CurveFwdFxConst.h"
#include "tryMirSetUpFwdFXConstantCurve.h"

namespace google_test
{
    CurveFwdFxConst::CurveFwdFxConst( const AQLString& inputFile )
        : CurveAccessors( inputFile )
    {
        try
        {
            validation::tryMirSetUpFwdFXConstantCurve(
                getDataInstance(),
                curveID_,
                marketName_,
                inputFile_["generalProps"],
                inputFile_["fwdFXConstantConv"],
                inputFile_["curveNames"] );
        }
        catch( const AQLCoreError& m )
        {
            std::cout <<  m.getMsg();
        }
        catch( const std::exception& e )
        {
            std::cout << e.what();
        }
    };
}
