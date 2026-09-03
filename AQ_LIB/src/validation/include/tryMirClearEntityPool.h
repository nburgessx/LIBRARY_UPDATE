/*
 * @brief			validation interface for the mirClearEntityPool function
 * @Created:		27 April 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include "LACoreTemplateType.h"

namespace validation_api
{
    /* @brief			validation interface for the mirClearEntityPool function, to clear the object pool
     *  @return	A notification string
     */
    LAString tryMirClearEntityPool();

}