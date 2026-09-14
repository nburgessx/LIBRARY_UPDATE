/*
 * Excel-side helpers for the AQ xlOil add-in. See aqXllTools.h.
 */

#include <aqXllTools.h>

#include "ExceptionMacros.h"

#include <xloil/ExcelArray.h>
#include <xloil/ArrayBuilder.h>
#include <xloil/ExcelCall.h>
#include <xloil/XlCallSlim.h>

#include <algorithm>
#include <cctype>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <functional>

#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/static_visitor.hpp>

#include <CommonConstants.h>
#include <DateUtilities.h>
#include <AQLDateScheduleHelpers.h>
#include <Environment.h>
#include <AQLCoreAppError.h>
#include <Variant.h>                // etrading::canStringConvertToNumber

namespace aq_xll
{
    namespace
    {
        // Handle behaviour switches.
        bool instanceCountNames_             = true;
        bool decorateNamesWithExcelAddress_  = false;
        bool convertExcelAddressToUniqueID_  = false;

        // objectName -> current instance counter
        std::unordered_map< std::string, int > namesToCounter_;

        // xlOil can register thread-safe worksheet functions, so the map is
        // mutex-guarded.
        std::mutex counterMutex_;

        // The counter wraps at 100 so it never grows without bound, and never
        // returns to 0 once used, so a live handle is always visibly decorated.
        const int COUNTER_WRAP = 100;

        const std::string& counterDelimiter()
        {
            return etrading::AQOBJ_KEY::AQOBJ_OBJECT_COUNTER_DELIMITER;
        }
    }

    // -------------------------------------------------------------------------
    //  Marshalling: Excel -> AQ
    // -------------------------------------------------------------------------

    std::string toNarrowString( const xloil::ExcelObj& obj )
    {
        const std::wstring wide = obj.toString();
        return std::string( wide.begin(), wide.end() );
    }

    AQLString toAQLString( const xloil::ExcelObj& obj )
    {
        return AQLString( toNarrowString( obj ).c_str() );
    }

    AQLDate toAQLDate( const xloil::ExcelObj& obj )
    {
        // A number is an Excel date serial; anything else is parsed as a date string.
        if ( obj.isType( xloil::ExcelType::Num ) || obj.isType( xloil::ExcelType::Int ) )
        {
            const int excelSerial = static_cast< int >( obj.get< double >() );
            return etrading::toAQLDateFromGregorianDate( etrading::toGregorianDateFromExcelDate( excelSerial ) );
        }

        return etrading::toAQLDateFromREGEX( toNarrowString( obj ) );
    }

    DateVector toDateVector( const xloil::ExcelObj& obj,
                             bool skipTrailingBlanks,
                             const std::string& nameOfVariable )
    {
        // Flatten a single cell or a range into a list of cells, row by row.
        std::vector< const xloil::ExcelObj* > cells;

        if ( obj.isType( xloil::ExcelType::Multi ) )
        {
            xloil::ExcelArray array( obj, false /* do not trim, we handle it below */ );
            cells.reserve( array.size() );
            for ( size_t row = 0; row < array.nRows(); ++row )
            {
                for ( size_t col = 0; col < array.nCols(); ++col )
                {
                    cells.push_back( &array( row, col ) );
                }
            }
        }
        else
        {
            cells.push_back( &obj );
        }

        // An error value anywhere is a hard failure, so a #REF! in the input
        // never silently becomes a date.
        for ( const xloil::ExcelObj* cell : cells )
        {
            if ( cell->isType( xloil::ExcelType::Err ) )
            {
                const std::string message = "Invalid Vector Input: " + nameOfVariable + " date input required";
                AQ_THROW( message );
            }
        }

        // Trailing blanks are dropped so a user can select a whole column.
        size_t count = cells.size();
        if ( skipTrailingBlanks )
        {
            while ( count > 0 && !cells[count - 1]->isNonEmpty() )
            {
                --count;
            }
        }

        DateVector dates;
        dates.reserve( count );
        for ( size_t i = 0; i < count; ++i )
        {
            dates.push_back( toAQLDate( *cells[i] ) );
        }

        return dates;
    }

    boost::gregorian::date toGregorian( const AQLDate& date )
    {
        return etrading::toGregorianDateFromAQLDate( date );
    }

    std::vector<boost::gregorian::date> toGregorianVector( const xloil::ExcelObj& obj,
                                                            bool skipTrailingBlanks,
                                                            const std::string& nameOfVariable )
    {
        const DateVector aqlDates = toDateVector( obj, skipTrailingBlanks, nameOfVariable );

        std::vector<boost::gregorian::date> dates;
        dates.reserve( aqlDates.size() );
        for ( const AQLDate& date : aqlDates )
        {
            dates.push_back( toGregorian( date ) );
        }
        return dates;
    }

