#pragma once

#include <string>
#include <vector>

/* @brief			swig interface for aqCMSObjectPVUsingConvexityAdjustment
*  @param [in]		swapName			CMS swap object name
*  @param [in]		valuationSettingsLVB	Valuation settings as a label/value block
*  @param [in]		convexityAdjustment	The CMS convexity adjustment to apply
*  @param [in]		legName				The CMS leg to value
*  @param [in]		fixingTableNames	Optional. Fixing-table name overrides as a label/value block
*  @return			Present value of the CMS swap
*/
double aqCMSObjectPVUsingConvexityAdjustment( const std::string& swapName,
                                               const std::vector<std::vector<std::string> >& valuationSettingsLVB,
                                               const double convexityAdjustment,
                                               const std::string& legName,
                                               const std::vector<std::vector<std::string> >& fixingTableNames = std::vector<std::vector<std::string> >() );

/* @brief			swig interface for aqCMSObjectParRateUsingConvexityAdjustment
*  @param [in]		swapName			CMS swap object name
*  @param [in]		valuationSettingsLVB	Valuation settings as a label/value block
*  @param [in]		convexityAdjustment	The CMS convexity adjustment to apply
*  @param [in]		fixingTableNames	Optional. Fixing-table name overrides as a label/value block
*  @return			Par rate of the CMS swap
*/
double aqCMSObjectParRateUsingConvexityAdjustment( const std::string& swapName,
                                                    const std::vector<std::vector<std::string> >& valuationSettingsLVB,
                                                    const double convexityAdjustment,
                                                    const std::vector<std::vector<std::string> >& fixingTableNames = std::vector<std::vector<std::string> >() );
