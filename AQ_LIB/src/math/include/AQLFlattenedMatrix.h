/*! @file
    @brief Shared flattened-storage matrix template - the base every consolidated matrix type in
    Phase 6.11 composes (contiguous row-major storage, copy-on-write, bounds-checked element
    access, transpose, equality). Generalises the design proved out by AQLNumericMatrix (formerly
    AQLMatrix) - see AQLNumericMatrix.h for the full rationale of every design choice reused here
    (one contiguous std::vector<T> instead of a row-pointer array, shared_ptr-based COW because
    the row-view/transpose/copy shape is genuinely copy-heavy, move semantics so the compiler can
    skip even the refcount bump in a "build a temporary, return it" pattern).

    Arithmetic (+, -, *, decompositions) deliberately does NOT live here - AQLNumericMatrix keeps
    its own rich arithmetic surface un-migrated for now (a separate, larger step), and most other
    consolidated types (AQLIntMatrix, AQLComplexMatrix, and eventually AQLBoolMatrix/AQLAnyMatrix)
    are plain data containers with no matrix-arithmetic call sites today. Add arithmetic to a
    specific type's own thin wrapper if and when a real caller needs it - do not add it here
    speculatively.
*/
#pragma once

#include <memory>
#include <vector>
#include <initializer_list>
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <ostream>
#include "AQLCoreAppError.h"

/*! @brief Unchecked, non-owning view over one row - a thin (pointer, length) pair, not a bare
    T*, because several of the consolidated typedef-family call sites this is designed to absorb
    genuinely rely on `.size()` on a row (mirroring the outer vector<vector<T>>::operator[]
    contract they migrate from) and not just element indexing. Never outlives the row it points
    into - the usual raw-pointer/reference caveat, same as AQLNumericMatrix's own operator[].
*/
template<typename T>
class AQLFlattenedMatrixRowView
{
public:
    AQLFlattenedMatrixRowView( T* data, std::size_t size ) : data_( data ), size_( size ) {}

    T&          operator[]( std::size_t j )        { return data_[j]; }
    const T&    operator[]( std::size_t j ) const  { return data_[j]; }
    std::size_t size( void ) const                 { return size_; }
    T*          begin( void )                      { return data_; }
    T*          end( void )                        { return data_ + size_; }
    const T*    begin( void ) const                { return data_; }
    const T*    end( void ) const                  { return data_ + size_; }

private:
    T*          data_;
    std::size_t size_;
};

template<typename T>
class AQLFlattenedMatrixConstRowView
{
public:
    AQLFlattenedMatrixConstRowView( const T* data, std::size_t size ) : data_( data ), size_( size ) {}

    const T&    operator[]( std::size_t j ) const  { return data_[j]; }
    std::size_t size( void ) const                 { return size_; }
    const T*    begin( void ) const                { return data_; }
    const T*    end( void ) const                  { return data_ + size_; }

private:
    const T*    data_;
    std::size_t size_;
};

template<typename T>
class AQLFlattenedMatrix
{
public:
    AQLFlattenedMatrix( void ) : pData_( std::make_shared<Data>( 0, 0 ) ) {}

    AQLFlattenedMatrix( std::size_t rows, std::size_t cols ) :
        pData_( std::make_shared<Data>( rows, cols ) )
    {}

    // Compatibility with the classic std::vector<std::vector<T>>(rows, std::vector<T>(cols))
    // fill-construction idiom (e.g. `ComplexMatrix A(termSize, ComplexVector(GL_Size));`) - lets
    // every existing call site built on that idiom migrate onto this type with a pure type-name
    // rename and nothing else. Every row is a copy of rowTemplate.
    AQLFlattenedMatrix( std::size_t rows, const std::vector<T>& rowTemplate ) :
        pData_( std::make_shared<Data>( rows, rowTemplate.size() ) )
    {
        for ( std::size_t i = 0; i < rows; ++i )
        {
            std::copy( rowTemplate.begin(), rowTemplate.end(), ( *pData_ )[i] );
        }
    }

