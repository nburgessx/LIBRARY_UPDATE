#include "CurveOis.h"

#include "tryMirSetUpOISCurve.h"
#include "YieldCurveUtil.h"
#include "InitializeAQETrading.h"

namespace google_test
{
    CurveOis::CurveOis( const LAString& inputFile )
        : CurveAccessors( inputFile )
    {
        if ( fileLoaded_ )
        {
            try
            {
                validation::tryMirSetUpOISCurve( getDataInstance(),
                                                     curveID_,
                                                     marketName_,
                                                     inputFile_["generalProps"],
                                                     inputFile_["oisRates"],
                                                     inputFile_["oisConv"],
                                                     inputFile_["curveNames"],
                                                     inputFile_["oisHistRates"],
                                                     inputFile_["loBasisRates"],
                                                     inputFile_["loBasisConv"],
                                                     inputFile_.getOptional( "swapRates" ),
                                                     inputFile_.getOptional( "swapConv" ) );
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
    };

	/* 
	*  @brief			Set up OIS curve
	*  @param [in]		inputFile	File representation of the curve
    */
	void setUpOISCurve(const LAString& inputFile)
	{
		if ( inputFile.size() != 0 )
        {
            etrading::ReadDataFile::Load inputFileObj = etrading::ReadDataFile::Load( inputFile );
            LAString curveID = etrading::getCurveID( inputFileObj );
            LAString marketName = etrading::getMarketName( inputFileObj );
            LAStringVector curveNames = etrading::getCurveNames( inputFileObj );

			try
            {
                validation::tryMirSetUpOISCurve( etrading::InitializeAQETrading::instance().dataInstance(),
                                                     curveID,
                                                     marketName,
                                                     inputFileObj["generalProps"],
                                                     inputFileObj["oisRates"],
                                                     inputFileObj["oisConv"],
                                                     inputFileObj["curveNames"],
                                                     inputFileObj["oisHistRates"],
                                                     inputFileObj["loBasisRates"],
                                                     inputFileObj["loBasisConv"],
                                                     inputFileObj.getOptional( "swapRates" ),
                                                     inputFileObj.getOptional( "swapConv" ) );
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
