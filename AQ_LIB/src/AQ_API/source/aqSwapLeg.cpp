// aqSwapLeg.cpp

/*
 * @brief			Swig interface for aqSwapLeg... and aqSwapObjectLeg... functions
 */

#include "aqSwapLeg.h"
#include "AQLCoreTemplateType.h"
#include "TypeUtilities.h"
#include "tryAqSwapLeg.h"
#include "tryAqSwapObjectLeg.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros

/* -------------------------------------------------------------------------
 *  Stateless swap leg (a single-leg LVB in, value out)
 * ---------------------------------------------------------------------- */

/* @brief			swig interface for the aqSwapLegLVBKeys function - the expected keys for a swap leg label/value block
*  @param [in]		legName		Leg type name, e.g. FIXED, FLOAT
*  @return			expected keys
*/
std::vector<std::string> aqSwapLegLVBKeys( const std::string& legName )
{
    AQ_API_START

    AQLString legNameAsAQLString( legName.c_str() );
    std::vector<std::string> result = validation::tryAqSwapLegLVBKeys( legNameAsAQLString );
    return result;

    AQ_API_END
}

/* @brief			swig interface for the aqSwapLegDisplay function
*  @param [in]		legLVB				The leg definition as a label value block
*  @param [in]		validateKeys		True to validate the all keys provided are valid. Default to True
*  @param [in]		showColumnHeaders	Optional. Default TRUE. Include a header row
*  @param [in]		columnList			Column header names to show specified columns. Default to empty list showing all columns
*  @return			Matrix of leg schedule and cashflows
*/
SWIG_STRINGMATRIX aqSwapLegDisplay( const SWIG_STRINGMATRIX & legLVB,
                                 bool validateKeys,
                                 bool showColumnHeaders,
                                 const std::vector<std::string>& columnList )
{
    AQ_API_START

    LabelValueBlock lvb = swig::buildSingleLabelValueBlock( legLVB );

    AnyTypeMatrix result = validation::tryAqSwapLegDisplay( lvb, validateKeys, showColumnHeaders, columnList );

    SWIG_STRINGMATRIX resultsStringMatrix = swig::fromAnyTypeMatrixToMatrixOfString( result );
    return resultsStringMatrix;

    AQ_API_END
}

/* @brief			swig interface for the aqSwapLegPV function
*  @param [in]		legLVB			A label value block defining the swap leg
*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
*  @return			Swap Leg PV
*/
double aqSwapLegPV( const SWIG_STRINGMATRIX & legLVB, bool validateKeys )
{
    AQ_API_START

    LabelValueBlock lvb = swig::buildSingleLabelValueBlock( legLVB );

    double result = validation::tryAqSwapLegPV( lvb, validateKeys );
    return result;

    AQ_API_END
}

/* @brief			swig interface for the aqSwapLegAnnuity function
*  @param [in]		legLVB			A label value block defining the swap leg
*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
*  @return			Swap Leg Annuity
*/
double aqSwapLegAnnuity( const SWIG_STRINGMATRIX & legLVB, bool validateKeys )
{
    AQ_API_START

    LabelValueBlock lvb = swig::buildSingleLabelValueBlock( legLVB );

    double result = validation::tryAqSwapLegAnnuity( lvb, validateKeys );
    return result;

    AQ_API_END
}

/* -------------------------------------------------------------------------
 *  Swap leg object lifecycle
 * ---------------------------------------------------------------------- */

/* @brief			swig interface for the aqSwapObjectLegCreate function
*  @param [in]		legObjectName	Leg object name
*  @param [in]		legLVB			Leg label value block
*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
*  @return			The leg object handle name
*/
std::string aqSwapObjectLegCreate( const std::string& legObjectName,
                                const SWIG_STRINGMATRIX & legLVB,
                                bool validateKeys )
{
    AQ_API_START

    LabelValueBlock lvb = swig::buildSingleLabelValueBlock( legLVB );

    std::string result = validation::tryAqSwapObjectLegCreate( legObjectName, lvb, validateKeys );
    return result;

    AQ_API_END
}