    // Brace-init, e.g. AQLIntMatrix m{ {1,2}, {3,4} } - same ergonomic purpose and the same
    // ragged-row rejection as AQLNumericMatrix's equivalent constructor.
    AQLFlattenedMatrix( std::initializer_list<std::initializer_list<T>> rows )
    {
        const std::size_t rowCount = rows.size();
        const std::size_t colCount = rowCount == 0 ? 0 : rows.begin()->size();
        pData_ = std::make_shared<Data>( rowCount, colCount );
        std::size_t i = 0;
        for ( const auto& oneRow : rows )
        {
            if ( oneRow.size() != colCount )
            {
                throw AQLCoreNumericalError(
                    "AQLFlattenedMatrix{{...}}: every row must be the same length", __FILE__, __LINE__ );
            }
            std::copy( oneRow.begin(), oneRow.end(), ( *pData_ )[i] );
            ++i;
        }
    }

    AQLFlattenedMatrix( const AQLFlattenedMatrix& other ) : pData_( other.pData_ ) {}
    AQLFlattenedMatrix( AQLFlattenedMatrix&& other ) noexcept : pData_( std::move( other.pData_ ) ) {}

    AQLFlattenedMatrix& operator=( const AQLFlattenedMatrix& other )
    {
        pData_ = other.pData_;
        return *this;
    }

    AQLFlattenedMatrix& operator=( AQLFlattenedMatrix&& other ) noexcept
    {
        if ( this != &other ) pData_ = std::move( other.pData_ );
        return *this;
    }

    std::size_t row( void ) const      { return pData_ ? pData_->row : 0; }
    std::size_t column( void ) const   { return pData_ ? pData_->col : 0; }

    // Alias for row() - drop-in compatibility with the old vector<vector<T>> convention, where
    // the OUTER vector's .size() meant row count (e.g. `sgns.size()`).
    std::size_t size( void ) const     { return row(); }

    bool isWithin( std::size_t r, std::size_t c ) const { return r < row() && c < column(); }

    // Bounds-checked, detaching read/write element access - the AQLNumericMatrix::operator()
    // pattern.
    T& operator()( std::size_t i, std::size_t j )
    {
        if ( !isWithin( i, j ) )
            throw AQLCoreNumericalError( "Boundary Error", __FILE__, __LINE__ );
        makeUnShared();
        return ( *pData_ )[i][j];
    }

    const T& operator()( std::size_t i, std::size_t j ) const
    {
        if ( !isWithin( i, j ) )
            throw AQLCoreNumericalError( "Boundary Error", __FILE__, __LINE__ );
        return ( *pData_ )[i][j];
    }

    // Unchecked row access, matching the old raw vector<vector<T>>::operator[] contract exactly
    // (that was unchecked too - this is not a regression). Detaches once per row, not once per
    // element - the existing call sites this absorbs write a whole row's worth of elements per
    // outer-index access, not a single element, so this is the right granularity; it is not the
    // per-element makeUnShared() cost AQLNumericMatrix's decomposition kernels were fixed to
    // avoid (see AQLNumericMatrix.cpp's ludcmp/svdcmp/tred2/tqli).
    AQLFlattenedMatrixRowView<T> operator[]( std::size_t i )
    {
        makeUnShared();
        return AQLFlattenedMatrixRowView<T>( ( *pData_ )[i], column() );
    }

    AQLFlattenedMatrixConstRowView<T> operator[]( std::size_t i ) const
    {
        return AQLFlattenedMatrixConstRowView<T>( ( *pData_ )[i], column() );
    }

    std::vector<T> getRow( std::size_t i ) const
    {
        if ( !isWithin( i, 0 ) )
            throw AQLCoreNumericalError( "Boundary Error", __FILE__, __LINE__ );
        const T* rowPtr = ( *pData_ )[i];
        return std::vector<T>( rowPtr, rowPtr + column() );
    }

    std::vector<T> getColumn( std::size_t j ) const
    {
        if ( !isWithin( 0, j ) )
            throw AQLCoreNumericalError( "Boundary Error", __FILE__, __LINE__ );
        std::vector<T> result( row() );
        for ( std::size_t i = 0; i < row(); ++i )
        {
            result[i] = ( *pData_ )[i][j];
        }
        return result;
    }

