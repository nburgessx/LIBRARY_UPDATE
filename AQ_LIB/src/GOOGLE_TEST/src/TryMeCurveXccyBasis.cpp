#include "TryMeCurveXccyBasis.h"
#include "tryMeCurveCalibrateBasis.h"

namespace google_test
{
    TryMeCurveXccyBasis::TryMeCurveXccyBasis( const AQLString& inputFile )
        : TryMeCurveAccessors( inputFile )
    {
        if ( fileLoaded_ )
        {
            try
            {
                validation::tryMeCurveCalibrateBasis(
                    inputFile_["curveCollection"],
                    inputFile_["staticDataTable"],
                    inputFile_["curveIndex"],
                    inputFile_["curveConv"],
                    inputFile_["basisConv"],
                    inputFile_["basisRates"],
                    inputFile_["fxFwdConv"],
                    inputFile_["fxFwdRates"],
                    inputFile_["spotFxRates"],
					inputFile_.getOptional("fraConv"),
					inputFile_.getOptional("fraRates"));
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

	/* 
	*  @brief			Set up xccy basis curve
	*  @param [in]		inputFile	File representation of the curve
	*/
	void setUpMeXccyBasisCurve(const AQLString& inputFile)
	{
		if ( inputFile.size() != 0 )
        {
            etrading::ReadDataFile::Load inputFileObj = etrading::ReadDataFile::Load( inputFile );
			 
			try
            {
                validation::tryMeCurveCalibrateBasis(
                    inputFileObj["curveCollection"],
                    inputFileObj["staticDataTable"],
                    inputFileObj["curveIndex"],
                    inputFileObj["curveConv"],
                    inputFileObj["basisConv"],
                    inputFileObj["basisRates"],
                    inputFileObj["fxFwdConv"],
                    inputFileObj["fxFwdRates"],
                    inputFileObj["spotFxRates"],
					inputFileObj.getOptional("fraConv"),
					inputFileObj.getOptional("fraRates"));
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
