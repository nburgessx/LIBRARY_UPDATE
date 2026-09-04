#include "CurveFwdFxConst.h"
#include "AQLUpdateStaticDataManager.h"
#include "CurveValidation.h"          // etrading::checkIfCurveExists
#include "CurveResultsContainer.h"    // AQ_CLEAR_CURVE_RESULTS_CACHE

namespace google_test
{
    // Argument order below is that of etrading::AQLUpdateStaticDataManager::setUpFwdFXConstantCurve
    // (fwdFXConstantConv before generalProps); the validation wrapper that previously sat here
    // took them the other way round and swapped internally.
    CurveFwdFxConst::CurveFwdFxConst( const AQLString& inputFile )
        : CurveAccessors( inputFile )
    {
        try
        {
            AQ_CLEAR_CURVE_RESULTS_CACHE
            etrading::AQLUpdateStaticDataManager::setUpFwdFXConstantCurve(
                getDataInstance(),
                curveID_,
                marketName_,
                inputFile_["fwdFXConstantConv"],
                inputFile_["generalProps"],
                inputFile_["curveNames"] );
            etrading::checkIfCurveExists( getDataInstance(), curveID_ );
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
