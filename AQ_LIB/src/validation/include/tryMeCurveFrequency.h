#pragma once

#include "LACoreTemplateType.h"

namespace validation_api
{
    /* @brief			Returns the curveFrequency i.e. the float index frequency
    *  @param [in]		curveCollection		The curve collection ID
    *  @param [in]		curveIndex			Equivalent names of the curve being built
    *  @param [out]     curveFrequency      The curve frequency i.e the float index frequency
    */
    const LAString tryMeCurveFrequency( const LAString& curveCollection,
                                        const LAString& curveIndex );

}

