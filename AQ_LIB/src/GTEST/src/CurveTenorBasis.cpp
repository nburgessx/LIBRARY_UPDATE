#include "CurveTenorBasis.h"
#include "AQLUpdateStaticDataManager.h"
#include "CurveValidation.h"          // etrading::checkIfCurveExists
#include "CurveResultsContainer.h"    // AQ_CLEAR_CURVE_RESULTS_CACHE
#include "YieldCurveUtil.h"
#include "InitializeETrading.h"

namespace google_test
{
    // Argument order below is that of etrading::AQLUpdateStaticDataManager::setUpBasisCurve;
    // the validation wrapper that previously sat here reordered the fixture inputs into it.
    CurveTenorBasis::CurveTenorBasis( const AQLString& inputFile )
        : CurveAccessors( inputFile )
    {
        if ( fileLoaded_ )
        {
            try
            {
                AQ_CLEAR_CURVE_RESULTS_CACHE
                etrading::AQLUpdateStaticDataManager::setUpBasisCurve(
                    getDataInstance(),
                    curveID_,
                    marketName_,
                    inputFile_["basisRates"],
                    inputFile_["basisConv"],
                    inputFile_["fwdFXs"],
                    inputFile_["fwdConv"],
                    inputFile_["spotFXs"],
                    inputFile_["generalProps"],
                    AQLStringMatrix( 0 ),          // moneyConv (unused for tenor basis)
                    inputFile_["curveNames"],
                    inputFile_["fraConv"],
                    inputFile_["fraRates"] );
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
	*  @brief			Set up tenor basis curve
	*  @param [in]		inputFile	File representation of the curve
    */
	void setUpTenorBasisCurve(const AQLString& inputFile)
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
                etrading::AQLUpdateStaticDataManager::setUpBasisCurve(
                    etrading::InitializeETrading::instance().dataInstance(),
                    curveID,
                    marketName,
                    inputFileObj["basisRates"],
                    inputFileObj["basisConv"],
                    inputFileObj["fwdFXs"],
                    inputFileObj["fwdConv"],
                    inputFileObj["spotFXs"],
                    inputFileObj["generalProps"],
                    AQLStringMatrix( 0 ),          // moneyConv (unused for tenor basis)
                    inputFileObj["curveNames"],
                    inputFileObj["fraConv"],
                    inputFileObj["fraRates"] );
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
