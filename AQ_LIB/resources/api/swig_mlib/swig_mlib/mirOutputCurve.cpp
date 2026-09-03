/* 
 * @brief			Swig interface to Java for function mirOutputCurve
 * @Created:		31 March 2016 
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#include "stdafx.h"
#include "mirOutputCurve.h"
#include "LAString.h"
#include "tryMirOutputCurve.h"

/* @brief			swig interface for mirOutputCurve
*  @param [in]		curveId			ID of the curve set
*  @param [in]		curveName		Name of the curve set
*  @return			Array of terms and corresponding discount factors from the yield curve
*/
std::vector<double> mirOutputCurve(const std::string& curveId,
								const std::string& curveName) throw(std::exception)
{
	DoubleArray ret;
	try 
	{
		// marshall all inputs		
		LAString curId			(curveId.c_str());
		LAString curName		(curveName.c_str());
		
		ret = validation_api::tryMirOutputCurve(g_root,
												curId,
												curName);


	} 
	catch (MEError& mesx) 
	{
		throw std::exception(mesx.getMsg());
	} 

	return ret;

}