    std::vector<double> toDoubleVector( const xloil::ExcelObj& obj,
                                       bool skipTrailingBlanks,
                                       const std::string& nameOfVariable )
    {
        // Flatten a single cell or a range into a list of cells, row by row -
        // the same shape handling as toDateVector.
        std::vector< const xloil::ExcelObj* > cells;

        if ( obj.isType( xloil::ExcelType::Multi ) )
        {
            xloil::ExcelArray array( obj, false /* do not trim, handled below */ );
            cells.reserve( array.size() );
            for ( size_t row = 0; row < array.nRows(); ++row )
            {
                for ( size_t col = 0; col < array.nCols(); ++col )
                {
                    cells.push_back( &array( row, col ) );
                }
            }
        }
        else
        {
            cells.push_back( &obj );
        }

        // An error value anywhere is a hard failure, so a #REF! in the input
        // never silently becomes a number.
        for ( const xloil::ExcelObj* cell : cells )
        {
            if ( cell->isType( xloil::ExcelType::Err ) )
            {
                const std::string message = "Invalid Vector Input: " + nameOfVariable + " numeric input required";
                AQ_THROW( message );
            }
        }

        // Trailing blanks are dropped so a user can select a whole column.
        size_t count = cells.size();
        if ( skipTrailingBlanks )
        {
            while ( count > 0 && !cells[count - 1]->isNonEmpty() )
            {
                --count;
            }
        }

        std::vector<double> values;
        values.reserve( count );
        for ( size_t i = 0; i < count; ++i )
        {
            const xloil::ExcelObj& cell = *cells[i];

            if ( cell.isType( xloil::ExcelType::Num ) || cell.isType( xloil::ExcelType::Int ) )
            {
                values.push_back( cell.get<double>() );
                continue;
            }

            if ( !cell.isNonEmpty() )
            {
                // An interior blank counts as zero.
                values.push_back( 0.0 );
                continue;
            }

            // A text cell that reads cleanly as a number is accepted; anything
            // else is rejected the same way an error cell is.
            const std::string text = toNarrowString( cell );
            bool parsed = false;
            try
            {
                size_t consumed = 0;
                const double number = std::stod( text, &consumed );
                if ( consumed == text.size() )
                {
                    values.push_back( number );
                    parsed = true;
                }
            }
            catch ( ... )
            {
                // Fall through to the failure below.
            }

            if ( !parsed )
            {
                const std::string message = "Invalid Vector Input: " + nameOfVariable + " numeric input required";
                AQ_THROW( message );
            }
        }

        return values;
    }

    std::vector<std::string> toStringVector( const xloil::ExcelObj& obj,
                                             bool skipTrailingBlanks )
    {
        std::vector< const xloil::ExcelObj* > cells;

        if ( obj.isType( xloil::ExcelType::Multi ) )
        {
            xloil::ExcelArray array( obj, false );
            cells.reserve( array.size() );
            for ( size_t row = 0; row < array.nRows(); ++row )
            {
                for ( size_t col = 0; col < array.nCols(); ++col )
                {
                    cells.push_back( &array( row, col ) );
                }
            }
        }
        else
        {
            cells.push_back( &obj );
        }

        size_t count = cells.size();
        if ( skipTrailingBlanks )
        {
            while ( count > 0 && !cells[count - 1]->isNonEmpty() )
            {
                --count;
            }
        }

        std::vector<std::string> values;
        values.reserve( count );
        for ( size_t i = 0; i < count; ++i )
        {
            values.push_back( cells[i]->isNonEmpty() ? toNarrowString( *cells[i] ) : std::string() );
        }

        return values;
    }

    AQLStringVector toAQLStringVector( const xloil::ExcelObj& obj, bool skipTrailingBlanks )
    {
        const std::vector<std::string> narrow = toStringVector( obj, skipTrailingBlanks );

        AQLStringVector values;
        values.reserve( narrow.size() );
        for ( const std::string& value : narrow )
        {
            values.push_back( AQLString( value.c_str() ) );
        }
        return values;
    }

    namespace
    {
        // One Excel cell -> a Variant, keeping its native type. An error cell is
        // carried through as its text rather than throwing; a blank cell is an
        // EMPTY Variant.
        etrading::Variant excelCellToVariant( const xloil::ExcelObj& cell )
        {
            if ( cell.isType( xloil::ExcelType::Bool ) )
            {
                return etrading::Variant( cell.get<bool>() );
            }
            if ( cell.isType( xloil::ExcelType::Num ) || cell.isType( xloil::ExcelType::Int ) )
            {
                return etrading::Variant( cell.get<double>() );
            }
            if ( !cell.isNonEmpty() )
            {
                return etrading::Variant();
            }
            return etrading::Variant( toNarrowString( cell ).c_str() );
        }

