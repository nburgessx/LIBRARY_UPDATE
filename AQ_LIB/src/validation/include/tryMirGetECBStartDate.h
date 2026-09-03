#pragma once

#include "LADate.h"

namespace validation_api
{

    /* @brief			validation interface for the mirGetECBStartDate method
    *  @param [in]		ecbDate ECB(European Central Bank) date
    *  @return			The start date of an ECB Swap base on the ecb date
    */
    LADate tryMirGetECBStartDate( const LADate& ecbDate );

}
