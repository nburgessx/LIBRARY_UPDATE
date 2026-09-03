#pragma once

#include "AQLDate.h"

namespace validation
{

    /* @brief			validation interface for the mirGetECBStartDate method
    *  @param [in]		ecbDate ECB(European Central Bank) date
    *  @return			The start date of an ECB Swap base on the ecb date
    */
    AQLDate tryMirGetECBStartDate( const AQLDate& ecbDate );

}
