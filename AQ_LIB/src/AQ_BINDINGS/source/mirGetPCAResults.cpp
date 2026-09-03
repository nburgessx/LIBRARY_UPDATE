/* 
 * @brief			Swig interface to Java for function mirGetPCAResults
 * @Created:		27 March 2016 
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "InitializeMLibETrading.h"
#include "mirGetPCAResults.h"
#include "LAString.h"
#include "tryMirGetPCAResults.h"
#include "TypeUtilities.h"

/* @brief			swig interface for mirGetPCAResults
*  @param [in]		ID			ID used to identify a PCA analysis
*  @param [in]		update		A number that tracks the nth calculation of the same PCA analysis
*  @return			PCA results
*/
SWIG_STRINGMATRIX mirGetPCAResults(const std::string& ID, int update)
{
	SWIG_STRINGMATRIX ret;
	try 
	{
		// Input marshalling
		LAString id(ID.c_str());

		LAStringMatrix temp = validation_api::tryMirGetPCAResults(etrading::InitializeMLibETrading::instance().dataInstance(), id, update);

		ret = swig::fromStringMatrixToMatrixOfString(temp);

	} 
	catch (LACoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 

	return ret;

}