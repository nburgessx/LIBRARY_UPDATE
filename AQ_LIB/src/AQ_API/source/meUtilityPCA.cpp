#include "LAString.h"
#include "TypeUtilities.h"

#include "meUtilityPCA.h"
#include "tryMeUtilityPCA.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros


/* @brief			swig interface for meUtilityPCA
*  @param [in]		key						Key used to identify a PCA analysis
*  @param [in]		data					Data under analysis
*  @param [in]		useCorrelationMatrix	True to use correlation matrix. False to use covariance matrix. Default to False
*  @param [in]		nFactors				The number of PCA factors
*  @return			A LAStringMatrix for the PCA results
*/
SWIG_STRINGMATRIX meUtilityPCA(const std::string& key,
							   const std::vector<std::vector<double> >& data,
							   bool useCorrelationMatrix,
							   int nFactors)
{
    AQ_API_START

	// Input marshalling
	LAString tmp_key(key.c_str());

	SWIG_STRINGMATRIX ret;
	
	LAStringMatrix pca = validation::tryMeUtilityPCA(tmp_key, data, useCorrelationMatrix, nFactors);
	ret = swig::fromStringMatrixToMatrixOfString(pca);
	
	return ret;

    AQ_API_END
}