        // One Variant -> an Excel cell, keeping its native type.
        xloil::ExcelObj variantToExcel( const etrading::Variant& value )
        {
            switch ( value.getType() )
            {
                case etrading::INTEGER_VALUE:
                case etrading::DOUBLE_VALUE:
                    return xloil::ExcelObj( value.getValue<double>() );
                case etrading::BOOL_VALUE:
                    return xloil::ExcelObj( value.getValue<bool>() );
                case etrading::EMPTY_VALUE:
                    return xloil::ExcelObj();
                default:
                {
                    const std::string text = value.getValueAsString();
                    return xloil::ExcelObj( std::wstring( text.begin(), text.end() ) );
                }
            }
        }
    }

    DoubleMatrix toDoubleMatrix( const xloil::ExcelObj& obj )
    {
        DoubleMatrix matrix;

        if ( !obj.isType( xloil::ExcelType::Multi ) )
        {
            matrix.push_back( DoubleVector( 1, obj.isNonEmpty() ? obj.get<double>() : 0.0 ) );
            return matrix;
        }

        xloil::ExcelArray array( obj, false );
        matrix.reserve( array.nRows() );
        for ( size_t r = 0; r < array.nRows(); ++r )
        {
            DoubleVector row;
            row.reserve( array.nCols() );
            for ( size_t c = 0; c < array.nCols(); ++c )
            {
                const xloil::ExcelObj& cell = array( r, c );
                row.push_back( cell.isNonEmpty() ? cell.get<double>() : 0.0 );
            }
            matrix.push_back( std::move( row ) );
        }
        return matrix;
    }

    StandardStringMatrix toStandardStringMatrix( const xloil::ExcelObj& obj )
    {
        const AQLStringMatrix aqlMatrix = toAQLStringMatrix( obj );

        StandardStringMatrix matrix;
        matrix.reserve( aqlMatrix.size() );
        for ( const AQLStringVector& aqlRow : aqlMatrix )
        {
            StandardStringVector row;
            row.reserve( aqlRow.size() );
            for ( const AQLString& cell : aqlRow )
            {
                row.emplace_back( cell.getCString() );
            }
            matrix.push_back( std::move( row ) );
        }
        return matrix;
    }

    etrading::VariantMatrix toVariantMatrix( const xloil::ExcelObj& obj )
    {
        etrading::VariantMatrix matrix;

        if ( !obj.isType( xloil::ExcelType::Multi ) )
        {
            etrading::VariantVector row;
            row.push_back( excelCellToVariant( obj ) );
            matrix.push_back( row );
            return matrix;
        }

        xloil::ExcelArray array( obj, false );
        matrix.reserve( array.nRows() );
        for ( size_t r = 0; r < array.nRows(); ++r )
        {
            etrading::VariantVector row;
            row.reserve( array.nCols() );
            for ( size_t c = 0; c < array.nCols(); ++c )
            {
                row.push_back( excelCellToVariant( array( r, c ) ) );
            }
            matrix.push_back( std::move( row ) );
        }
        return matrix;
    }

    etrading::VariantVector toVariantVector( const xloil::ExcelObj& obj )
    {
        etrading::VariantVector values;

        if ( !obj.isType( xloil::ExcelType::Multi ) )
        {
            values.push_back( excelCellToVariant( obj ) );
            return values;
        }

        xloil::ExcelArray array( obj, false );
        values.reserve( array.nRows() * array.nCols() );
        for ( size_t r = 0; r < array.nRows(); ++r )
        {
            for ( size_t c = 0; c < array.nCols(); ++c )
            {
                values.push_back( excelCellToVariant( array( r, c ) ) );
            }
        }
        return values;
    }

    // -------------------------------------------------------------------------
    //  Marshalling: AQ -> Excel
    // -------------------------------------------------------------------------

    double toExcelDate( const AQLDate& date )
    {
        return static_cast< double >( etrading::AQLDateScheduleHelpers::getExcelDate( date ) );
    }

