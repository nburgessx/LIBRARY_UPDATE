/*
 * @brief			validation interface for mirGetDateFromTerm method(s)
 * @Created:		17 March 2016
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */


#pragma once

#include "LACoreTemplateType.h"
#include "LADataInstance.h"

namespace validation_api
{

    /* @brief			swig interface for mirGetDateFromTerm
    *  @param [in]		fromDate			The curve collection ID
    *  @param [in]		termY				Year fraction
    *  @param [in]		dayCount			Day count convention
    *  @param [in]		includeLast			True if include the last day, not used in the core function
    *  @return			The end date derived from the FromDate and given year fraction
    */
    LAString tryMirGetDateFromTerm( const LADate& fromDate,
                                    double termY,
                                    const LAString& dayCount,
                                    bool includeLast );


}