/* @brief			swig interface for the aqSwapObjectLegCreateFromSchedule function
*  @param [in]		legObjectName	Leg object name
*  @param [in]		scheduleName	A cached schedule handle
*  @param [in]		legLVB			Leg label value block
*  @param [in]		validateKeys	True to validate the all keys provided are valid. Default to True
*  @return			The leg object handle name
*/
std::string aqSwapObjectLegCreateFromSchedule( const std::string& legObjectName,
                                            const std::string& scheduleName,
                                            const SWIG_STRINGMATRIX & legLVB,
                                            bool validateKeys )
{
    AQ_API_START

    LabelValueBlock lvb = swig::buildSingleLabelValueBlock( legLVB );

    std::string result = validation::tryAqSwapObjectLegCreateFromSchedule( legObjectName, scheduleName, lvb, validateKeys );
    return result;

    AQ_API_END
}

/* @brief			swig interface for the aqSwapObjectLegPV function
*  @param [in]		legObjectName			Leg object name
*  @param [in]		valuationSettingsLVB	A LVB containing ModelName, CurveCollection, ValuationDate, etc.
*  @param [in]		fixingTableName			Optional. A cached fixing-table handle
*  @return			Leg PV
*/
double aqSwapObjectLegPV( const std::string& legObjectName,
                       const SWIG_STRINGMATRIX & valuationSettingsLVB,
                       const std::string& fixingTableName )
{
    AQ_API_START

    LabelValueBlock valuationSettingsLvb = swig::buildSingleLabelValueBlock( valuationSettingsLVB );

    double result = validation::tryAqSwapObjectLegPV( legObjectName, valuationSettingsLvb, fixingTableName );
    return result;

    AQ_API_END
}

/* @brief			swig interface for the aqSwapObjectLegDisplay function, displays the INPUT parameters of the cached swap leg
*  @param [in]		legObjectName	Leg object name
*  @return			Leg display of the input parameters
*/
SWIG_STRINGMATRIX aqSwapObjectLegDisplay( const std::string& legObjectName )
{
    AQ_API_START

    AQLStringMatrix result = validation::tryAqSwapObjectLegDisplay( legObjectName );

    SWIG_STRINGMATRIX resultsStringMatrix = swig::fromStringMatrixToMatrixOfString( result );
    return resultsStringMatrix;

    AQ_API_END
}

/* @brief			swig interface for the aqSwapObjectLegDisplayCashflows function
*  @param [in]		legObjectName			Leg object name
*  @param [in]		valuationSettingsLVB	A LVB containing ModelName, CurveCollection, ValuationDate, etc.
*  @param [in]		fixingTableName			Optional. A cached fixing-table handle
*  @param [in]		showColumnHeaders		Optional. Default TRUE
*  @param [in]		columnList				Column header names to show specified columns. Default to empty list showing all columns
*  @return			Leg display of leg output
*/
SWIG_STRINGMATRIX aqSwapObjectLegDisplayCashflows( const std::string& legObjectName,
                                                const SWIG_STRINGMATRIX & valuationSettingsLVB,
                                                const std::string& fixingTableName,
                                                bool showColumnHeaders,
                                                const std::vector<std::string>& columnList )
{
    AQ_API_START

    LabelValueBlock valuationSettingsLvb = swig::buildSingleLabelValueBlock( valuationSettingsLVB );

    AnyTypeMatrix result = validation::tryAqSwapObjectLegDisplayCashflows( legObjectName, valuationSettingsLvb, fixingTableName, showColumnHeaders, columnList );

    SWIG_STRINGMATRIX resultsStringMatrix = swig::fromAnyTypeMatrixToMatrixOfString( result );
    return resultsStringMatrix;

    AQ_API_END
}
