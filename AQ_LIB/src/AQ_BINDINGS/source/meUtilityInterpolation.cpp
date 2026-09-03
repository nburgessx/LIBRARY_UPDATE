/* 
 * @brief			Swig interface to Java for function meUtilityInterpolation
 * @Created:		03 June 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "TypeUtilities.h"
#include "LAString.h"

#include "meUtilityInterpolation.h"
#include "tryMeUtilityInterpolation.h"
#include "APISetUp.h"               // MLIB_API_START and MLIB_API_END Macros


/* @brief			swig interface for meUtilityInterpolation
*  @param [in]		xValues			X-axis values of the data
*  @param [in]		yValues			Y-axis values of the data
*  @param [in]		xPoint			X-axis point
*  @param [in]		interpolation	Interpolation method
*  @return			the Y-axis interpolated value
*/
double meUtilityInterpolation(const std::vector<double>& array1, 
						const std::vector<double>& array2,
						double point,
						const std::string& interpolation)
{
    MLIB_API_START
	double ret;
			
	LAString interp(interpolation.c_str());
	ret = validation_api::tryMeUtilityInterpolation(array1, array2, point, interp);
 
	return ret;
    MLIB_API_END
}
