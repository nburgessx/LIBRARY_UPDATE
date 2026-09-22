// aqInflationObject.cpp

/*
 * @brief			Swig interface for aqInflationCurve... / aqInflationObject... functions
 */

#include "aqInflationObject.h"
#include "AQLCoreTemplateType.h"
#include "AQLDate.h"
#include "TypeUtilities.h"          // Swig Marshalling Helper Methods
#include "ContainerUtilities.h"
#include "JSONInfoBlock.h"
#include "CoreEnumerations.h"
#include "ExceptionMacros.h"
#include "Variant.h"                // Variant and Variant Matrix Types
#include "tryAqInflationObjectPricing.h"
#include "APISetUp.h"					// AQ_API_START and AQ_API_END Macros
#include "ParameterValidation.h"		// etrading::stringToDate

namespace
{
    /* @brief			Build a JSONInfoBlockTuple (column names, column types, transposed data) from a key/value matrix.
    *  @param [in]		block			The data block, as an AQLStringMatrix
    *  @return			The equivalent JSONInfoBlockTuple
    */
    etrading::JSONInfoBlockTuple aqInflationTableInfo( const AQLStringMatrix& block )
    {
        etrading::VariantMatrix variantMatrix;
        swig::buildVariantMatrix( variantMatrix, block );
        const etrading::VariantMatrix transposedMatrix = etrading::transpose( variantMatrix );

        size_t numColumns = transposedMatrix.size();

        // EnumTypes for each column
        std::vector<etrading::ContainedTypeEnum> columnEnumTypes = etrading::Variant::getContainedTypeInfo( transposedMatrix );

        // Construct dummy column headings
        std::vector<int> nColCounters;
        boost::push_back( nColCounters, boost::irange( 1, static_cast<int>( numColumns ) + 1 ) );
        std::vector<std::string> columnNames( numColumns, std::string( "COL_" ) );
        columnNames = etrading::zip_paste<std::vector<std::string>, std::vector<int>, std::string>( columnNames, nColCounters );

        // Construct the TableInfo
        return std::make_tuple( columnNames, columnEnumTypes, transposedMatrix );
    }
}

/* @brief			Function to create and store an inflation curve from one or two named data blocks
*  @param [in]		inflationCurveName	Name for the inflation curve object
*  @param [in]		key1				Name of the first data block
*  @param [in]		value1				First data block, as a key/value matrix
*  @param [in]		key2				Optional. Name of the second data block
*  @param [in]		value2				Optional. Second data block, as a key/value matrix
*  @return			Inflation curve object handle
*/
std::string aqInflationCurveCreate( const std::string& inflationCurveName,
                                     const std::string& key1,
                                     const SWIG_STRINGMATRIX& value1,
                                     const std::string& key2,
                                     const SWIG_STRINGMATRIX& value2 )
{
    AQ_API_START

    // Marshall Inputs
    std::vector<std::string> dataBlockNames;
    etrading::JSONInfoBlockTuples infoBlocks;

    AQLStringMatrix matrix1;
    swig::buildStringMatrix( matrix1, value1 );
    dataBlockNames.push_back( key1 );
    infoBlocks.push_back( aqInflationTableInfo( matrix1 ) );

    if ( value2.size() > 0 )
    {
        AQLStringMatrix matrix2;
        swig::buildStringMatrix( matrix2, value2 );
        dataBlockNames.push_back( key2 );
        infoBlocks.push_back( aqInflationTableInfo( matrix2 ) );
    }

    // Call Function and Return Result
    std::string result = validation::tryAqInflationCurveCreate( inflationCurveName, dataBlockNames, infoBlocks );
    return result;

    AQ_API_END
}

/* @brief			Function to obtain the calibration parameters from a cached inflation curve
*  @param [in]		inflationCurveName	Inflation curve object name
*  @return			A matrix containing node dates and calibrated index levels
*/
SWIG_STRINGMATRIX aqInflationCurveCalibrationParameters( const std::string& inflationCurveName )
{
    AQ_API_START

    // Call the Function
    AnyTypeMatrix calibrationParameters = validation::tryAqInflationCurveCalibrationParameters( inflationCurveName );

    // Marshall Output(s)
    SWIG_STRINGMATRIX result = swig::fromAnyTypeMatrixToMatrixOfString( calibrationParameters );
    return result;

    AQ_API_END
}

