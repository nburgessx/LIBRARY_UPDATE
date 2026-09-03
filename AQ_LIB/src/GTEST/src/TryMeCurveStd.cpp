#include "TryMeCurveStd.h"

#include "tryMeCurveCalibrateSwap.h"

namespace google_test
{
    TryMeCurveStd::TryMeCurveStd( const AQLString& inputFile )
        : TryMeCurveAccessors( inputFile )
    {
        if ( fileLoaded_ )
        {
            try
            {
                validation::tryMeCurveCalibrateSwap(
                    inputFile_["curveCollection"],
                    inputFile_["staticDataTable"],
                    inputFile_["curveIndex"],
                    inputFile_["curveConv"],
                    inputFile_["moneyMarketConv"],
                    inputFile_["liborConv"],
                    inputFile_["liborRates"],
                    inputFile_["swapConv"],
                    inputFile_["swapRates"],
                    inputFile_["fraConv"],
                    inputFile_["fra3mRates"],
                    inputFile_["fra6mRates"],
                    inputFile_["futureConv"],
                    inputFile_["futureRates"],
                    inputFile_["convexityAdjConv"],
                    inputFile_["convexityAdjRates"] );
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
    };

	/* 
	*  @brief			Set up STD swap curve
	*  @param [in]		inputFile	File representation of the curve
	*/
	void setUpMeSTDCurve(const AQLString& inputFile)
	{
		if ( inputFile.size() != 0 )
        {
            etrading::ReadDataFile::Load inputFileObj = etrading::ReadDataFile::Load( inputFile );

            try
            {
                validation::tryMeCurveCalibrateSwap(
                    inputFileObj["curveCollection"],
                    inputFileObj["staticDataTable"],
                    inputFileObj["curveIndex"],
                    inputFileObj["curveConv"],
                    inputFileObj["moneyMarketConv"],
                    inputFileObj["liborConv"],
                    inputFileObj["liborRates"],
                    inputFileObj["swapConv"],
                    inputFileObj["swapRates"],
                    inputFileObj["fraConv"],
                    inputFileObj["fra3mRates"],
                    inputFileObj["fra6mRates"],
                    inputFileObj["futureConv"],
                    inputFileObj["futureRates"],
                    inputFileObj["convexityAdjConv"],
                    inputFileObj["convexityAdjRates"] );
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
