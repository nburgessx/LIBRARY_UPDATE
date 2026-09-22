// aqCapFloorObject.cpp

/*
 * @brief			Swig interface for aqCapFloorObject... functions
 */

#include "aqCapFloorObject.h"
#include "AQLCoreTemplateType.h"
#include "TypeUtilities.h"
#include "tryAqCapFloorObject.h"
#include "APISetUp.h"					// AQ_API_START and AQ_API_END Macros

/* @brief			Function to create and store a cap/floor from a label/value block
*  @param [in]		objectName			CapFloor object name
*  @param [in]		tradeLVB			CapFloor definition as a key/value matrix
*  @param [in]		validateKeys		Optional. Default TRUE. Check the LVB keys
*  @return			CapFloor object handle
*/
std::string aqCapFloorObjectCreate( const std::string& objectName,
                                     const SWIG_STRINGMATRIX& tradeLVB,
                                     const bool validateKeys )
{
    AQ_API_START

    // Marshall Inputs
    AQLStringMatrix tradeLVB_;
    swig::buildStringMatrix( tradeLVB_, tradeLVB );
    LabelValueBlock tradeLVBAsLabelValueBlock( tradeLVB_ );

    // Call Function and Return Result
    std::string result = validation::tryAqCapFloorObjectCreate( objectName, tradeLVBAsLabelValueBlock, validateKeys );
    return result;

    AQ_API_END
}

/* @brief			Function to display a cached cap/floor's input parameters
*  @param [in]		objectName			CapFloor object name
*  @return			Display of the cap/floor input parameters
*/
SWIG_STRINGMATRIX aqCapFloorObjectDisplay( const std::string& objectName )
{
    AQ_API_START

    // Call the Function
    AnyTypeMatrix display = validation::tryAqCapFloorObjectDisplay( objectName );

    // Marshall Output(s)
    SWIG_STRINGMATRIX result = swig::fromAnyTypeMatrixToMatrixOfString( display );
    return result;

    AQ_API_END
}

/* @brief			Function to display the cashflows of a cached cap/floor, priced off a volatility surface and a curve collection
*  @param [in]		objectName			CapFloor object name
*  @param [in]		volObjectName		Volatility object name
*  @param [in]		curveCollection		Discount / forward curve collection
*  @param [in]		fixingTableName		Optional. Fixing table object name for past fixings
*  @param [in]		showColumnHeaders	Optional. Default TRUE. Include a header row
*  @param [in]		columnList			Optional. Column header names to show specified columns. Default to empty list showing all columns.
*  @return			CapFloor cashflow display
*/
SWIG_STRINGMATRIX aqCapFloorObjectDisplayCashflows( const std::string& objectName,
                                                     const std::string& volObjectName,
                                                     const std::string& curveCollection,
                                                     const std::string& fixingTableName,
                                                     const bool showColumnHeaders,
                                                     const std::vector<std::string>& columnList )
{
    AQ_API_START

    // Call the Function
    AnyTypeMatrix display = validation::tryAqCapFloorObjectDisplayCashflows(
        objectName, volObjectName, curveCollection, fixingTableName, showColumnHeaders, columnList );

    // Marshall Output(s)
    SWIG_STRINGMATRIX result = swig::fromAnyTypeMatrixToMatrixOfString( display );
    return result;

    AQ_API_END
}

/* @brief			Function to calculate the PV of a cached cap/floor, priced off a volatility surface and a curve collection
*  @param [in]		objectName			CapFloor object name
*  @param [in]		volObjectName		Volatility object name
*  @param [in]		curveCollection		Discount / forward curve collection
*  @param [in]		fixingTableName		Optional. Fixing table object name for past fixings
*  @return			The cap/floor PV
*/
double aqCapFloorObjectPV( const std::string& objectName,
                           const std::string& volObjectName,
                           const std::string& curveCollection,
                           const std::string& fixingTableName )
{
    AQ_API_START

    // Call Function and Return Result
    double result = validation::tryAqCapFloorObjectPV( objectName, volObjectName, curveCollection, fixingTableName );
    return result;

    AQ_API_END
}

/* @brief			Function to calculate the analytical Greeks of a cached cap/floor
*  @param [in]		objectName			CapFloor object name
*  @param [in]		volObjectName		Volatility object name
*  @param [in]		curveCollection		Discount / forward curve collection
*  @param [in]		fixingTableName		Optional. Fixing table object name for past fixings
*  @param [in]		showColumnHeaders	Optional. Default TRUE. Include a header row
*  @return			CapFloor Greeks' display, calculated analytically
*/
SWIG_STRINGMATRIX aqCapFloorObjectGreeksAnalytical( const std::string& objectName,
                                                     const std::string& volObjectName,
                                                     const std::string& curveCollection,
                                                     const std::string& fixingTableName,
                                                     const bool showColumnHeaders )
{
    AQ_API_START

    // Call the Function
    AnyTypeMatrix display = validation::tryAqCapFloorObjectGreeksAnalytical(
        objectName, volObjectName, curveCollection, fixingTableName, showColumnHeaders );

    // Marshall Output(s)
    SWIG_STRINGMATRIX result = swig::fromAnyTypeMatrixToMatrixOfString( display );
    return result;

    AQ_API_END
}

