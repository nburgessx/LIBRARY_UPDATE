#include "ModelTools.h"

#include "LAPriceDataCalendar.h"
#include "LAPriceDataSlidingRule.h"
#include "LADateScheduleHelpers.h"
#include "LAMathDefine.h"
#include "LADataBasics.h"
#include "LAPriceCMSTools.h"
#include "LAMathSwaptionVolUtility.h"
#include "LAMathParameterUtility.h"

#include "InitializeAQETrading.h"
#include "Dependency.h"
#include "CurveOis.h"
#include "CurveStd.h"
#include "ParameterValidation.h"

namespace google_test
{
// Find AsOfDate stored in convention object
LADate ModelUtility::AsOfDate(const LAString& convID)
{
    LADataInstance* dataInstance = etrading::getDataInstance();
    LAObject conventions = dataInstance->getObjectPool().getObject(convID, ENCHKTYPE_ISDEFINED ).get();
    return dynamic_cast<const LADataDate& >(conventions.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL).get()).get();
}

/* Calendar shift of a date by a term with default conventions. This is used for simple estimates of shifted
   dates, not intended to be used for accurate pricing following well defined conventions. */
LADate ModelUtility::ShiftDate(LADate valDate, const LAString& term)
{
    LAString calendar = LAString("TKB:LNB");
    LAPriceDataCalendar cdr; cdr.convertFromString(calendar);
    LAString slidingRule = LAString("NO_CHANGE");
    LAPriceDataSlidingRule sdr; sdr.convertFromString(slidingRule);
    return etrading::CalendarAdvance(valDate, term, sdr, cdr);
}

// Wrapper to set 1 forecast and 1 discount curve in object pool
void ModelUtility::SetCurves(const LAString& directory, const LAString& fileNameForecast, const LAString& fileNameDiscount)
{
    LAString forecastFile = directory + fileNameForecast + LAString(".csv");
    LAString discountFile = directory + fileNameDiscount + LAString(".csv");
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
void ModelUtility::SetParameters(const LAString& directory, const LAString& paramFileName, const LAString& currency,
                                 const LAStringVector& paramNames, LAStringVector& paramIDs, LADate& asOfDate)
{
    LADataInstance* dataInstance = etrading::InitializeAQETrading::instance().dataInstance();

    // Set file object
    LAString inputFile = directory + paramFileName + LAString(".csv");
    etrading::ReadDataFile::Load inputFileObj = etrading::ReadDataFile::Load(inputFile);

    // Create conventions
    LAString convID = inputFileObj["ConventionID"]();
    LAStringMatrix convMatrix = inputFileObj["Conventions"];
    LAMathSwaptionVolUtility::setUpConvention(dataInstance, convID, convMatrix);

    // Set-up parameter matrices
    size_t paramSize = paramNames.size();
    paramIDs = LAStringVector(paramSize);
    for (size_t i = 0; i < paramSize; i++)
    {
        LAString paramName = paramNames[i];
        LAStringMatrix paramMatrix = inputFileObj[paramName];
        LAString paramID = LAPriceCMSObject::MatrixID("_" + paramName + "_", currency);
        LAMathSwaptionVolUtility::setUpSABRGrid(dataInstance, paramID, convID, paramMatrix);
        paramIDs[i] = paramID;
    }

    // AsOfDate
    asOfDate = google_test::ModelUtility::AsOfDate(convID);
}

/* Loads parameter matrices in object pool. The logic is very similar to SetParameters(). The difference is that
   the x-axis is not a tenor (more generally, it is a non-interpolat-able string). This is used for
   example for spread quotes which are indexed on the x-axis by tenor pairs of the form 30Y/2Y. */
void ModelUtility::SetNonInterpolatedParameters(const LAString& directory, const LAString& paramFileName, const LAString& currency,
                                                const LAStringVector& paramNames, LAStringVector& paramIDs, LADate& asOfDate)
{
    LADataInstance* dataInstance = etrading::InitializeAQETrading::instance().dataInstance();

    // Set file object
    LAString inputFile = directory + paramFileName + LAString(".csv");
    etrading::ReadDataFile::Load inputFileObj = etrading::ReadDataFile::Load(inputFile);

    // Create conventions
    LAString convID = inputFileObj["ConventionID"]();
    LAStringMatrix convMatrix = inputFileObj["Conventions"];
    LAMathSwaptionVolUtility::setUpConvention(dataInstance, convID, convMatrix);

    // Set-up parameter matrices
    size_t paramSize = paramNames.size();
    paramIDs = LAStringVector(paramSize);
    for (size_t i = 0; i < paramSize; i++)
    {
        LAString paramName = paramNames[i];
        LAStringMatrix paramMatrix = inputFileObj[paramName];
        LAString paramID = LAPriceCMSObject::MatrixID("_" + paramName + "_", currency);
        LAMathParameterObject::SetParameterMatrix(dataInstance, paramID, convID, paramMatrix);
        paramIDs[i] = paramID;
    }

    // AsOfDate
    asOfDate = google_test::ModelUtility::AsOfDate(convID);
}
}
