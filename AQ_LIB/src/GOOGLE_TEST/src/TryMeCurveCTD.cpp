/*
 * @brief			Calling calibration method of the cheapest-to-deliver curves
 * @Created:		08 AUG 2017
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#include "TryMeCurveOis.h"

#include "tryMeCurveCalibrateCTD.h"

namespace google_test
{
	/* 
	*  @brief			Set up CTD curve
	*  @param [in]		inputFile	File representation of the curve
	*/
	void setUpMeCTDCurve(const LAString& inputFile)
	{
		if ( inputFile.size() != 0 )
        {
            etrading::ReadDataFile::Load inputFileObj = etrading::ReadDataFile::Load( inputFile );

            try
            {
                validation_api::tryMeCurveCalibrateCTD( inputFileObj["curveCollection"],
                                                        inputFileObj["staticDataTable"],
                                                        inputFileObj["curveIndex"],
                                                        inputFileObj["curveConv"],
                                                        inputFileObj["collateralCurves"]);
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
