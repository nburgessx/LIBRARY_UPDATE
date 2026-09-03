#include "TryMeCurveTenorBasis.h"
#include "tryMeCurveCalibrateBasis.h"

namespace google_test
{
    TryMeCurveTenorBasis::TryMeCurveTenorBasis( const LAString& inputFile )
        : TryMeCurveAccessors( inputFile )
    {
        if ( fileLoaded_ )
        {
            try
            {
                validation_api::tryMeCurveCalibrateBasis(
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
            catch( const LACoreError& m )
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
	void setUpMeTenorBasisCurve(const LAString& inputFile)
	{
		if ( inputFile.size() != 0 )
        {
            etrading::ReadDataFile::Load inputFileObj = etrading::ReadDataFile::Load( inputFile );
			 
			try
            {
                validation_api::tryMeCurveCalibrateBasis(
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
            catch( const LACoreError& m )
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