    xloil::ExcelObj toExcelDateColumn( const DateVector& dates )
    {
        if ( dates.empty() )
        {
            return xloil::ExcelObj( xloil::CellError::NA );
        }

        // A single result is returned as a scalar so it does not need array entry.
        if ( dates.size() == 1 )
        {
            return xloil::ExcelObj( toExcelDate( dates[0] ) );
        }

        xloil::ExcelArrayBuilder builder( static_cast< uint32_t >( dates.size() ), 1 );
        for ( size_t i = 0; i < dates.size(); ++i )
        {
            builder( static_cast< uint32_t >( i ), 0 ) = toExcelDate( dates[i] );
        }

        return builder.toExcelObj();
    }

    xloil::ExcelObj toExcelColumn( const std::vector<std::string>& values )
    {
        if ( values.empty() )
        {
            return xloil::ExcelObj( xloil::CellError::NA );
        }

        size_t totalStringLength = 0;
        for ( const std::string& value : values )
        {
            totalStringLength += value.size();
        }

        // No padTo2DimArray: an N x 1 column must stay N x 1. Padding it to a
        // 2-D minimum leaves an uninitialised second column, which Excel then
        // renders by repeating the first value down the caller range.
        xloil::ExcelArrayBuilder builder( static_cast<uint32_t>( values.size() ), 1,
                                          totalStringLength );
        for ( uint32_t i = 0; i < values.size(); ++i )
        {
            builder( i, 0 ) = xloil::ExcelObj( std::wstring( values[i].begin(), values[i].end() ) );
        }
        return builder.toExcelObj();
    }

    xloil::ExcelObj toExcelDoubleColumn( const std::vector<double>& values )
    {
        if ( values.empty() )
        {
            return xloil::ExcelObj( xloil::CellError::NA );
        }

        // A single result is returned as a scalar so it does not need array entry.
        if ( values.size() == 1 )
        {
            return xloil::ExcelObj( values[0] );
        }

        xloil::ExcelArrayBuilder builder( static_cast< uint32_t >( values.size() ), 1 );
        for ( size_t i = 0; i < values.size(); ++i )
        {
            builder( static_cast< uint32_t >( i ), 0 ) = values[i];
        }

        return builder.toExcelObj();
    }

    xloil::ExcelObj toExcelIntColumn( const std::vector<int>& values )
    {
        if ( values.empty() )
        {
            return xloil::ExcelObj( xloil::CellError::NA );
        }

        if ( values.size() == 1 )
        {
            return xloil::ExcelObj( static_cast< double >( values[0] ) );
        }

        xloil::ExcelArrayBuilder builder( static_cast< uint32_t >( values.size() ), 1 );
        for ( size_t i = 0; i < values.size(); ++i )
        {
            builder( static_cast< uint32_t >( i ), 0 ) = static_cast< double >( values[i] );
        }

        return builder.toExcelObj();
    }

    // -------------------------------------------------------------------------
    //  Marshalling helpers: Excel -> AQ  (scalars, matrices, LVBs)
    // -------------------------------------------------------------------------

    bool toBool( const xloil::ExcelObj& obj, bool defaultValue )
    {
        if ( obj.isMissing() || !obj.isNonEmpty() )
        {
            return defaultValue;
        }

        if ( obj.isType( xloil::ExcelType::Bool ) )
        {
            return obj.get<bool>();
        }

        if ( obj.isType( xloil::ExcelType::Num ) || obj.isType( xloil::ExcelType::Int ) )
        {
            return obj.get<double>() != 0.0;
        }

        // Fall back to a text reading: "true"/"false", "yes"/"no", "1"/"0".
        std::string text = toNarrowString( obj );
        std::transform( text.begin(), text.end(), text.begin(),
                        []( unsigned char c ){ return static_cast<char>( std::tolower( c ) ); } );

        if ( text == "true" || text == "yes" || text == "1" )  return true;
        if ( text == "false" || text == "no" || text == "0" || text.empty() ) return false;

        return defaultValue;
    }

    AQLStringMatrix toAQLStringMatrix( const xloil::ExcelObj& obj )
    {
        AQLStringMatrix matrix;

        if ( !obj.isType( xloil::ExcelType::Multi ) )
        {
            // A single cell is a 1x1 block.
            AQLStringVector row;
            row.push_back( AQLString( toNarrowString( obj ).c_str() ) );
            matrix.push_back( row );
            return matrix;
        }

        xloil::ExcelArray array( obj, false /* keep the caller's shape, trim below */ );

        for ( size_t r = 0; r < array.nRows(); ++r )
        {
            AQLStringVector row;
            row.reserve( array.nCols() );

            bool rowIsAllBlank = true;
            for ( size_t c = 0; c < array.nCols(); ++c )
            {
                const xloil::ExcelObj& cell = array( r, c );

                if ( cell.isType( xloil::ExcelType::Err ) )
                {
                    AQ_THROW( "cell contains an error value" );
                }

                const std::string text = cell.isNonEmpty() ? toNarrowString( cell ) : std::string();
                if ( !text.empty() )
                {
                    rowIsAllBlank = false;
                }
                row.push_back( AQLString( text.c_str() ) );
            }

            // Defer fully-blank rows; only keep them if a later row has content,
            // so an over-selected block does not carry a tail of empty pairs.
            if ( rowIsAllBlank )
            {
                continue;
            }
            matrix.push_back( row );
        }

        return matrix;
    }

