/*
 * Volatility category - xlOil worksheet functions.
 *
 * aqVolatilityObject* operates on a cached vol surface / SABR market data /
 * SABR model (names in). Each function pairs with the identically named
 * validation wrapper (plus the `try` prefix). Marshalling to and from Excel is
 * the aq_xll helpers in xllSupport.h.
 *
 * aqVolatilitySABR{Calibrate,GetPrem,GetVol,OutputParameter,SetupConvention,
 * SetupParameter,SetupSwaptionVol} (below) are the legacy procedural,
 * ID-string-driven calibration workflow, ported as-is alongside the
 * object-based Sabr* API above.
 */

#include <xllMain.h>

#include <cmath>
#include <string>
#include <vector>

#include <xllSupport.h>
#include <CoreEnumerations.h>        // etrading::trim_to_upper
#include <tryAqVolatilityObject.h>   // validation::tryAqVolatilityObject*
#include <tryAqVolatilitySABRCalibrate.h>
#include <tryAqVolatilitySABRGetPrem.h>
#include <tryAqVolatilitySABRGetVol.h>
#include <tryAqVolatilitySABROutputParameter.h>
#include <tryAqVolatilitySABRSetupConvention.h>
#include <tryAqVolatilitySABRSetupParameter.h>
#include <tryAqVolatilitySABRSetupSwaptionVol.h>

using namespace aq_xll;


