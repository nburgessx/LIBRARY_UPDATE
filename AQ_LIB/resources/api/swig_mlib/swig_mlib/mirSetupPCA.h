/* 
 * @brief			Swig interface to Java for function mirSetupPCA
 * @Created:		27 March 2016 
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include <string>
#include <vector>

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
						int FactorNum) throw(std::exception);