#include "TryMeCurveOis.h"

#include "tryMeCurveCalibrateOIS.h"

namespace google_test
{
    TryMeCurveOis::TryMeCurveOis( const AQLString& inputFile )
        : TryMeCurveAccessors( inputFile )
    {
        if ( fileLoaded_ )
        {
            try
            {
                validation::tryMeCurveCalibrateOIS( inputFile_["curveCollection"],
                                                        inputFile_["staticDataTable"],
                                                        inputFile_["curveIndex"],
                                                        inputFile_["curveConv"],
                                                        inputFile_["oisConv"],
                                                        inputFile_["oisRates"],
                                                        inputFile_["oisHistoricalRates"],
                                                        inputFile_["liborOisBasisConv"],
                                                        inputFile_["liborOisBasisRates"],
                                                        inputFile_.getOptional( "swapConv" ),
                                                        inputFile_.getOptional( "swapRates" ) );
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
	*  @brief			Set up OIS curve
	*  @param [in]		inputFile	File representation of the curve
	*/
	void setUpMeOISCurve(const AQLString& inputFile)
	{
		if ( inputFile.size() != 0 )
        {
            etrading::ReadDataFile::Load inputFileObj = etrading::ReadDataFile::Load( inputFile );

            try
            {
                validation::tryMeCurveCalibrateOIS( inputFileObj["curveCollection"],
                                                        inputFileObj["staticDataTable"],
                                                        inputFileObj["curveIndex"],
                                                        inputFileObj["curveConv"],
                                                        inputFileObj["oisConv"],
                                                        inputFileObj["oisRates"],
                                                        inputFileObj["oisHistoricalRates"],
                                                        inputFileObj["liborOisBasisConv"],
                                                        inputFileObj["liborOisBasisRates"],
                                                        inputFileObj.getOptional( "swapConv" ),
                                                        inputFileObj.getOptional( "swapRates" ) );
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
