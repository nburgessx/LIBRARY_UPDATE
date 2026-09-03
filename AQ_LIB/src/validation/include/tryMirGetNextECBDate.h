#pragma once

#include "AQLDate.h"

namespace validation
{

    /* @brief			validation interface for the mirGetNextECBDate method
    *  @param [in]		baseDate		Base date
    *  @param [in]		strictlyAfter	True if strictly after the base date
    *  @return			Date of the next ECB (European Central Bank) meeting on/after the base date
    */
    AQLDate tryMirGetNextECBDate( const AQLDate& baseDate,
                                 bool strictlyAfter = true );


}