    etrading::LabelValueBlock toLabelValueBlock( const xloil::ExcelObj& obj )
    {
        return etrading::LabelValueBlock( toAQLStringMatrix( obj ) );
    }

    namespace
    {
        AQLStringMatrix transposeStringMatrix( const AQLStringMatrix& matrix )
        {
            if ( matrix.empty() )
            {
                return matrix;
            }

            size_t cols = 0;
            for ( const AQLStringVector& row : matrix )
            {
                cols = std::max( cols, row.size() );
            }

            AQLStringMatrix result( cols, AQLStringVector( matrix.size() ) );
            for ( size_t r = 0; r < matrix.size(); ++r )
            {
                for ( size_t c = 0; c < matrix[r].size(); ++c )
                {
                    result[c][r] = matrix[r][c];
                }
            }
            return result;
        }
    }

    etrading::LabelValueBlock toLabelValueBlock( const xloil::ExcelObj& obj, bool keysAreVertical )
    {
        const AQLStringMatrix matrix = toAQLStringMatrix( obj );
        return etrading::LabelValueBlock( keysAreVertical ? matrix : transposeStringMatrix( matrix ) );
    }

    std::tuple< std::vector<std::string>,
                std::vector<etrading::ContainedTypeEnum>,
                etrading::VariantMatrix >
        toTableInfo( const xloil::ExcelObj& obj )
    {
        // Read the range column by column. A single cell is a 1x1 range.
        std::unique_ptr<xloil::ExcelArray> array;
        size_t rows = 1;
        size_t cols = 1;
        if ( obj.isType( xloil::ExcelType::Multi ) )
        {
            array.reset( new xloil::ExcelArray( obj, false ) );
            rows = array->nRows();
            cols = array->nCols();
        }

        auto at = [&]( size_t r, size_t c ) -> const xloil::ExcelObj&
        {
            return array ? ( *array )( r, c ) : obj;
        };

        etrading::VariantMatrix dataByColumn;
        dataByColumn.reserve( cols );
        for ( size_t c = 0; c < cols; ++c )
        {
            etrading::VariantVector column;
            column.reserve( rows );
            for ( size_t r = 0; r < rows; ++r )
            {
                const xloil::ExcelObj& cell = at( r, c );
                if ( cell.isType( xloil::ExcelType::Err ) )
                {
                    AQ_THROW( "generator block contains an error value" );
                }
                const std::string text = cell.isNonEmpty() ? toNarrowString( cell ) : std::string();
                column.push_back( etrading::Variant( text.c_str() ) );
            }
            dataByColumn.push_back( column );
        }

        std::vector<std::string> columnNames;
        columnNames.reserve( cols );
        for ( size_t c = 0; c < cols; ++c )
        {
            columnNames.push_back( "COL_" + std::to_string( c + 1 ) );
        }

        const std::vector<etrading::ContainedTypeEnum> columnTypes =
            etrading::Variant::getContainedTypeInfo( dataByColumn );

        return std::make_tuple( columnNames, columnTypes, dataByColumn );
    }

    // -------------------------------------------------------------------------
    //  Marshalling helpers: AQ -> Excel  (matrices)
    // -------------------------------------------------------------------------

    namespace
    {
        // A cell that arrives as text but reads as a number is returned to Excel
        // as a number, so it can be formatted (currency, date serial, decimals).
        // Anything that is not cleanly numeric stays as text. LabelValueBlock
        // stores every value as a std::string, so without this every displayed
        // rate, notional and price would land in Excel as un-formattable text.
        xloil::ExcelObj numericAwareStringToExcel( const std::string& text )
        {
            if ( !text.empty() && etrading::canStringConvertToNumber( text ) )
            {
                try
                {
                    size_t consumed = 0;
                    const double number = std::stod( text, &consumed );
                    if ( consumed == text.size() )
                    {
                        return xloil::ExcelObj( number );
                    }
                }
                catch ( ... )
                {
                    // Fall through and return the original text.
                }
            }
            return xloil::ExcelObj( std::wstring( text.begin(), text.end() ) );
        }

