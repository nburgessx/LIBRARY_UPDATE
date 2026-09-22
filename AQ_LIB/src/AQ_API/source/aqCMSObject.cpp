// aqCMSObject.cpp

/*
 * @brief			Swig interface for aqCMSObject... functions
 */

#include "aqCMSObject.h"
#include "AQLCoreTemplateType.h"
#include "TypeUtilities.h"
#include "tryAqSwapObjectPricing.h"
#include "APISetUp.h"					// AQ_API_START and AQ_API_END Macros

/* @brief			Function to calculate the PV of a cached CMS swap, applying an explicit convexity adjustment
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
                                               const std::vector<std::vector<std::string> >& fixingTableNames )
{
    AQ_API_START

    // Marshall Inputs
    LabelValueBlock valuationSettingsLVB_ = swig::buildSingleLabelValueBlock( valuationSettingsLVB );
    AQLString legNameAsAQLString( legName.c_str() );
    LabelValueBlock fixingTableNames_ = swig::buildSingleLabelValueBlock( fixingTableNames );

    // Call Function and Return Result
    double result = validation::tryAqCMSObjectPVUsingConvexityAdjustment( swapName,
                                                                            valuationSettingsLVB_,
                                                                            convexityAdjustment,
                                                                            legNameAsAQLString,
                                                                            fixingTableNames_ );
    return result;

    AQ_API_END
}

/* @brief			Function to calculate the par rate of a cached CMS swap, applying an explicit convexity adjustment
*  @param [in]		swapName			CMS swap object name
*  @param [in]		valuationSettingsLVB	Valuation settings as a label/value block
*  @param [in]		convexityAdjustment	The CMS convexity adjustment to apply
*  @param [in]		fixingTableNames	Optional. Fixing-table name overrides as a label/value block
*  @return			Par rate of the CMS swap
*/
double aqCMSObjectParRateUsingConvexityAdjustment( const std::string& swapName,
                                                    const std::vector<std::vector<std::string> >& valuationSettingsLVB,
                                                    const double convexityAdjustment,
                                                    const std::vector<std::vector<std::string> >& fixingTableNames )
{
    AQ_API_START

    // Marshall Inputs
    LabelValueBlock valuationSettingsLVB_ = swig::buildSingleLabelValueBlock( valuationSettingsLVB );
    LabelValueBlock fixingTableNames_ = swig::buildSingleLabelValueBlock( fixingTableNames );

    // Call Function and Return Result
    double result = validation::tryAqCMSObjectParRateUsingConvexityAdjustment( swapName,
                                                                                 valuationSettingsLVB_,
                                                                                 convexityAdjustment,
                                                                                 fixingTableNames_ );
    return result;

    AQ_API_END
}
