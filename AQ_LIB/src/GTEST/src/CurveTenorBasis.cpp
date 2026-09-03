#include "CurveTenorBasis.h"
#include "YieldCurveUtil.h"
#include "InitializeETrading.h"
#include "tryMirSetUpBasisSwapCurve.h"

namespace google_test
{
    CurveTenorBasis::CurveTenorBasis( const AQLString& inputFile )
        : CurveAccessors( inputFile )
    {
        if ( fileLoaded_ )
        {
            try
            {
                validation::tryMirSetUpBasisSwapCurve(
                    getDataInstance(),
                    curveID_,
                    marketName_,
                    inputFile_["generalProps"],
                    inputFile_["basisConv"],
                    inputFile_["basisRates"],
                    inputFile_["curveNames"],
                    inputFile_["fwdConv"],
                    inputFile_["fwdFXs"],
                    inputFile_["spotFXs"],
					inputFile_["fraConv"],
					inputFile_["fraRates"]);
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
				validation::tryMirSetUpBasisSwapCurve(
					etrading::InitializeETrading::instance().dataInstance(),
					curveID,
					marketName,
					inputFileObj["generalProps"],
					inputFileObj["basisConv"],
					inputFileObj["basisRates"],
					inputFileObj["curveNames"],
					inputFileObj["fwdConv"],
					inputFileObj["fwdFXs"],
					inputFileObj["spotFXs"],
					inputFileObj["fraConv"],
					inputFileObj["fraRates"]);
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


