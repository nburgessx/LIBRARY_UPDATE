#include "CurveOis.h"

#include "AQLUpdateStaticDataManager.h"
#include "CurveValidation.h"          // etrading::checkIfCurveExists
#include "CurveResultsContainer.h"    // AQ_CLEAR_CURVE_RESULTS_CACHE
#include "YieldCurveUtil.h"
#include "InitializeETrading.h"

namespace google_test
{
    CurveOis::CurveOis( const AQLString& inputFile )
        : CurveAccessors( inputFile )
    {
        if ( fileLoaded_ )
        {
            try
            {
                AQ_CLEAR_CURVE_RESULTS_CACHE
                etrading::AQLUpdateStaticDataManager::setUpOISCurve( getDataInstance(),
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
        }
    };

	/*
	*  @brief			Set up OIS curve
	*  @param [in]		inputFile	File representation of the curve
    */
	void setUpOISCurve(const AQLString& inputFile)
	{
		if ( inputFile.size() != 0 )
        {
            etrading::ReadDataFile::Load inputFileObj = etrading::ReadDataFile::Load( inputFile );
            AQLString curveID = etrading::getCurveID( inputFileObj );
            AQLString marketName = etrading::getMarketName( inputFileObj );
            AQLStringVector curveNames = etrading::getCurveNames( inputFileObj );

			try
            {
                AQ_CLEAR_CURVE_RESULTS_CACHE
                etrading::AQLUpdateStaticDataManager::setUpOISCurve( etrading::InitializeETrading::instance().dataInstance(),
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
                etrading::checkIfCurveExists( etrading::InitializeETrading::instance().dataInstance(), curveID );
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
