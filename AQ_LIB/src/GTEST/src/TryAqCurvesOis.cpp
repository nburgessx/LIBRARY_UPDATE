#include "TryAqCurvesOis.h"

#include "tryAqCurveCalibrateOIS.h"

namespace google_test
{
    TryAqCurvesOis::TryAqCurvesOis( const AQLString& inputFile )
        : TryAqCurvesAccessors( inputFile )
    {
        if ( fileLoaded_ )
        {
            try
            {
                validation::tryAqCurveCalibrateOIS( inputFile_["curveCollection"],
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
	void setUpAqOISCurve(const AQLString& inputFile)
	{
		if ( inputFile.size() != 0 )
        {
            etrading::ReadDataFile::Load inputFileObj = etrading::ReadDataFile::Load( inputFile );

            try
            {
                validation::tryAqCurveCalibrateOIS( inputFileObj["curveCollection"],
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
