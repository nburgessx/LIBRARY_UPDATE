#pragma once

#include "LADate.h"
#include "LACoreTemplateType.h"
#include <gTest/gTest.h>
#include "InitializeMLibGoogleTest.h"

#define MLIB_CURVE_DIR           "Vanilla/Models/"
#define MLIB_SABR_DIR            "Vanilla/Models/SABR/"
#define MLIB_TOTEM_DIR           "Vanilla/Models/Totem/"
#define MLIB_COPULA_DIR          "Vanilla/Models/Copula/"

namespace google_test
{
    class ModelsBasics : public testing::Test, public virtual google_test::InitializeMLibGoogleTest {};

    class ModelUtility
    {
    public:
        // Find AsOfDate stored in convention object
        static LADate AsOfDate(const LAString& convID);

        /* Calendar shift of a date by a term with default conventions. This is used for simple estimates of shifted
           dates, not intended to be used for accurate pricing following well defined conventions. */
        static LADate ShiftDate(LADate valDate, const LAString& term);

        // Wrapper to set 1 forecast and 1 discount curve in object pool
        static void SetCurves(const LAString& directory, const LAString& fileNameForecast,
                              const LAString& fileNameDiscount);

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
        static void SetParameters(const LAString& directory, const LAString& paramFileName, const LAString& currency,
                                  const LAStringVector& paramNames, LAStringVector& paramIDs, LADate& asOfDate);

        /* Loads parameter matrices in object pool. The logic is very similar to SetParameters(). The difference is that
           the x-axis is not a tenor (more generally, it is a non-interpolat-able string). This is used for
           example for spread quotes which are indexed on the x-axis by tenor pairs of the form 30Y/2Y. */
        static void SetNonInterpolatedParameters(const LAString& directory, const LAString& paramFileName, const LAString& currency,
                                                 const LAStringVector& paramNames, LAStringVector& paramIDs, LADate& asOfDate);
    };
}
