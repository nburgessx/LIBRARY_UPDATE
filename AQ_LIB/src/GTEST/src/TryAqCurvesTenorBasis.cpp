#include "TryAqCurvesTenorBasis.h"
#include "tryAqCurvesCalibrateBasis.h"

namespace google_test
{
    TryAqCurvesTenorBasis::TryAqCurvesTenorBasis( const AQLString& inputFile )
        : TryAqCurvesAccessors( inputFile )
    {
        if ( fileLoaded_ )
        {
            try
            {
                validation::tryAqCurvesCalibrateBasis(
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
					inputFile_.getOptional("fraRates"),
                    inputFile_.getOptional("liborConv"),
                    inputFile_.getOptional("liborRates") );
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
	*  @brief			Set up tenor basis curve
	*  @param [in]		inputFile	File representation of the curve
	*/
	void setUpAqTenorBasisCurve(const AQLString& inputFile)
	{
		if ( inputFile.size() != 0 )
        {
            etrading::ReadDataFile::Load inputFileObj = etrading::ReadDataFile::Load( inputFile );
			 
			try
            {
                validation::tryAqCurvesCalibrateBasis(
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
					inputFileObj.getOptional("fraRates"),
                    inputFileObj.getOptional("liborConv"),
                    inputFileObj.getOptional("liborRates") );
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


