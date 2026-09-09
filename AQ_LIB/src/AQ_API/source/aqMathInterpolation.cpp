#include "TypeUtilities.h"
#include "AQLString.h"

#include "aqMathInterpolation.h"
#include "tryAqMathInterpolation.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros


/* @brief			swig interface for aqMathInterpolation
*  @param [in]		xValues			X-axis values of the data
*  @param [in]		yValues			Y-axis values of the data
*  @param [in]		xPoint			X-axis point
*  @param [in]		interpolation	Interpolation method
*  @return			the Y-axis interpolated value
*/
double aqMathInterpolation(const std::vector<double>& array1, 
						const std::vector<double>& array2,
						double point,
						const std::string& interpolation)
{
    AQ_API_START
	double ret;
			
	AQLString interp(interpolation.c_str());
	ret = validation::tryAqMathInterpolation(array1, array2, point, interp);
 
	return ret;
    AQ_API_END
}
