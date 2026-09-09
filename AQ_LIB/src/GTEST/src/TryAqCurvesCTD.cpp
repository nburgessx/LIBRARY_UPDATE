#include "TryAqCurvesOis.h"

#include "tryAqCurveCalibrateCTD.h"

namespace google_test
{
	/* 
	*  @brief			Set up CTD curve
	*  @param [in]		inputFile	File representation of the curve
	*/
	void setUpAqCTDCurve(const AQLString& inputFile)
	{
		if ( inputFile.size() != 0 )
        {
            etrading::ReadDataFile::Load inputFileObj = etrading::ReadDataFile::Load( inputFile );

            try
            {
                validation::tryAqCurveCalibrateCTD( inputFileObj["curveCollection"],
                                                        inputFileObj["staticDataTable"],
                                                        inputFileObj["curveIndex"],
                                                        inputFileObj["curveConv"],
                                                        inputFileObj["collateralCurves"]);
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