        // Turn one AnyType (boost::variant) cell into an ExcelObj, keeping the
        // native Excel type. Strings are collected as wide strings; the caller
        // has already reserved room for them in the ArrayBuilder.
        struct AnyTypeToExcel : boost::static_visitor<xloil::ExcelObj>
        {
            xloil::ExcelObj operator()( int value ) const        { return xloil::ExcelObj( static_cast<double>( value ) ); }
            xloil::ExcelObj operator()( double value ) const      { return xloil::ExcelObj( value ); }
            xloil::ExcelObj operator()( bool value ) const        { return xloil::ExcelObj( value ); }
            xloil::ExcelObj operator()( const std::string& value ) const
            {
                return numericAwareStringToExcel( value );
            }
            xloil::ExcelObj operator()( const AQLString& value ) const
            {
                return numericAwareStringToExcel( value.getCString() );
            }
            xloil::ExcelObj operator()( const char* value ) const
            {
                return numericAwareStringToExcel( value != nullptr ? value : "" );
            }
        };

        size_t wideLengthOfAnyType( const AnyType& value )
        {
            struct LengthVisitor : boost::static_visitor<size_t>
            {
                size_t operator()( int ) const                    { return 0; }
                size_t operator()( double ) const                 { return 0; }
                size_t operator()( bool ) const                   { return 0; }
                size_t operator()( const std::string& s ) const   { return s.size(); }
                size_t operator()( const AQLString& s ) const     { return std::string( s.getCString() ).size(); }
                size_t operator()( const char* s ) const          { return s != nullptr ? std::char_traits<char>::length( s ) : 0; }
            };
            return boost::apply_visitor( LengthVisitor(), value );
        }
    }

    xloil::ExcelObj toExcelMatrix( const AnyTypeMatrix& matrix )
    {
        if ( matrix.empty() || matrix[0].empty() )
        {
            return xloil::ExcelObj( xloil::CellError::NA );
        }

        const uint32_t nRows = static_cast<uint32_t>( matrix.size() );
        uint32_t nCols = 0;
        size_t totalStringLength = 0;
        for ( const auto& row : matrix )
        {
            nCols = std::max( nCols, static_cast<uint32_t>( row.size() ) );
            for ( const AnyType& cell : row )
            {
                totalStringLength += wideLengthOfAnyType( cell );
            }
        }

        xloil::ExcelArrayBuilder builder( nRows, nCols, totalStringLength, true /* pad to 2D */ );
        const AnyTypeToExcel toExcel;

        for ( uint32_t r = 0; r < nRows; ++r )
        {
            for ( uint32_t c = 0; c < nCols; ++c )
            {
                if ( c < matrix[r].size() )
                {
                    builder( r, c ) = boost::apply_visitor( toExcel, matrix[r][c] );
                }
                else
                {
                    builder( r, c ) = xloil::ExcelObj( xloil::CellError::NA );
                }
            }
        }

        return builder.toExcelObj();
    }

    xloil::ExcelObj toExcelMatrix( const AQLStringMatrix& matrix )
    {
        if ( matrix.empty() || matrix[0].empty() )
        {
            return xloil::ExcelObj( xloil::CellError::NA );
        }

        const uint32_t nRows = static_cast<uint32_t>( matrix.size() );
        uint32_t nCols = 0;
        size_t totalStringLength = 0;
        for ( const auto& row : matrix )
        {
            nCols = std::max( nCols, static_cast<uint32_t>( row.size() ) );
            for ( const AQLString& cell : row )
            {
                totalStringLength += std::string( cell.getCString() ).size();
            }
        }

        xloil::ExcelArrayBuilder builder( nRows, nCols, totalStringLength, true /* pad to 2D */ );
        for ( uint32_t r = 0; r < nRows; ++r )
        {
            for ( uint32_t c = 0; c < nCols; ++c )
            {
                builder( r, c ) = ( c < matrix[r].size() )
                    ? numericAwareStringToExcel( std::string( matrix[r][c].getCString() ) )
                    : xloil::ExcelObj( xloil::CellError::NA );
            }
        }

        return builder.toExcelObj();
    }