// Create and store a volatility surface from a label/value block.
#if AQ_XLL_ENABLED(aqVolatilityObjectCreate)
XLO_FUNC_START( aqVolatilityObjectCreate(
    const ExcelObj& objectName,
    const ExcelObj& volLVB,
    const ExcelObj& validateKeys ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string name = decorateWithExcelLocation( toNarrowString( objectName ) );

    const std::string storedName = validation::tryAqVolatilityObjectCreate(
        name, toLabelValueBlock( volLVB ), toBool( validateKeys, true ) );

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqVolatilityObjectCreate )
    .help( L"Create and store a volatility surface from a label/value block; returns its handle." )
    .arg( L"ObjectName",   L"Name for the volatility-surface object" )
    .arg( L"VolLVB",       L"The volatility-surface definition as a label/value block" )
    .arg( L"ValidateKeys", L"Optional. Default TRUE. Check the LVB keys" );
#endif


// Create and store SABR market data from one or two named data blocks.
#if AQ_XLL_ENABLED(aqVolatilityObjectSabrMarketDataCreate)
XLO_FUNC_START( aqVolatilityObjectSabrMarketDataCreate(
    const ExcelObj& objectName,
    const ExcelObj& key1,
    const ExcelObj& value1,
    const ExcelObj& key2,
    const ExcelObj& value2 ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string name = decorateWithExcelLocation( toNarrowString( objectName ) );

    std::vector<std::string>           dataBlockNames;
    etrading::JSONInfoBlockTuples infoBlocks;

    dataBlockNames.push_back( etrading::trim_to_upper( toNarrowString( key1 ) ) );
    infoBlocks.push_back( toTableInfo( value1 ) );

    if ( !value2.isMissing() && value2.isNonEmpty() )
    {
        dataBlockNames.push_back( etrading::trim_to_upper( toNarrowString( key2 ) ) );
        infoBlocks.push_back( toTableInfo( value2 ) );
    }

    const std::string storedName =
        validation::tryAqVolatilityObjectSabrMarketDataCreate( name, dataBlockNames, infoBlocks );

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqVolatilityObjectSabrMarketDataCreate )
    .help( L"Create and store SABR market data from one or two named data blocks; returns its handle." )
    .arg( L"ObjectName", L"Name for the SABR market-data object" )
    .arg( L"Key1",       L"Name of the first data block" )
    .arg( L"Value1",     L"First data block, as a range" )
    .arg( L"Key2",       L"Optional. Name of the second data block" )
    .arg( L"Value2",     L"Optional. Second data block, as a range" );
#endif


// Calibrate and store a SABR model from one or two named data blocks.
#if AQ_XLL_ENABLED(aqVolatilityObjectSabrModelCalibrate)
XLO_FUNC_START( aqVolatilityObjectSabrModelCalibrate(
    const ExcelObj& objectName,
    const ExcelObj& key1,
    const ExcelObj& value1,
    const ExcelObj& key2,
    const ExcelObj& value2 ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const std::string name = decorateWithExcelLocation( toNarrowString( objectName ) );

    std::vector<std::string>           dataBlockNames;
    etrading::JSONInfoBlockTuples infoBlocks;

    dataBlockNames.push_back( etrading::trim_to_upper( toNarrowString( key1 ) ) );
    infoBlocks.push_back( toTableInfo( value1 ) );

    if ( !value2.isMissing() && value2.isNonEmpty() )
    {
        dataBlockNames.push_back( etrading::trim_to_upper( toNarrowString( key2 ) ) );
        infoBlocks.push_back( toTableInfo( value2 ) );
    }

    const std::string storedName =
        validation::tryAqVolatilityObjectSabrModelCalibrate( name, dataBlockNames, infoBlocks );

    return returnValue( appendInstanceCounter( storedName ) );
}
XLO_FUNC_END( aqVolatilityObjectSabrModelCalibrate )
    .help( L"Calibrate and store a SABR model from one or two named data blocks; returns its handle." )
    .arg( L"ObjectName", L"Name for the SABR model object" )
    .arg( L"Key1",       L"Name of the first data block" )
    .arg( L"Value1",     L"First data block, as a range" )
    .arg( L"Key2",       L"Optional. Name of the second data block" )
    .arg( L"Value2",     L"Optional. Second data block, as a range" );
#endif


// SABR-implied volatility at an expiry/tenor/strike/forward point.
#if AQ_XLL_ENABLED(aqVolatilityObjectSabrVolatility)
XLO_FUNC_START( aqVolatilityObjectSabrVolatility(
    const ExcelObj& volSurfaceName,
    const ExcelObj& expiry,
    const ExcelObj& tenor,
    const ExcelObj& strike,
    const ExcelObj& forward ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqVolatilityObjectSabrVolatility(
        getNameWithoutCounter( volSurfaceName ),
        toNarrowString( expiry ),
        toNarrowString( tenor ),
        strike.get<double>(),
        forward.get<double>() ) );
}
XLO_FUNC_END( aqVolatilityObjectSabrVolatility )
    .help( L"SABR-implied volatility at an expiry/tenor/strike/forward point." )
    .arg( L"VolSurfaceName", L"A volatility-surface handle" )
    .arg( L"Expiry",         L"Option expiry, e.g. 3M, 1Y" )
    .arg( L"Tenor",          L"Underlying tenor, e.g. 5Y, 10Y" )
    .arg( L"Strike",         L"Strike" )
    .arg( L"Forward",        L"Forward rate" );
#endif


// One SABR parameter at an expiry/tenor point.
#if AQ_XLL_ENABLED(aqVolatilityObjectSabrParameter)
XLO_FUNC_START( aqVolatilityObjectSabrParameter(
    const ExcelObj& volSurfaceName,
    const ExcelObj& expiry,
    const ExcelObj& tenor,
    const ExcelObj& paramName ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( validation::tryAqVolatilityObjectSabrParameter(
        getNameWithoutCounter( volSurfaceName ),
        toNarrowString( expiry ),
        toNarrowString( tenor ),
        toNarrowString( paramName ) ) );
}
XLO_FUNC_END( aqVolatilityObjectSabrParameter )
    .help( L"One SABR parameter (alpha, beta, rho, nu, ...) at an expiry/tenor point." )
    .arg( L"VolSurfaceName", L"A volatility-surface handle" )
    .arg( L"Expiry",         L"Option expiry, e.g. 3M, 1Y" )
    .arg( L"Tenor",          L"Underlying tenor, e.g. 5Y, 10Y" )
    .arg( L"ParamName",      L"Which parameter, e.g. ALPHA, BETA, RHO, NU" );
#endif


// Display stored SABR market data.
#if AQ_XLL_ENABLED(aqVolatilityObjectSabrMarketDataDisplay)
XLO_FUNC_START( aqVolatilityObjectSabrMarketDataDisplay(
    const ExcelObj& marketDataObjectName,
    const ExcelObj& marketDataKey ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix( validation::tryAqVolatilityObjectSabrMarketDataDisplay(
        getNameWithoutCounter( marketDataObjectName ), toNarrowString( marketDataKey ) ) ) );
}
XLO_FUNC_END( aqVolatilityObjectSabrMarketDataDisplay )
    .help( L"Display stored SABR market data as a matrix." )
    .arg( L"MarketDataObjectName", L"A SABR market-data handle" )
    .arg( L"MarketDataKey",        L"The data block to display" );
#endif


// Display a stored SABR model.
#if AQ_XLL_ENABLED(aqVolatilityObjectSabrModelDisplay)
XLO_FUNC_START( aqVolatilityObjectSabrModelDisplay(
    const ExcelObj& modelObjectName,
    const ExcelObj& modelKey ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    return returnValue( toExcelMatrix( validation::tryAqVolatilityObjectSabrModelDisplay(
        getNameWithoutCounter( modelObjectName ), toNarrowString( modelKey ) ) ) );
}
XLO_FUNC_END( aqVolatilityObjectSabrModelDisplay )
    .help( L"Display a stored SABR model as a matrix." )
    .arg( L"ModelObjectName", L"A SABR model handle" )
    .arg( L"ModelKey",        L"The model block to display" );
#endif


// SABR calibration workflow (legacy procedural, ID-string driven).
//
// Unlike aqVolatilityObject* above, these do not operate on AQObj handles -
// their ID parameters (GridID, ConventionID, ...) are plain string keys into
// the shared AQLDataInstance object pool, set up by one of these same
// functions. They are passed straight through with toAQLString, with no
// decorateWithExcelLocation / appendInstanceCounter decoration - the same
// convention xllDate.cpp uses for a plain string ID (e.g. CentralBankId in
// aqDateCentralBank).

// Calibrate a SABR volatility matrix.
#if AQ_XLL_ENABLED(aqVolatilitySABRCalibrate)
XLO_FUNC_START( aqVolatilitySABRCalibrate(
    const ExcelObj& approxMethod,
    const ExcelObj& calibFlg,
    const ExcelObj& calibMethod,
    const ExcelObj& curveSetID,
    const ExcelObj& alphaID,
    const ExcelObj& betaID,
    const ExcelObj& nuID,
    const ExcelObj& rhoID,
    const ExcelObj& convID,
    const ExcelObj& capConvID,
    const ExcelObj& swapVolID,
    const ExcelObj& target,
    const ExcelObj& weight,
    const ExcelObj& sgn,
    const ExcelObj& forwardID,
    const ExcelObj& forwardShiftValue,
    const ExcelObj& numeraireID,
    const ExcelObj& curveMat,
    const ExcelObj& volType ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    // Sgn -> IntVector: xllSupport.h has no dedicated int-vector helper, so
    // this reuses toDoubleVector and rounds, consistent with the file's other
    // numeric-vector marshalling.
    const std::vector<double> sgnAsDouble = toDoubleVector( sgn, true, "Sgn" );
    IntVector sgnVector;
    for ( const double value : sgnAsDouble )
    {
        sgnVector.push_back( static_cast<int>( std::lround( value ) ) );
    }

    const std::string result = std::string( validation::tryAqVolatilitySABRCalibrate(
        toAQLString( approxMethod ),
        toAQLStringVector( calibFlg ),
        toAQLString( calibMethod ),
        toAQLString( curveSetID ),
        toAQLString( alphaID ),
        toAQLString( betaID ),
        toAQLString( nuID ),
        toAQLString( rhoID ),
        toAQLString( convID ),
        toAQLString( capConvID ),
        toAQLStringVector( swapVolID ),
        toAQLString( target ),
        toDoubleVector( weight, true, "Weight" ),
        sgnVector,
        toAQLString( forwardID ),
        forwardShiftValue.get<double>(),
        toAQLString( numeraireID ),
        toAQLStringMatrix( curveMat ),
        toNarrowString( volType ) ).getCString() );

    return returnValue( result );
}
XLO_FUNC_END( aqVolatilitySABRCalibrate )
    .help( L"Calibrate a SABR volatility matrix (legacy procedural, ID-string driven)." )
    .arg( L"ApproxMethod",     L"SABR approximation method, e.g. HAGAN" )
    .arg( L"CalibFlg",         L"Per-parameter calibration flags (TRUE/FALSE)" )
    .arg( L"CalibMethod",      L"Calibration method" )
    .arg( L"CurveSetID",       L"ID of the curve set" )
    .arg( L"AlphaID",          L"ID for the alpha parameter matrix" )
    .arg( L"BetaID",           L"ID for the beta parameter matrix" )
    .arg( L"NuID",             L"ID for the nu parameter matrix" )
    .arg( L"RhoID",            L"ID for the rho parameter matrix" )
    .arg( L"ConvID",           L"ID of the convention data. Do not supply together with ForwardID" )
    .arg( L"CapConvID",        L"ID of the cap convention data" )
    .arg( L"SwapVolID",        L"IDs of the swaption vol matrices to calibrate against" )
    .arg( L"Target",           L"Calibration target" )
    .arg( L"Weight",           L"Calibration weights" )
    .arg( L"Sgn",              L"Calibration signs (1 or -1) per point" )
    .arg( L"ForwardID",        L"ID for the forward rate matrix. Do not supply together with ConvID" )
    .arg( L"ForwardShiftValue", L"Shift applied to the forward rate" )
    .arg( L"NumeraireID",      L"ID for the numeraire/annuity matrix" )
    .arg( L"CurveMat",         L"Curve matrix, required when ForwardID is not given" )
    .arg( L"VolType",          L"Optional. Volatility type, default lognormal" );
#endif


// SABR-implied premium at an expiry/tenor/strike point (legacy ID-string workflow).
#if AQ_XLL_ENABLED(aqVolatilitySABRGetPrem)
XLO_FUNC_START( aqVolatilitySABRGetPrem(
    const ExcelObj& expPoint,
    const ExcelObj& tenorPoint,
    const ExcelObj& strike,
    const ExcelObj& sign,
    const ExcelObj& forwardID,
    const ExcelObj& numeraireID,
    const ExcelObj& alphaID,
    const ExcelObj& betaID,
    const ExcelObj& nuID,
    const ExcelObj& rhoID,
    const ExcelObj& approxMethod,
    const ExcelObj& shift,
    const ExcelObj& volType ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const double shiftValue = shift.isMissing() ? 0.0 : shift.get<double>();

    return returnValue( validation::tryAqVolatilitySABRGetPrem(
        toAQLString( expPoint ),
        toAQLString( tenorPoint ),
        strike.get<double>(),
        static_cast<int>( std::lround( sign.get<double>() ) ),
        toAQLString( forwardID ),
        toAQLString( numeraireID ),
        toAQLString( alphaID ),
        toAQLString( betaID ),
        toAQLString( nuID ),
        toAQLString( rhoID ),
        toAQLString( approxMethod ),
        shiftValue,
        toNarrowString( volType ) ) );
}
XLO_FUNC_END( aqVolatilitySABRGetPrem )
    .help( L"SABR-implied premium at an expiry/tenor/strike point (legacy procedural, ID-string driven)." )
    .arg( L"ExpPoint",     L"Expiry tenor" )
    .arg( L"TenorPoint",   L"Underlying swap tenor" )
    .arg( L"Strike",       L"Strike" )
    .arg( L"Sign",         L"1 for Call, -1 for Put" )
    .arg( L"ForwardID",    L"ID for the forward rate matrix" )
    .arg( L"NumeraireID",  L"ID for the annuity matrix" )
    .arg( L"AlphaID",      L"ID for the alpha parameter matrix" )
    .arg( L"BetaID",       L"ID for the beta parameter matrix" )
    .arg( L"NuID",         L"ID for the nu parameter matrix" )
    .arg( L"RhoID",        L"ID for the rho parameter matrix" )
    .arg( L"ApproxMethod", L"Approximation method, e.g. HAGAN" )
    .arg( L"Shift",        L"Optional. Default 0. Shift size of the shifted SABR" )
    .arg( L"VolType",      L"Optional. Volatility type, default lognormal" );
#endif


// SABR-implied volatility at an expiry/tenor/strike point (legacy ID-string workflow).
#if AQ_XLL_ENABLED(aqVolatilitySABRGetVol)
XLO_FUNC_START( aqVolatilitySABRGetVol(
    const ExcelObj& expPoint,
    const ExcelObj& tenorPoint,
    const ExcelObj& strike,
    const ExcelObj& forwardID,
    const ExcelObj& alphaID,
    const ExcelObj& betaID,
    const ExcelObj& nuID,
    const ExcelObj& rhoID,
    const ExcelObj& approxMethod,
    const ExcelObj& shift,
    const ExcelObj& volType ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    const double shiftValue = shift.isMissing() ? 0.0 : shift.get<double>();

    return returnValue( validation::tryAqVolatilitySABRGetVol(
        toAQLString( expPoint ),
        toAQLString( tenorPoint ),
        strike.get<double>(),
        toAQLString( forwardID ),
        toAQLString( alphaID ),
        toAQLString( betaID ),
        toAQLString( nuID ),
        toAQLString( rhoID ),
        toAQLString( approxMethod ),
        shiftValue,
        toNarrowString( volType ) ) );
}
XLO_FUNC_END( aqVolatilitySABRGetVol )
    .help( L"SABR-implied volatility at an expiry/tenor/strike point (legacy procedural, ID-string driven)." )
    .arg( L"ExpPoint",     L"Expiry tenor" )
    .arg( L"TenorPoint",   L"Underlying swap tenor" )
    .arg( L"Strike",       L"Strike" )
    .arg( L"ForwardID",    L"ID for the forward rate matrix" )
    .arg( L"AlphaID",      L"ID for the alpha parameter matrix" )
    .arg( L"BetaID",       L"ID for the beta parameter matrix" )
    .arg( L"NuID",         L"ID for the nu parameter matrix" )
    .arg( L"RhoID",        L"ID for the rho parameter matrix" )
    .arg( L"ApproxMethod", L"Approximation method, e.g. HAGAN" )
    .arg( L"Shift",        L"Optional. Default 0. Shift size of the shifted SABR" )
    .arg( L"VolType",      L"Optional. Volatility type, default lognormal" );
#endif


// Return a stored SABR grid as a matrix (legacy ID-string workflow).
#if AQ_XLL_ENABLED(aqVolatilitySABROutputParameter)
XLO_FUNC_START( aqVolatilitySABROutputParameter(
    const ExcelObj& gridID ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    DoubleVector ret;
    size_t row = 0;
    size_t column = 0;
    validation::tryAqVolatilitySABROutputParameter( toAQLString( gridID ), ret, row, column );

    // ret is flattened row-major (see AQLMathSwaptionVolUtility::outPutSABRGrid:
    // the outer loop walks rows, the inner loop walks columns, pushing
    // mat[i][j] in that order) - reshape it back into a row x column matrix.
    // toExcelMatrix has no DoubleVector-plus-dimensions overload, so each
    // value is stringified into an AQLStringMatrix cell (toExcelMatrix(AQLStringMatrix)
    // already returns numeric-looking text as a real Excel number).
    AQLStringMatrix matrix( row, AQLStringVector( column ) );
    for ( size_t r = 0; r < row; ++r )
    {
        for ( size_t c = 0; c < column; ++c )
        {
            matrix[r][c] = AQLString( std::to_string( ret[r * column + c] ).c_str() );
        }
    }

    return returnValue( toExcelMatrix( matrix ) );
}
XLO_FUNC_END( aqVolatilitySABROutputParameter )
    .help( L"Return a stored SABR grid as a matrix (legacy procedural, ID-string driven)." )
    .arg( L"GridID", L"ID identifying the data grid" );
#endif


// Store SABR calibration convention data under an ID (legacy ID-string workflow).
#if AQ_XLL_ENABLED(aqVolatilitySABRSetupConvention)
XLO_FUNC_START( aqVolatilitySABRSetupConvention(
    const ExcelObj& conventionID,
    const ExcelObj& convData ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    // tryAqVolatilitySABRSetupConvention is void - convData is read by the
    // function (see the .cpp: it upper-cases and searches its own local
    // copy), not written back for the caller's benefit. There is no XLL
    // precedent yet for a genuinely void validation wrapper, so this returns
    // TRUE to confirm the setup succeeded, matching the boolean-confirmation
    // shape used for other non-value-returning calls in this add-in.
    AQLStringMatrix convDataMatrix = toAQLStringMatrix( convData );
    validation::tryAqVolatilitySABRSetupConvention( toAQLString( conventionID ), convDataMatrix );

    return returnValue( true );
}
XLO_FUNC_END( aqVolatilitySABRSetupConvention )
    .help( L"Store SABR calibration convention data under an ID (legacy procedural, ID-string driven)." )
    .arg( L"ConventionID", L"ID to store the convention data under" )
    .arg( L"ConvData",     L"Convention data, as a range" );
#endif


// Store a SABR parameter grid under an ID (legacy ID-string workflow).
#if AQ_XLL_ENABLED(aqVolatilitySABRSetupParameter)
XLO_FUNC_START( aqVolatilitySABRSetupParameter(
    const ExcelObj& gridID,
    const ExcelObj& conventionID,
    const ExcelObj& gridData ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    // As aqVolatilitySABRSetupConvention above: gridData is read, not written
    // back; TRUE confirms the setup succeeded.
    AQLStringMatrix gridDataMatrix = toAQLStringMatrix( gridData );
    validation::tryAqVolatilitySABRSetupParameter(
        toAQLString( gridID ), toAQLString( conventionID ), gridDataMatrix );

    return returnValue( true );
}
XLO_FUNC_END( aqVolatilitySABRSetupParameter )
    .help( L"Store a SABR parameter grid under an ID (legacy procedural, ID-string driven)." )
    .arg( L"GridID",       L"ID of the parameter grid being set up" )
    .arg( L"ConventionID", L"ID of the convention data" )
    .arg( L"GridData",     L"Data of the parameter grid being set up, as a range" );
#endif


// Store a swaption vol/strike/sign matrix set under an ID (legacy ID-string workflow).
#if AQ_XLL_ENABLED(aqVolatilitySABRSetupSwaptionVol)
XLO_FUNC_START( aqVolatilitySABRSetupSwaptionVol(
    const ExcelObj& gridID,
    const ExcelObj& volMat,
    const ExcelObj& strikeMat,
    const ExcelObj& signMat ) )
{
    AQ_XLL_GUARD
    AQ_INITIALIZE

    // The validation header marks volMat/strikeMat/signMat [inout], but the
    // .cpp implementation (AQLMathSwaptionVolUtility::setUpSwaptionVol) only
    // erases its own header row/column while reading them - nothing is
    // written back that the caller needs, so as above these are marshalled
    // as plain inputs and TRUE confirms the setup succeeded.
    AQLStringMatrix volMatrix    = toAQLStringMatrix( volMat );
    AQLStringMatrix strikeMatrix = toAQLStringMatrix( strikeMat );
    AQLStringMatrix signMatrix   = toAQLStringMatrix( signMat );

    validation::tryAqVolatilitySABRSetupSwaptionVol(
        toAQLString( gridID ), volMatrix, strikeMatrix, signMatrix );

    return returnValue( true );
}
XLO_FUNC_END( aqVolatilitySABRSetupSwaptionVol )
    .help( L"Store a swaption vol/strike/sign matrix set under an ID (legacy procedural, ID-string driven)." )
    .arg( L"GridID",     L"ID that identifies the swaption vol matrix set" )
    .arg( L"VolMat",     L"Matrix of swaption vols, as a range" )
    .arg( L"StrikeMat",  L"Matrix of strikes, as a range" )
    .arg( L"SignMat",    L"Matrix of signs, as a range" );
#endif
