#pragma once

#include "AQLCoreTemplateType.h"

namespace validation
{
    /* @brief			Returns the curveFrequency i.e. the float index frequency
    *  @param [in]		curveCollection		The curve collection ID
    *  @param [in]		curveIndex			Equivalent names of the curve being built
    *  @param [out]     curveFrequency      The curve frequency i.e the float index frequency
    */
    const AQLString tryAqCurveFrequency( const AQLString& curveCollection,
                                        const AQLString& curveIndex );

}

