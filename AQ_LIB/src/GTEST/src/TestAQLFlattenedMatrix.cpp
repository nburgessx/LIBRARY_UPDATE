// Includes: This Library
#include "AQLCoreTemplateType.h"
#include "AQLCoreAppError.h"

// Include: Google Test Library
#include <gTest/gTest.h>

#include <vector>
#include <complex>
#include <sstream>

/*
 * @brief AQLFlattenedMatrix<T> is the shared base introduced for Phase 6.11 (matrix/table-type
 * consolidation) - generalises AQLNumericMatrix's proven flattened-storage design so
 * AQLIntMatrix/AQLComplexMatrix (this pilot) and, later, AQLBoolMatrix/AQLAnyMatrix can compose
 * it instead of each being a bare vector<vector<T>> typedef with zero methods. These tests
 * exercise the template directly through its two pilot aliases (AQLIntMatrix = int,
 * AQLComplexMatrix = complex<double>) rather than a synthetic instantiation, since those are the
 * two real consumers today (AQLMathDisplacedHestonTDP.h/.cpp).
 */

TEST( TestAQLFlattenedMatrix, UNIT_ConstructionIsZeroInitialized )
{
	AQLIntMatrix m( 2, 3 );
	EXPECT_EQ( 2u, m.row() );
	EXPECT_EQ( 3u, m.column() );
	EXPECT_EQ( 2u, m.size() );  // size() aliases row(), matching the old outer vector<vector<T>>::size()

	for ( std::size_t i = 0; i < m.row(); ++i )
		for ( std::size_t j = 0; j < m.column(); ++j )
			EXPECT_EQ( 0, m( i, j ) );
}

TEST( TestAQLFlattenedMatrix, UNIT_FillConstructorIdiom_MatchesOldVectorOfVectorsPattern )
{
	// The exact idiom AQLMathDisplacedHestonTDP.cpp uses: ComplexMatrix A(termSize, ComplexVector(GL_Size));
	ComplexVector rowTemplate( 4 );
	rowTemplate[0] = DoubleComplex( 1.0, 2.0 );

	AQLComplexMatrix m( 3, rowTemplate );
	ASSERT_EQ( 3u, m.row() );
	ASSERT_EQ( 4u, m.column() );
	// Every row is an independent copy of rowTemplate, not an alias of the same data.
	for ( std::size_t i = 0; i < m.row(); ++i )
		EXPECT_EQ( DoubleComplex( 1.0, 2.0 ), m( i, 0 ) );

	m( 1, 0 ) = DoubleComplex( 9.0, 9.0 );
	EXPECT_EQ( DoubleComplex( 1.0, 2.0 ), m( 0, 0 ) );  // row 0 unaffected by row 1's mutation
	EXPECT_EQ( DoubleComplex( 9.0, 9.0 ), m( 1, 0 ) );
}

TEST( TestAQLFlattenedMatrix, UNIT_BraceInitConstructor )
{
	AQLIntMatrix m{ { 1, 2, 3 }, { 4, 5, 6 } };
	ASSERT_EQ( 2u, m.row() );
	ASSERT_EQ( 3u, m.column() );
	EXPECT_EQ( 1, m( 0, 0 ) );
	EXPECT_EQ( 6, m( 1, 2 ) );
}

TEST( TestAQLFlattenedMatrix, UNIT_BraceInitConstructor_RaggedRowsThrows )
{
	EXPECT_THROW( ( AQLIntMatrix{ { 1, 2 }, { 3 } } ), AQLCoreNumericalError );
}

TEST( TestAQLFlattenedMatrix, UNIT_CallOperator_ReadWriteAndBoundsCheck )
{
	AQLIntMatrix m( 2, 2 );
	m( 0, 0 ) = 1;
	m( 1, 1 ) = 4;
	EXPECT_EQ( 1, m( 0, 0 ) );
	EXPECT_EQ( 4, m( 1, 1 ) );

	EXPECT_THROW( m( 2, 0 ), AQLCoreNumericalError );
	EXPECT_THROW( m( 0, 2 ) = 1, AQLCoreNumericalError );
}

TEST( TestAQLFlattenedMatrix, UNIT_IndexOperator_RowViewReadWrite )
{
	// The [i][k] idiom AQLMathDisplacedHestonTDP.cpp's calibration loops use directly (e.g.
	// `A[i][k] += A_TDP(...)`), unchecked exactly like the old vector<vector<T>>::operator[] -
	// not a regression, just preserved.
	AQLComplexMatrix m( 2, 2 );
	m[0][0] = DoubleComplex( 1.0, 0.0 );
	m[0][1] = DoubleComplex( 2.0, 0.0 );
	m[0][0] += DoubleComplex( 0.5, 0.5 );

	EXPECT_EQ( DoubleComplex( 1.5, 0.5 ), m[0][0] );
	EXPECT_EQ( DoubleComplex( 2.0, 0.0 ), m[0][1] );

	const AQLComplexMatrix& constRef = m;
	EXPECT_EQ( 2u, constRef[0].size() );  // row-view .size() == column count
}

