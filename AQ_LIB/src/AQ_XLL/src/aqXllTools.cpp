/*
 * @brief   Excel-side helpers for the AQ xlOil add-in. See aqXllTools.h.
 */

#include "aqXllTools.h"

#include <xloil/ExcelArray.h>
#include <xloil/ArrayBuilder.h>

#include <algorithm>
#include <mutex>
#include <unordered_map>

#include "CommonConstants.h"
#include "DateUtilities.h"
#include "AQLDateScheduleHelpers.h"
#include "Environment.h"
#include "AQLCoreAppError.h"

namespace aq_xll
{
    namespace
    {
        // Handle behaviour switches. Both default to the legacy add-in's settings.
        bool instanceCountNames_            = true;
        bool decorateNamesWithExcelAddress_ = false;

        // objectName -> current instance counter
        std::unordered_map< std::string, int > namesToCounter_;

        // The legacy map was unguarded. xlOil can register thread-safe worksheet
        // functions, so the map is guarded here.
        std::mutex counterMutex_;

        // The counter wraps at 100 so it never grows without bound, and never
        // returns to 0 once used, so a live handle is always visibly decorated.
        const int COUNTER_WRAP = 100;

        const std::string& counterDelimiter()
        {
            return etrading::AQOBJ_OBJECT_COUNTER_DELIMITER;
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

        // An error value anywhere is a hard failure - the same rule the legacy
        // add-in applied, so a #REF! in the input never silently becomes a date.
        for ( const xloil::ExcelObj* cell : cells )
        {
            if ( cell->isType( xloil::ExcelType::Err ) )
            {
                const std::string message = "#Error: Invalid Vector Input: " + nameOfVariable + " date input required";
                throw AQLCoreInvalidData( message.c_str(), __FILE__, __LINE__ );
            }
        }

        // Trailing blanks are dropped so a user can select a whole column.
        size_t count = cells.size();
        if ( skipTrailingBlanks )
        {
            while ( count > 0 && cells[count - 1]->isType( xloil::ExcelType::Missing | xloil::ExcelType::Nil ) )
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

        return etrading::trim_to_upper( std::string( address.begin(), address.end() ) );
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
}
