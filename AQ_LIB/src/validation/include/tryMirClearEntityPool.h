#pragma once

#include "AQLCoreTemplateType.h"

namespace validation
{
    /* @brief			validation interface for the mirClearEntityPool function, to clear the object pool
     *  @return	A notification string
     */
    AQLString tryMirClearEntityPool();

}