TEST( TestAQLFlattenedMatrix, UNIT_IndexOperator_WriteDoesNotAliasACopy )
{
	AQLIntMatrix original{ { 1, 2 }, { 3, 4 } };
	AQLIntMatrix copy( original );

	copy[0][0] = 99;
	EXPECT_EQ( 99, copy[0][0] );
	EXPECT_EQ( 1, original[0][0] );  // COW detach on the non-const operator[] protects the source
}

TEST( TestAQLFlattenedMatrix, UNIT_GetRowAndGetColumn )
{
	AQLIntMatrix m{ { 1, 2, 3 }, { 4, 5, 6 } };

	const std::vector<int> row0 = m.getRow( 0 );
	ASSERT_EQ( 3u, row0.size() );
	EXPECT_EQ( 1, row0[0] ); EXPECT_EQ( 2, row0[1] ); EXPECT_EQ( 3, row0[2] );

	const std::vector<int> col2 = m.getColumn( 2 );
	ASSERT_EQ( 2u, col2.size() );
	EXPECT_EQ( 3, col2[0] ); EXPECT_EQ( 6, col2[1] );

	EXPECT_THROW( m.getRow( 2 ), AQLCoreNumericalError );
	EXPECT_THROW( m.getColumn( 3 ), AQLCoreNumericalError );
}

TEST( TestAQLFlattenedMatrix, UNIT_Resize_GrowPreservesOverlapAndZeroFillsTheRest )
{
	AQLIntMatrix m{ { 1, 2 }, { 3, 4 } };
	m.resize( 3, 3 );

	ASSERT_EQ( 3u, m.row() );
	ASSERT_EQ( 3u, m.column() );
	EXPECT_EQ( 1, m( 0, 0 ) ); EXPECT_EQ( 2, m( 0, 1 ) );
	EXPECT_EQ( 3, m( 1, 0 ) ); EXPECT_EQ( 4, m( 1, 1 ) );
	EXPECT_EQ( 0, m( 2, 2 ) );
}

TEST( TestAQLFlattenedMatrix, UNIT_Transpose )
{
	AQLIntMatrix m{ { 1, 2, 3 }, { 4, 5, 6 } };
	AQLIntMatrix mt = m.transpose();

	ASSERT_EQ( 3u, mt.row() );
	ASSERT_EQ( 2u, mt.column() );
	EXPECT_EQ( 1, mt( 0, 0 ) ); EXPECT_EQ( 4, mt( 0, 1 ) );
	EXPECT_EQ( 2, mt( 1, 0 ) ); EXPECT_EQ( 5, mt( 1, 1 ) );
	EXPECT_EQ( 3, mt( 2, 0 ) ); EXPECT_EQ( 6, mt( 2, 1 ) );

	// original unchanged
	EXPECT_EQ( 1, m( 0, 0 ) );

	// double transpose round-trips
	AQLIntMatrix mtt = mt.transpose();
	EXPECT_TRUE( m == mtt );
}

TEST( TestAQLFlattenedMatrix, UNIT_EqualityOperators )
{
	AQLIntMatrix a{ { 1, 2 }, { 3, 4 } };
	AQLIntMatrix b{ { 1, 2 }, { 3, 4 } };
	AQLIntMatrix c{ { 1, 2 }, { 3, 5 } };

	EXPECT_TRUE( a == b );
	EXPECT_FALSE( a != b );
	EXPECT_TRUE( a != c );
}

TEST( TestAQLFlattenedMatrix, UNIT_Equals_Tolerance_Complex )
{
	AQLComplexMatrix a{ { DoubleComplex( 1.0, 1.0 ) } };
	AQLComplexMatrix nearlyEqual{ { DoubleComplex( 1.0 + 1e-9, 1.0 ) } };
	AQLComplexMatrix notEqual{ { DoubleComplex( 1.5, 1.0 ) } };

	EXPECT_FALSE( a == nearlyEqual );
	EXPECT_TRUE( a.equals( nearlyEqual, 1e-6 ) );
	EXPECT_FALSE( a.equals( notEqual, 1e-6 ) );
}

TEST( TestAQLFlattenedMatrix, UNIT_MoveConstructorAndAssignment )
{
	AQLIntMatrix source{ { 1, 2 }, { 3, 4 } };
	AQLIntMatrix moved( std::move( source ) );
	EXPECT_EQ( 1, moved( 0, 0 ) );
	EXPECT_EQ( 4, moved( 1, 1 ) );

	AQLIntMatrix target;
	target = std::move( moved );
	EXPECT_EQ( 4, target( 1, 1 ) );
}

TEST( TestAQLFlattenedMatrix, UNIT_StreamOperator )
{
	AQLIntMatrix m{ { 1, 2 }, { 3, 4 } };
	std::ostringstream oss;
	oss << m;

	const std::string text = oss.str();
	EXPECT_NE( std::string::npos, text.find( "1" ) );
	EXPECT_NE( std::string::npos, text.find( "4" ) );
}