    xloil::ExcelObj toExcelMatrix( const etrading::VariantMatrix& matrix )
    {
        if ( matrix.empty() || matrix[0].empty() )
        {
            return xloil::ExcelObj( xloil::CellError::NA );
        }

        const uint32_t nRows = static_cast<uint32_t>( matrix.size() );
        uint32_t nCols = 0;
        size_t totalStringLength = 0;
        for ( const etrading::VariantVector& row : matrix )
        {
            nCols = std::max( nCols, static_cast<uint32_t>( row.size() ) );
            for ( const etrading::Variant& cell : row )
            {
                const etrading::ContainedTypeEnum type = cell.getType();
                if ( type != etrading::DOUBLE_VALUE && type != etrading::INTEGER_VALUE &&
                     type != etrading::BOOL_VALUE && type != etrading::EMPTY_VALUE )
                {
                    totalStringLength += cell.getValueAsString().size();
                }
            }
        }

        xloil::ExcelArrayBuilder builder( nRows, nCols, totalStringLength, true /* pad to 2D */ );
        for ( uint32_t r = 0; r < nRows; ++r )
        {
            for ( uint32_t c = 0; c < nCols; ++c )
            {
                builder( r, c ) = ( c < matrix[r].size() )
                    ? variantToExcel( matrix[r][c] )
                    : xloil::ExcelObj( xloil::CellError::NA );
            }
        }

        return builder.toExcelObj();
    }

    xloil::ExcelObj reshapeToSize( const xloil::ExcelObj& obj,
                                   uint32_t numRows,
                                   uint32_t numCols )
    {
        if ( numRows == 0 || numCols == 0 )
        {
            return xloil::ExcelObj( xloil::CellError::NA );
        }

        // Address the source by its own (row, column) position - the reshape is
        // a positional clip / pad, NOT a row-major reflow. Cell (r, c) of the
        // result is source (r, c) when that exists; rows or columns beyond the
        // source are blank-filled, and a source larger than the requested shape
        // is truncated.
        size_t srcRows = 1;
        size_t srcCols = 1;

        // A range: take its real dimensions. A single value: a 1x1 source.
        std::unique_ptr<xloil::ExcelArray> sourceArray;
        if ( obj.isType( xloil::ExcelType::Multi ) )
        {
            sourceArray.reset( new xloil::ExcelArray( obj, false ) );
            srcRows = sourceArray->nRows();
            srcCols = sourceArray->nCols();
        }

        auto at = [&]( size_t r, size_t c ) -> const xloil::ExcelObj&
        {
            if ( sourceArray )
            {
                return ( *sourceArray )( r, c );
            }
            return obj;
        };

        // Reserve string room for the cells that actually survive the clip.
        size_t totalStringLength = 0;
        for ( size_t r = 0; r < srcRows && r < numRows; ++r )
        {
            for ( size_t c = 0; c < srcCols && c < numCols; ++c )
            {
                const xloil::ExcelObj& cell = at( r, c );
                if ( cell.isType( xloil::ExcelType::Str ) )
                {
                    totalStringLength += cell.toString().size();
                }
            }
        }

        xloil::ExcelArrayBuilder builder( numRows, numCols, totalStringLength, true );

        for ( uint32_t r = 0; r < numRows; ++r )
        {
            for ( uint32_t c = 0; c < numCols; ++c )
            {
                if ( r < srcRows && c < srcCols )
                {
                    builder( r, c ) = at( r, c );
                }
                else
                {
                    builder( r, c ) = xloil::ExcelObj( std::wstring() );
                }
            }
        }

        return builder.toExcelObj();
    }

    // -------------------------------------------------------------------------
    //  AQObj handles - the instance counter
    // -------------------------------------------------------------------------

    std::string appendInstanceCounter( const std::string& objectName, bool updateCounter )
    {
        if ( !instanceCountNames_ )
        {
            return objectName;
        }

        std::lock_guard< std::mutex > lock( counterMutex_);

        const auto found = namesToCounter_.find( objectName );
        if ( found != namesToCounter_.end() )
        {
            if ( updateCounter )
            {
                // Wrap at COUNTER_WRAP, and never fall back to 0 once counting has started.
                found->second = std::max( 1, ( found->second + 1 ) % COUNTER_WRAP );
            }
        }
        else
        {
            // The first instance of a name counts from zero.
            namesToCounter_[objectName] = 0;
        }

        return objectName + counterDelimiter() + std::to_string( namesToCounter_[objectName] );
    }

    std::string appendInstanceCounter( const AQLString& objectName, bool updateCounter )
    {
        return appendInstanceCounter( std::string( objectName.getCString() ), updateCounter );
    }

    std::string getInstanceCounterAsString( const std::string& objectName )
    {
        std::lock_guard< std::mutex > lock( counterMutex_);

        const auto found = namesToCounter_.find( objectName );
        if ( found == namesToCounter_.end() )
        {
            return "";
        }

        return counterDelimiter() + std::to_string( found->second );
    }