/* @brief			Function to obtain the CPI level from a cached inflation curve for the specified date
*  @param [in]		inflationCurveName	Inflation curve object name
*  @param [in]		date				Return the CPI for this date
*  @param [in]		inflationResetType	Reset convention: MonthlyInterpolation or DailyInterpolation
*  @param [in]		lag					Adjust the specified date backwards by this lag tenor
*  @return			The calculated CPI level
*/
double aqInflationObjectCPI( const std::string& inflationCurveName,
                              const std::string& date,
                              const std::string& inflationResetType,
                              const std::string& lag )
{
    AQ_API_START

    // Marshall Inputs
    AQLDate date_( etrading::stringToDate( date ) );

    // Call Function and Return Result
    double result = validation::tryAqInflationObjectCPI( inflationCurveName, date_, inflationResetType, lag );
    return result;

    AQ_API_END
}

/* @brief			Function to calculate the PV of a cached zero coupon inflation swap, off a stored inflation curve
*  @param [in]		swapName			Zero coupon inflation swap object name
*  @param [in]		inflationCurveName	Inflation curve object name
*  @param [in]		valuationSettingsLVB	Valuation settings as a label/value block
*  @param [in]		legName				If specified, calculate the PV of the single swap leg
*  @return			Present value of the zero coupon inflation swap
*/
double aqInflationObjectZCSwapPV( const std::string& swapName,
                                   const std::string& inflationCurveName,
                                   const std::vector<std::vector<std::string> >& valuationSettingsLVB,
                                   const std::string& legName )
{
    AQ_API_START

    // Marshall Inputs
    LabelValueBlock valuationSettingsLVB_ = swig::buildSingleLabelValueBlock( valuationSettingsLVB );

    // Call Function and Return Result
    double result = validation::tryAqInflationObjectZCSwapPV( swapName, inflationCurveName, valuationSettingsLVB_, legName );
    return result;

    AQ_API_END
}

/* @brief			Function to calculate the PV of a cached zero coupon inflation swap, off explicit base/reset index levels
*  @param [in]		swapName			Zero coupon inflation swap object name
*  @param [in]		valuationSettingsLVB	Valuation settings as a label/value block
*  @param [in]		baseIndex			The inflation level at the effective date of the swap
*  @param [in]		resetIndex			The inflation level at the maturity of the swap
*  @param [in]		legName				If specified, calculate the PV of the single swap leg
*  @return			Present value of the zero coupon inflation swap
*/
double aqInflationObjectZCSwapPVFromIndex( const std::string& swapName,
                                            const std::vector<std::vector<std::string> >& valuationSettingsLVB,
                                            const double baseIndex,
                                            const double resetIndex,
                                            const std::string& legName )
{
    AQ_API_START

    // Marshall Inputs
    LabelValueBlock valuationSettingsLVB_ = swig::buildSingleLabelValueBlock( valuationSettingsLVB );

    // Call Function and Return Result
    double result = validation::tryAqInflationObjectZCSwapPVFromIndex( swapName, valuationSettingsLVB_, baseIndex, resetIndex, legName );
    return result;

    AQ_API_END
}

/* @brief			Function to calculate the break-even par rate of a cached zero coupon inflation swap, off a stored inflation curve
*  @param [in]		swapName			Zero coupon inflation swap object name
*  @param [in]		inflationCurveName	Inflation curve object name
*  @param [in]		valuationSettingsLVB	Valuation settings as a label/value block
*  @return			Break-even par rate of the zero coupon inflation swap
*/
double aqInflationObjectZCSwapParRate( const std::string& swapName,
                                        const std::string& inflationCurveName,
                                        const std::vector<std::vector<std::string> >& valuationSettingsLVB )
{
    AQ_API_START

    // Marshall Inputs
    LabelValueBlock valuationSettingsLVB_ = swig::buildSingleLabelValueBlock( valuationSettingsLVB );

    // Call Function and Return Result
    double result = validation::tryAqInflationObjectZCSwapParRate( swapName, inflationCurveName, valuationSettingsLVB_ );
    return result;

    AQ_API_END
}

/* @brief			Function to calculate the break-even par rate of a cached zero coupon inflation swap, off explicit base/reset index levels
*  @param [in]		swapName			Zero coupon inflation swap object name
*  @param [in]		valuationSettingsLVB	Valuation settings as a label/value block
*  @param [in]		baseIndex			The inflation level at the effective date of the swap
*  @param [in]		resetIndex			The inflation level at the maturity of the swap
*  @return			Break-even par rate of the zero coupon inflation swap
*/
double aqInflationObjectZCSwapParRateFromIndex( const std::string& swapName,
                                                 const std::vector<std::vector<std::string> >& valuationSettingsLVB,
                                                 const double baseIndex,
                                                 const double resetIndex )
{
    AQ_API_START

    // Marshall Inputs
    LabelValueBlock valuationSettingsLVB_ = swig::buildSingleLabelValueBlock( valuationSettingsLVB );

    // Call Function and Return Result
    double result = validation::tryAqInflationObjectZCSwapParRateFromIndex( swapName, valuationSettingsLVB_, baseIndex, resetIndex );
    return result;

    AQ_API_END
}
