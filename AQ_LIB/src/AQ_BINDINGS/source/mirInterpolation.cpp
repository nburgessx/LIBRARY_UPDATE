#include "mirInterpolation.h"
#include "TypeUtilities.h"
#include "LAString.h"
#include "tryMirInterpolation.h"

/* @brief			swig interface for mirInterpolation
*  @param [in]		array1			X-axis values of the data to be interpolated
*  @param [in]		array2			Y-axis values of the data to be interpolated
*  @param [in]		point			X-axis point to interpolate
*  @param [in]		interpolation	Interpolation method of the yield curve
*  @return			the Y-axis interpolated value
*/
double mirInterpolation(const std::vector<double>& array1, 
						const std::vector<double>& array2,
						double point,
						const std::string& interpolation)
{
	double ret;
	try 
	{
		
		LAString interp(interpolation.c_str());
		ret = validation_api::tryMirInterpolation(array1, array2, point, interp);


	} 
	catch (LACoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 
	
	return ret;
}