    bool stopCountingName( const std::string& objectName )
    {
        std::lock_guard< std::mutex > lock( counterMutex_);

        const auto found = namesToCounter_.find( objectName );
        if ( found == namesToCounter_.end() )
        {
            return false;
        }

        namesToCounter_.erase( found );
        return true;
    }

    // Stop counting every name at once - the AQ_XLL-side counterpart of a
    // full aqObjectClearCache / aqObjectDeleteAll( <no type> ): once every
    // cached object is gone, the handle-name -> counter map would otherwise
    // keep growing with entries for names that no longer exist.
    void clearAllInstanceCounters()
    {
        std::lock_guard< std::mutex > lock( counterMutex_);
        namesToCounter_.clear();
    }

    std::string getNameWithoutCounter( const std::string& handle )
    {
        // The counter is the trailing "<delimiter><digits>", if present. Anything
        // else after the delimiter belongs to the name and is left alone.
        const std::string::size_type delimiterAt = handle.find_last_of( counterDelimiter() );
        if ( delimiterAt == std::string::npos || delimiterAt + 1 >= handle.size() )
        {
            return handle;
        }

        const std::string suffix = handle.substr( delimiterAt + 1 );
        const bool suffixIsCounter = std::all_of( suffix.begin(), suffix.end(),
                                                  []( unsigned char c ){ return std::isdigit( c ) != 0; } );

        return suffixIsCounter ? handle.substr( 0, delimiterAt ) : handle;
    }

    std::string getNameWithoutCounter( const xloil::ExcelObj& handle )
    {
        return getNameWithoutCounter( toNarrowString( handle ) );
    }

    std::vector< std::string > getNamesWithoutCounter( const xloil::ExcelObj& handles )
    {
        std::vector< std::string > names;

        if ( handles.isType( xloil::ExcelType::Multi ) )
        {
            xloil::ExcelArray array( handles );
            names.reserve( array.size() );
            for ( size_t row = 0; row < array.nRows(); ++row )
            {
                for ( size_t col = 0; col < array.nCols(); ++col )
                {
                    names.push_back( getNameWithoutCounter( array( row, col ) ) );
                }
            }
        }
        else
        {
            names.push_back( getNameWithoutCounter( handles ) );
        }

        return names;
    }

    std::string getExcelLocationAsString()
    {
        if ( !decorateNamesWithExcelAddress_ )
        {
            return "";
        }

        // CallerInfo asks Excel for the calling cell. It yields an empty address
        // when the caller is not a worksheet cell - a macro or a VBA call, say.
        const xloil::CallerInfo caller;
        const std::wstring address = caller.address( xloil::AddressStyle::A1 );
        if ( address.empty() )
        {
            return "";
        }

        const std::string upperAddress = etrading::trim_to_upper( std::string( address.begin(), address.end() ) );

        if ( !convertExcelAddressToUniqueID_ )
        {
            return upperAddress;
        }

        // A short, stable-within-this-session numeric ID standing in for the
        // full cell address - the "Showing Excel Location as UNIQUE ID" mode
        // (ported from the legacy convertExcelAddressToUniqueID switch).
        // std::hash is only guaranteed stable within one process run, which is
        // fine here: the whole point is a short handle, not a portable one.
        const size_t hashed = std::hash<std::string>()( upperAddress ) % 100000;
        return std::to_string( hashed );
    }

    std::string decorateWithExcelLocation( const std::string& objectName )
    {
        const std::string location = getExcelLocationAsString();
        if ( location.empty() )
        {
            return objectName;
        }
        return objectName + "@" + location;
    }

    bool allowAQObjUpdates( bool allowUpdate,
                            std::string& result,
                            const std::string& objectName,
                            const etrading::CachedObjectEnum objectType )
    {
        if ( !allowUpdate && etrading::Environment::defaultEnv().hasObject( objectName, objectType ) )
        {
            // The object already exists and the caller asked not to overwrite it,
            // so hand back the existing handle unchanged.
            result = objectName + getInstanceCounterAsString( objectName );
            return false;
        }

        return true;
    }

    // -------------------------------------------------------------------------
    //  Handle behaviour switches
    // -------------------------------------------------------------------------

    void setInstanceCountNames( bool on )              { instanceCountNames_ = on; }
    bool instanceCountNames()                          { return instanceCountNames_; }
    void setDecorateNamesWithExcelAddress( bool on )   { decorateNamesWithExcelAddress_ = on; }
    bool decorateNamesWithExcelAddress()               { return decorateNamesWithExcelAddress_; }
    void setConvertExcelAddressToUniqueID( bool on )   { convertExcelAddressToUniqueID_ = on; }
    bool convertExcelAddressToUniqueID()               { return convertExcelAddressToUniqueID_; }
}
