/* 
 * @brief			Swig interface to Java for function mirGetPCAResults
 * @Created:		27 March 2016 
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#include "stdafx.h"
#include "mirGetPCAResults.h"

#include "LAString.h"
#include "tryMirGetPCAResults.h"
#include "TypeUtilities.h"

/* @brief			swig interface for mirGetPCAResults
*  @param [in]		ID			ID used to identify a PCA analysis
*  @param [in]		update		A number that tracks the nth calculation of the same PCA analysis
*  @return			PCA results
*/
std::vector<std::vector<std::string> > mirGetPCAResults(const std::string& ID, int update) throw(std::exception)
{
	std::vector<std::vector<std::string> > ret;
	try 
	{
		// Input marshalling
		LAString id(ID.c_str());

		StringMatrix temp = validation_api::tryMirGetPCAResults(g_root, id, update);

		ret = swig::fromStringMatrixToMatrixOfString(temp);

	} 
	catch (MEError& mesx) 
	{
		throw std::exception(mesx.getMsg());
	} 

	return ret;

}