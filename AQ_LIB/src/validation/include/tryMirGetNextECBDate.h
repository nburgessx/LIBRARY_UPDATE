#pragma once

#include "LADate.h"

namespace validation
{

    /* @brief			validation interface for the mirGetNextECBDate method
    *  @param [in]		baseDate		Base date
    *  @param [in]		strictlyAfter	True if strictly after the base date
    *  @return			Date of the next ECB (European Central Bank) meeting on/after the base date
    */
    LADate tryMirGetNextECBDate( const LADate& baseDate,
                                 bool strictlyAfter = true );


}
