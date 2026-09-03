/* 
 * @brief			Swig interface to Java for function meUtilityPCA
 * @Created:		03 June 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include <string>
#include <vector>

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
							   int nFactors);