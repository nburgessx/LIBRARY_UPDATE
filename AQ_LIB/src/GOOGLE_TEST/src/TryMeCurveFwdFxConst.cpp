#include "TryMeCurveFwdFxConst.h"
#include "tryMeCurveCalibrateFXForwards.h"

namespace google_test
{
    TryMeCurveFwdFxConst::TryMeCurveFwdFxConst( const AQLString& inputFile )
        : TryMeCurveAccessors( inputFile )
    {
        try
        {
            validation::tryMeCurveCalibrateFXForwards(
                inputFile_["curveCollection"],
                inputFile_["staticDataTable"],
                inputFile_["curveIndex"],
                inputFile_["curveConv"],
                inputFile_["fxFwdConv"] );
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

	/* 
	*  @brief			Set up fwdfxconst curve
	*  @param [in]		inputFile	File representation of the curve
	*/
	void setUpMeFwdFxConstCurveCurve( const AQLString& inputFile )
	{
		if ( inputFile.size() != 0 )
        {
            etrading::ReadDataFile::Load inputFileObj = etrading::ReadDataFile::Load( inputFile );
			 
			try
			{
				validation::tryMeCurveCalibrateFXForwards(
					inputFileObj["curveCollection"],
					inputFileObj["staticDataTable"],
					inputFileObj["curveIndex"],
					inputFileObj["curveConv"],
					inputFileObj["fxFwdConv"] );
			}
			catch( const AQLCoreError& m )
			{
				std::cout <<  m.getMsg();
			}
			catch( const std::exception& e )
			{
				std::cout << e.what();
			}
		}
	}
}
