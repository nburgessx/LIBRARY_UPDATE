#include "CurveStd.h"
#include "AQLUpdateStaticDataManager.h"
#include "CurveValidation.h"          // etrading::checkIfCurveExists
#include "CurveResultsContainer.h"    // AQ_CLEAR_CURVE_RESULTS_CACHE
#include "YieldCurveUtil.h"
#include "InitializeETrading.h"

namespace google_test
{
    CurveStd::CurveStd( const AQLString& inputFile )
        : CurveAccessors( inputFile )
    {
        if ( fileLoaded_ )
        {
            try
            {
                AQ_CLEAR_CURVE_RESULTS_CACHE
                etrading::AQLUpdateStaticDataManager::setUpSwapCurve(
                    getDataInstance(),
                    curveID_,
                    marketName_,
                    inputFile_.getOptional( "generateProp", inputFile_.getOptional( "generalProps", AQLStringMatrix() ) ), // Some files have generateProp and others generalProps
                    inputFile_["moneyConv"],
                    inputFile_["liborRates"],
                    inputFile_["liborConv"],
                    inputFile_["swapRates"],
                    inputFile_["swapConv"],
                    inputFile_["fra3mRates"],
                    inputFile_["fra6mRates"],
                    inputFile_["fraConv"],
                    inputFile_["futureRates"],
                    inputFile_["futureConv"],
                    inputFile_["adjustSwapConv"],
                    inputFile_["adjustSwapRates"],
                    inputFile_["curveNames"],
                    inputFile_["curveName_DF2"] );
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
    }

	/* 
	*  @brief			Set up STD swap curve
	*  @param [in]		inputFile	File representation of the curve
    */
	void setUpSTDCurve(const AQLString& stdInputFile)
	{
		if ( stdInputFile.size() != 0 )
        {
            etrading::ReadDataFile::Load inputFileObj = etrading::ReadDataFile::Load( stdInputFile );
            AQLString curveID = etrading::getCurveID( inputFileObj );
            AQLString marketName = etrading::getMarketName( inputFileObj );
            AQLStringVector curveNames = etrading::getCurveNames( inputFileObj );

			try
            {
                AQ_CLEAR_CURVE_RESULTS_CACHE
                etrading::AQLUpdateStaticDataManager::setUpSwapCurve(
                    etrading::InitializeETrading::instance().dataInstance(),
                    curveID,
                    marketName,
                    inputFileObj.getOptional( "generateProp", inputFileObj.getOptional( "generalProps", AQLStringMatrix() ) ), // Some files have generateProp and others generalProps
                    inputFileObj["moneyConv"],
                    inputFileObj["liborRates"],
                    inputFileObj["liborConv"],
                    inputFileObj["swapRates"],
                    inputFileObj["swapConv"],
                    inputFileObj["fra3mRates"],
                    inputFileObj["fra6mRates"],
                    inputFileObj["fraConv"],
                    inputFileObj["futureRates"],
                    inputFileObj["futureConv"],
                    inputFileObj["adjustSwapConv"],
                    inputFileObj["adjustSwapRates"],
                    inputFileObj["curveNames"],
                    inputFileObj["curveName_DF2"] );
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
