#include "ModelTools.h"

#include "AQLPriceDataCalendar.h"
#include "AQLPriceDataSlidingRule.h"
#include "LADateScheduleHelpers.h"
#include "AQLMathDefine.h"
#include "AQLDataBasics.h"
#include "AQLPriceCMSTools.h"
#include "AQLMathSwaptionVolUtility.h"
#include "AQLMathParameterUtility.h"

#include "InitializeETrading.h"
#include "Dependency.h"
#include "CurveOis.h"
#include "CurveStd.h"
#include "ParameterValidation.h"

namespace google_test
{
// Find AsOfDate stored in convention object
AQLDate ModelUtility::AsOfDate(const AQLString& convID)
{
    AQLDataInstance* dataInstance = etrading::getDataInstance();
    AQLObject conventions = dataInstance->getObjectPool().getObject(convID, ENCHKTYPE_ISDEFINED ).get();
    return dynamic_cast<const AQLDataDate& >(conventions.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL).get()).get();
}

/* Calendar shift of a date by a term with default conventions. This is used for simple estimates of shifted
   dates, not intended to be used for accurate pricing following well defined conventions. */
AQLDate ModelUtility::ShiftDate(AQLDate valDate, const AQLString& term)
{
    AQLString calendar = AQLString("TKB:LNB");
    AQLPriceDataCalendar cdr; cdr.convertFromString(calendar);
    AQLString slidingRule = AQLString("NO_CHANGE");
    AQLPriceDataSlidingRule sdr; sdr.convertFromString(slidingRule);
    return etrading::CalendarAdvance(valDate, term, sdr, cdr);
}

// Wrapper to set 1 forecast and 1 discount curve in object pool
void ModelUtility::SetCurves(const AQLString& directory, const AQLString& fileNameForecast, const AQLString& fileNameDiscount)
{
    AQLString forecastFile = directory + fileNameForecast + AQLString(".csv");
    AQLString discountFile = directory + fileNameDiscount + AQLString(".csv");
    SET_UP_STD_CURVE(discountFile, forecastFile);
}

/* Loads parameter matrices in object pool. Originally this was used only for SABR but it has been extended
   and is also used for Benaim's tail parameters now. In fact it can be used for a generic number of parameters
   as collected in paramNames. Due to technical details, the effective parameter IDs to recover the parameters
   from memory later are not identical to those passed as inputs, which is why we return paramIDs, the effective
   parameters to call when pricing. See SABR test for an example of usage.
   The asOfDate of definition of the parameters is also retrieved, so that one may check consistency between
   asOfDate of different parameter sets.
   This particular function should be used for parameters along the directions of x-axis, which is tenor,
   and the y-axis which is expiry. It allows interpolation along the tenor. This is the case of SABR and Benaim.

   When a parameter has expiries is indexed on the x-axis by a non-interpolat-able quantity, this function
   cannot be used. Use SetNonInterpolatedParameters() instead. */
void ModelUtility::SetParameters(const AQLString& directory, const AQLString& paramFileName, const AQLString& currency,
                                 const AQLStringVector& paramNames, AQLStringVector& paramIDs, AQLDate& asOfDate)
{
    AQLDataInstance* dataInstance = etrading::InitializeETrading::instance().dataInstance();

    // Set file object
    AQLString inputFile = directory + paramFileName + AQLString(".csv");
    etrading::ReadDataFile::Load inputFileObj = etrading::ReadDataFile::Load(inputFile);

    // Create conventions
    AQLString convID = inputFileObj["ConventionID"]();
    AQLStringMatrix convMatrix = inputFileObj["Conventions"];
    AQLMathSwaptionVolUtility::setUpConvention(dataInstance, convID, convMatrix);

    // Set-up parameter matrices
    size_t paramSize = paramNames.size();
    paramIDs = AQLStringVector(paramSize);
    for (size_t i = 0; i < paramSize; i++)
    {
        AQLString paramName = paramNames[i];
        AQLStringMatrix paramMatrix = inputFileObj[paramName];
        AQLString paramID = AQLPriceCMSObject::MatrixID("_" + paramName + "_", currency);
        AQLMathSwaptionVolUtility::setUpSABRGrid(dataInstance, paramID, convID, paramMatrix);
        paramIDs[i] = paramID;
    }

    // AsOfDate
    asOfDate = google_test::ModelUtility::AsOfDate(convID);
}

/* Loads parameter matrices in object pool. The logic is very similar to SetParameters(). The difference is that
   the x-axis is not a tenor (more generally, it is a non-interpolat-able string). This is used for
   example for spread quotes which are indexed on the x-axis by tenor pairs of the form 30Y/2Y. */
void ModelUtility::SetNonInterpolatedParameters(const AQLString& directory, const AQLString& paramFileName, const AQLString& currency,
                                                const AQLStringVector& paramNames, AQLStringVector& paramIDs, AQLDate& asOfDate)
{
    AQLDataInstance* dataInstance = etrading::InitializeETrading::instance().dataInstance();

    // Set file object
    AQLString inputFile = directory + paramFileName + AQLString(".csv");
    etrading::ReadDataFile::Load inputFileObj = etrading::ReadDataFile::Load(inputFile);

    // Create conventions
    AQLString convID = inputFileObj["ConventionID"]();
    AQLStringMatrix convMatrix = inputFileObj["Conventions"];
    AQLMathSwaptionVolUtility::setUpConvention(dataInstance, convID, convMatrix);

    // Set-up parameter matrices
    size_t paramSize = paramNames.size();
    paramIDs = AQLStringVector(paramSize);
    for (size_t i = 0; i < paramSize; i++)
    {
        AQLString paramName = paramNames[i];
        AQLStringMatrix paramMatrix = inputFileObj[paramName];
        AQLString paramID = AQLPriceCMSObject::MatrixID("_" + paramName + "_", currency);
        AQLMathParameterObject::SetParameterMatrix(dataInstance, paramID, convID, paramMatrix);
        paramIDs[i] = paramID;
    }

    // AsOfDate
    asOfDate = google_test::ModelUtility::AsOfDate(convID);
}
}
