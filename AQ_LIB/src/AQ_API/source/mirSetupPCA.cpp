#include "InitializeAQETrading.h"
#include "mirSetupPCA.h"
#include "AQLString.h"
#include "tryMirSetupPCA.h"


/* @brief			swig interface for mirSetupPCA
*  @param [in]		ID			ID used to identify a PCA analysis
*  @param [in]		Data		Data under analysis
*  @param [in]		IsScale		True means using correlation matrix. False means using covariance matrix. Default to false
*  @param [in]		FactorNum	The number of PCA factors to show results for
*  @return			The ID string that identifies the current PCA analysis
*/
std::string mirSetupPCA(const std::string& ID,
						const std::vector<std::vector<double> >& data,
						bool IsScale,
						int FactorNum)
{
	std::string ret;
	try 
	{
		// Input marshalling
		AQLString id(ID.c_str());

		ret = validation::tryMirSetupPCA(etrading::InitializeAQETrading::instance().dataInstance(), id, data, IsScale, FactorNum).getCString();
	} 
	catch (AQLCoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 

	return ret;
}