/* @brief			Function to calculate the Greeks of a cached cap/floor by bump-and-revalue
*  @param [in]		objectName			CapFloor object name
*  @param [in]		volObjectName		Volatility object name
*  @param [in]		curveCollection		Discount / forward curve collection
*  @param [in]		deltaBump			Delta bump size
*  @param [in]		gammaBump			Gamma bump size
*  @param [in]		vegaBump			Vega bump size
*  @param [in]		thetaBump			Theta bump size
*  @param [in]		fixingTableName		Optional. Fixing table object name for past fixings
*  @param [in]		showColumnHeaders	Optional. Default TRUE. Include a header row
*  @return			CapFloor Greeks' display, calculated by bump-and-revalue
*/
SWIG_STRINGMATRIX aqCapFloorObjectGreeks( const std::string& objectName,
                                          const std::string& volObjectName,
                                          const std::string& curveCollection,
                                          const double deltaBump,
                                          const double gammaBump,
                                          const double vegaBump,
                                          const double thetaBump,
                                          const std::string& fixingTableName,
                                          const bool showColumnHeaders )
{
    AQ_API_START

    // Call the Function
    AnyTypeMatrix display = validation::tryAqCapFloorObjectGreeks(
        objectName, volObjectName, curveCollection, deltaBump, gammaBump, vegaBump, thetaBump, fixingTableName, showColumnHeaders );

    // Marshall Output(s)
    SWIG_STRINGMATRIX result = swig::fromAnyTypeMatrixToMatrixOfString( display );
    return result;

    AQ_API_END
}

/* @brief			Function to display the cashflows of a cached cap/floor, priced off an explicit rate matrix
*  @param [in]		objectName			CapFloor object name
*  @param [in]		volObjectName		Volatility object name
*  @param [in]		rateData			External discount factors and libor rates
*  @param [in]		showColumnHeaders	Optional. Default TRUE. Include a header row
*  @param [in]		columnList			Optional. Column header names to show specified columns. Default to empty list showing all columns.
*  @return			CapFloor cashflow display
*/
SWIG_STRINGMATRIX aqCapFloorObjectDisplayCashflowsFromRates( const std::string& objectName,
                                                              const std::string& volObjectName,
                                                              const std::vector<std::vector<double> >& rateData,
                                                              const bool showColumnHeaders,
                                                              const std::vector<std::string>& columnList )
{
    AQ_API_START

    // Call the Function
    AnyTypeMatrix display = validation::tryAqCapFloorObjectDisplayCashflowsFromRates(
        objectName, volObjectName, rateData, showColumnHeaders, columnList );

    // Marshall Output(s)
    SWIG_STRINGMATRIX result = swig::fromAnyTypeMatrixToMatrixOfString( display );
    return result;

    AQ_API_END
}

/* @brief			Function to calculate the analytical Greeks of a cached cap/floor, priced off an explicit rate matrix
*  @param [in]		objectName			CapFloor object name
*  @param [in]		volObjectName		Volatility object name
*  @param [in]		rateData			External discount factors and libor rates
*  @param [in]		showColumnHeaders	Optional. Default TRUE. Include a header row
*  @return			CapFloor Greeks' display, calculated analytically
*/
SWIG_STRINGMATRIX aqCapFloorObjectGreeksAnalyticalFromRates( const std::string& objectName,
                                                              const std::string& volObjectName,
                                                              const std::vector<std::vector<double> >& rateData,
                                                              const bool showColumnHeaders )
{
    AQ_API_START

    // Call the Function
    AnyTypeMatrix display = validation::tryAqCapFloorObjectGreeksAnalyticalFromRates(
        objectName, volObjectName, rateData, showColumnHeaders );

    // Marshall Output(s)
    SWIG_STRINGMATRIX result = swig::fromAnyTypeMatrixToMatrixOfString( display );
    return result;

    AQ_API_END
}

/* @brief			Function to calculate the Greeks of a cached cap/floor by bump-and-revalue, priced off an explicit rate matrix
*  @param [in]		objectName			CapFloor object name
*  @param [in]		volObjectName		Volatility object name
*  @param [in]		rateData			External discount factors and libor rates
*  @param [in]		deltaBump			Delta bump size
*  @param [in]		gammaBump			Gamma bump size
*  @param [in]		vegaBump			Vega bump size
*  @param [in]		thetaBump			Theta bump size
*  @param [in]		showColumnHeaders	Optional. Default TRUE. Include a header row
*  @return			CapFloor Greeks' display, calculated by bump-and-revalue
*/
SWIG_STRINGMATRIX aqCapFloorObjectGreeksFromRates( const std::string& objectName,
                                                    const std::string& volObjectName,
                                                    const std::vector<std::vector<double> >& rateData,
                                                    const double deltaBump,
                                                    const double gammaBump,
                                                    const double vegaBump,
                                                    const double thetaBump,
                                                    const bool showColumnHeaders )
{
    AQ_API_START

    // Call the Function
    AnyTypeMatrix display = validation::tryAqCapFloorObjectGreeksFromRates(
        objectName, volObjectName, rateData, deltaBump, gammaBump, vegaBump, thetaBump, showColumnHeaders );

    // Marshall Output(s)
    SWIG_STRINGMATRIX result = swig::fromAnyTypeMatrixToMatrixOfString( display );
    return result;

    AQ_API_END
}

/* @brief			Function to calculate the PV of a cached cap/floor, priced off an explicit rate matrix
*  @param [in]		objectName			CapFloor object name
*  @param [in]		volObjectName		Volatility object name
*  @param [in]		rateData			External discount factors and libor rates
*  @return			The cap/floor PV
*/
double aqCapFloorObjectPVFromRates( const std::string& objectName,
                                     const std::string& volObjectName,
                                     const std::vector<std::vector<double> >& rateData )
{
    AQ_API_START

    // Call Function and Return Result
    double result = validation::tryAqCapFloorObjectPVFromRates( objectName, volObjectName, rateData );
    return result;

    AQ_API_END
}