    void resize( std::size_t newRows, std::size_t newCols )
    {
        makeUnShared();
        pData_->resize( newRows, newCols );
    }

    AQLFlattenedMatrix transpose( void ) const
    {
        AQLFlattenedMatrix ret( column(), row() );
        for ( std::size_t i = 0; i < row(); ++i )
        {
            for ( std::size_t j = 0; j < column(); ++j )
            {
                ( *ret.pData_ )[j][i] = ( *pData_ )[i][j];
            }
        }
        return ret;
    }

    // Exact comparison. See AQLNumericMatrix::operator== for the same "genuinely wanted for
    // identity/reference checks, but after any per-element computation use equals() instead"
    // caveat - it applies here identically for T=double/complex<double>, less so for T=int
    // (exact int comparison is usually exactly what is wanted).
    bool operator==( const AQLFlattenedMatrix& other ) const
    {
        if ( row() != other.row() || column() != other.column() ) return false;
        for ( std::size_t i = 0; i < row(); ++i )
            for ( std::size_t j = 0; j < column(); ++j )
                if ( !( ( *pData_ )[i][j] == ( *other.pData_ )[i][j] ) ) return false;
        return true;
    }

    bool operator!=( const AQLFlattenedMatrix& other ) const { return !( *this == other ); }

    // Tolerance-based comparison, via std::abs(a-b) <= tolerance - works uniformly for
    // T=int/double/complex<double> (std::abs has an overload for each via ADL).
    bool equals( const AQLFlattenedMatrix& other, double tolerance ) const
    {
        if ( row() != other.row() || column() != other.column() ) return false;
        for ( std::size_t i = 0; i < row(); ++i )
            for ( std::size_t j = 0; j < column(); ++j )
                if ( std::abs( ( *pData_ )[i][j] - ( *other.pData_ )[i][j] ) > tolerance ) return false;
        return true;
    }

private:
    class Data
    {
    public:
        Data( std::size_t rows, std::size_t cols ) : row( rows ), col( cols )
        {
            if ( rows == 0 || cols == 0 ) { row = col = 0; return; }
            data_.resize( rows * cols );
        }

        void resize( std::size_t newRows, std::size_t newCols )
        {
            if ( newRows == 0 || newCols == 0 )
            {
                data_.clear();
                row = col = 0;
                return;
            }
            std::vector<T> newData( newRows * newCols );
            const std::size_t rowsToCopy = ( newRows < row ) ? newRows : row;
            const std::size_t colsToCopy = ( newCols < col ) ? newCols : col;
            for ( std::size_t i = 0; i < rowsToCopy; ++i )
            {
                const T* oldRow = data_.data() + i * col;
                T*       newRow = newData.data() + i * newCols;
                std::copy( oldRow, oldRow + colsToCopy, newRow );
            }
            data_ = std::move( newData );
            row = newRows;
            col = newCols;
        }

        T*       operator[]( std::size_t n )       { return data_.data() + n * col; }
        const T* operator[]( std::size_t n ) const { return data_.data() + n * col; }

        std::size_t row;
        std::size_t col;

    private:
        std::vector<T> data_;
    };

    void makeUnShared( void ) const
    {
        if ( !pData_ || pData_.use_count() == 1 ) return;
        std::shared_ptr<Data> shared = pData_;
        pData_ = std::make_shared<Data>( shared->row, shared->col );
        for ( std::size_t i = 0; i < shared->row; ++i )
        {
            std::copy( ( *shared )[i], ( *shared )[i] + shared->col, ( *pData_ )[i] );
        }
    }

    mutable std::shared_ptr<Data> pData_;
};

template<typename T>
std::ostream& operator<<( std::ostream& os, const AQLFlattenedMatrix<T>& m )
{
    for ( std::size_t i = 0; i < m.row(); ++i )
    {
        for ( std::size_t j = 0; j < m.column(); ++j )
        {
            os << m( i, j );
            if ( j + 1 < m.column() ) os << '\t';
        }
        os << '\n';
    }
    return os;
}
