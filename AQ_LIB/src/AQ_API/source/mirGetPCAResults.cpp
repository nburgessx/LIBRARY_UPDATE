#include "InitializeAQETrading.h"
#include "mirGetPCAResults.h"
#include "AQLString.h"
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
		AQLString id(ID.c_str());

		AQLStringMatrix temp = validation::tryMirGetPCAResults(etrading::InitializeAQETrading::instance().dataInstance(), id, update);

		ret = swig::fromStringMatrixToMatrixOfString(temp);

	} 
	catch (AQLCoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 

	return ret;

}