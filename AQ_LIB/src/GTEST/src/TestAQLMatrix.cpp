// Includes: This Library
#include "AQLMatrix.h"
#include "AQLCoreAppError.h"

// Include: Google Test Library
#include <gTest/gTest.h>

#include <vector>

/*
 * @brief These tests exercise AQLMatrix - construction, element/row/column access, scalar and
 * dot-product operations, transpose, resize and move semantics - after the internal storage was
 * flattened from a double** (row-pointer array + one allocation per row) to a single contiguous,
 * row-major std::vector<double>, and getRow()/getColumn()/dotRow()/dotCol() were added.
 *
 * The flattening was designed to be a pure internals change (operator[] keeps returning a
 * double / const double* row pointer either way), so these tests are mostly about locking down that
 * the externally-visible behaviour is unchanged, plus covering what is genuinely new: the
 * zero-initialization guarantee, getRow()/getColumn()/dotRow()/dotCol(), and move semantics.
 */

TEST( TestAQLMatrix, UNIT_ConstructionIsZeroInitialized )
{
	// The old new double[row*col] left elements uninitialized; std::vector value-initializes them.
	// This is a real behaviour change (a deliberate fix, not just carried forward) worth locking
	// down explicitly.
	AQLMatrix m( 3, 4 );
	EXPECT_EQ( 3u, m.row() );
	EXPECT_EQ( 4u, m.column() );

	for ( unsigned int i = 0; i < m.row(); ++i )
	{
		for ( unsigned int j = 0; j < m.column(); ++j )
		{
			EXPECT_EQ( 0.0, m.getValue( i, j ) );
		}
	}
}

TEST( TestAQLMatrix, UNIT_ElementAccess )
{
	AQLMatrix m( 3, 4 );

	double val = 1.0;
	for ( unsigned int i = 0; i < m.row(); ++i )
	{
		for ( unsigned int j = 0; j < m.column(); ++j )
		{
			m.setValue( i, j, val );
			val += 1.0;
		}
	}

	EXPECT_EQ( 1.0,  m.getValue( 0, 0 ) );
	EXPECT_EQ( 12.0, m.getValue( 2, 3 ) );
	EXPECT_EQ( 1.0,  m[0][0] );
	EXPECT_EQ( 12.0, m[2][3] );
}

TEST( TestAQLMatrix, UNIT_ConstructionFromDoubleMatrix )
{
	DoubleMatrix mat = { { 1.0, 2.0, 3.0 },
	                     { 4.0, 5.0, 6.0 } };

	AQLMatrix m( mat );
	EXPECT_EQ( 2u, m.row() );
	EXPECT_EQ( 3u, m.column() );
	EXPECT_EQ( 1.0, m.getValue( 0, 0 ) );
	EXPECT_EQ( 2.0, m.getValue( 0, 1 ) );
	EXPECT_EQ( 6.0, m.getValue( 1, 2 ) );
}

TEST( TestAQLMatrix, UNIT_GetRowAndGetColumn )
{
	// [1 2 3]
	// [4 5 6]
	DoubleMatrix mat = { { 1.0, 2.0, 3.0 },
	                     { 4.0, 5.0, 6.0 } };
	AQLMatrix m( mat );

	const std::vector<double> row0 = m.getRow( 0 );
	const std::vector<double> row1 = m.getRow( 1 );
	ASSERT_EQ( 3u, row0.size() );
	EXPECT_EQ( 1.0, row0[0] ); EXPECT_EQ( 2.0, row0[1] ); EXPECT_EQ( 3.0, row0[2] );
	ASSERT_EQ( 3u, row1.size() );
	EXPECT_EQ( 4.0, row1[0] ); EXPECT_EQ( 5.0, row1[1] ); EXPECT_EQ( 6.0, row1[2] );

	const std::vector<double> col0 = m.getColumn( 0 );
	const std::vector<double> col2 = m.getColumn( 2 );
	ASSERT_EQ( 2u, col0.size() );
	EXPECT_EQ( 1.0, col0[0] ); EXPECT_EQ( 4.0, col0[1] );
	ASSERT_EQ( 2u, col2.size() );
	EXPECT_EQ( 3.0, col2[0] ); EXPECT_EQ( 6.0, col2[1] );
}

TEST( TestAQLMatrix, UNIT_GetRowAndGetColumn_OutOfRangeThrows )
{
	AQLMatrix m( 2, 3 );
	EXPECT_THROW( m.getRow( 2 ), AQLCoreNumericalError );
	EXPECT_THROW( m.getColumn( 3 ), AQLCoreNumericalError );
}

TEST( TestAQLMatrix, UNIT_DotRowAndDotCol )
{
	// [1 2 3]
	// [4 5 6]
	DoubleMatrix mat = { { 1.0, 2.0, 3.0 },
	                     { 4.0, 5.0, 6.0 } };
	AQLMatrix m( mat );

	const std::vector<double> weights = { 1.0, 10.0, 100.0 };
	// row 0 . weights = 1*1 + 2*10 + 3*100 = 321
	EXPECT_EQ( 321.0, m.dotRow( 0, weights ) );
	// row 1 . weights = 4*1 + 5*10 + 6*100 = 654
	EXPECT_EQ( 654.0, m.dotRow( 1, weights ) );

	const std::vector<double> rowWeights = { 1.0, 10.0 };
	// col 0 . rowWeights = 1*1 + 4*10 = 41
	EXPECT_EQ( 41.0, m.dotCol( 0, rowWeights ) );
	// col 2 . rowWeights = 3*1 + 6*10 = 63
	EXPECT_EQ( 63.0, m.dotCol( 2, rowWeights ) );
}

TEST( TestAQLMatrix, UNIT_DotRowAndDotCol_SizeMismatchThrows )
{
	AQLMatrix m( 2, 3 );
	const std::vector<double> wrongSizeForRow = { 1.0, 2.0 };       // column() is 3
	const std::vector<double> wrongSizeForCol = { 1.0, 2.0, 3.0 };  // row() is 2

	EXPECT_THROW( m.dotRow( 0, wrongSizeForRow ), AQLCoreNumericalError );
	EXPECT_THROW( m.dotCol( 0, wrongSizeForCol ), AQLCoreNumericalError );
}

/*
 * @brief Mirrors the classic "numerical integration against a symmetric z-score grid cancels out to
 * (approximately) zero, an asymmetric one does not" pattern used to validate dot-product-based
 * numerical integration (e.g. Gaussian copula survival-probability calculations).
 */
TEST( TestAQLMatrix, UNIT_DotRow_NumericalIntegrationPattern )
{
	const double timeStep = 1.0;
	const std::vector<double> zScores = { -3.0, -2.0, -1.0, 0.0, 1.0, 2.0, 3.0 };

	// Every row is symmetric about its centre, so the dot product with an antisymmetric z-score
	// grid cancels to zero.
	DoubleMatrix symmetric = { { 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5 },
	                          { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 } };
	AQLMatrix survivalPDF( symmetric );

	EXPECT_NEAR( 0.0, survivalPDF.dotRow( 0, zScores ) * timeStep, 1e-12 );
	EXPECT_NEAR( 0.0, survivalPDF.dotRow( 1, zScores ) * timeStep, 1e-12 );

	// Perturb the first and last entries of each row so it is no longer symmetric - the cancellation
	// breaks and the result is exactly the sum of what was added/removed at the ends.
	DoubleMatrix asymmetric = { { 1.0, 0.5, 0.5, 0.5, 0.5, 0.5, 0.0 },
	                           { 2.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.0 } };
	AQLMatrix asymmetricPDF( asymmetric );

	// row 0: symmetric part cancels to 0, remaining = (1.0-0.5)*(-3) + (0.0-0.5)*3 = -1.5 - 1.5 = -3
	EXPECT_NEAR( -3.0, asymmetricPDF.dotRow( 0, zScores ) * timeStep, 1e-12 );
	// row 1: (2.0-1.0)*(-3) + (0.0-1.0)*3 = -3 - 3 = -6
	EXPECT_NEAR( -6.0, asymmetricPDF.dotRow( 1, zScores ) * timeStep, 1e-12 );
}

TEST( TestAQLMatrix, UNIT_ScalarMultiply )
{
	DoubleMatrix mat = { { 1.0, 2.0 },
	                     { 3.0, 4.0 } };
	AQLMatrix m( mat );

	m *= 2.0;
	EXPECT_EQ( 2.0, m.getValue( 0, 0 ) );
	EXPECT_EQ( 4.0, m.getValue( 0, 1 ) );
	EXPECT_EQ( 6.0, m.getValue( 1, 0 ) );
	EXPECT_EQ( 8.0, m.getValue( 1, 1 ) );

	AQLMatrix scaled = m * 0.5;
	EXPECT_EQ( 1.0, scaled.getValue( 0, 0 ) );
	EXPECT_EQ( 4.0, scaled.getValue( 1, 1 ) );
	// m itself is unchanged by operator* (only operator*= mutates)
	EXPECT_EQ( 2.0, m.getValue( 0, 0 ) );
}

TEST( TestAQLMatrix, UNIT_ClearValuesAndSetValueAll )
{
	DoubleMatrix mat = { { 1.0, 2.0 },
	                     { 3.0, 4.0 } };
	AQLMatrix m( mat );

	m.setValue( 9.0 );
	for ( unsigned int i = 0; i < m.row(); ++i )
		for ( unsigned int j = 0; j < m.column(); ++j )
			EXPECT_EQ( 9.0, m.getValue( i, j ) );

	m.clearValues();
	for ( unsigned int i = 0; i < m.row(); ++i )
		for ( unsigned int j = 0; j < m.column(); ++j )
			EXPECT_EQ( 0.0, m.getValue( i, j ) );
}

TEST( TestAQLMatrix, UNIT_IdentityMatrix )
{
	AQLMatrix m( 3, 3 );
	m.setValue( 5.0 );      // fill with something non-zero first
	m.IdentityMatrix();

	for ( unsigned int i = 0; i < m.row(); ++i )
	{
		for ( unsigned int j = 0; j < m.column(); ++j )
		{
			EXPECT_EQ( i == j ? 1.0 : 0.0, m.getValue( i, j ) );
		}
	}
}

TEST( TestAQLMatrix, UNIT_Transpose )
{
	// [1 2 3]
	// [4 5 6]
	DoubleMatrix mat = { { 1.0, 2.0, 3.0 },
	                     { 4.0, 5.0, 6.0 } };
	AQLMatrix m( mat );

	AQLMatrix mt = m.transpose();

	// original unchanged
	EXPECT_EQ( 2u, m.row() );
	EXPECT_EQ( 3u, m.column() );
	EXPECT_EQ( 1.0, m.getValue( 0, 0 ) );

	// transposed shape and values:
	// [1 4]
	// [2 5]
	// [3 6]
	ASSERT_EQ( 3u, mt.row() );
	ASSERT_EQ( 2u, mt.column() );
	EXPECT_EQ( 1.0, mt.getValue( 0, 0 ) ); EXPECT_EQ( 4.0, mt.getValue( 0, 1 ) );
	EXPECT_EQ( 2.0, mt.getValue( 1, 0 ) ); EXPECT_EQ( 5.0, mt.getValue( 1, 1 ) );
	EXPECT_EQ( 3.0, mt.getValue( 2, 0 ) ); EXPECT_EQ( 6.0, mt.getValue( 2, 1 ) );

	// double transpose round-trips back to the original
	AQLMatrix mtt = mt.transpose();
	ASSERT_EQ( m.row(), mtt.row() );
	ASSERT_EQ( m.column(), mtt.column() );
	for ( unsigned int i = 0; i < m.row(); ++i )
		for ( unsigned int j = 0; j < m.column(); ++j )
			EXPECT_EQ( m.getValue( i, j ), mtt.getValue( i, j ) );
}

TEST( TestAQLMatrix, UNIT_TransposeAboveOpenMPThreshold )
{
	// Exercises the >64-row branch of transpose()'s `#pragma omp parallel for if(...)` guard (see
	// OPENMP_SIZE_THRESHOLD in AQLMatrix.cpp) - correctness must be identical whichever branch runs,
	// with or without OpenMPSupport actually enabled for the compiler that built this.
	const unsigned int n = 100;
	AQLMatrix m( n, 3 );
	for ( unsigned int i = 0; i < n; ++i )
	{
		m.setValue( i, 0, static_cast<double>( i ) );
		m.setValue( i, 1, static_cast<double>( i ) * 2.0 );
		m.setValue( i, 2, static_cast<double>( i ) * 3.0 );
	}

	AQLMatrix mt = m.transpose();
	ASSERT_EQ( 3u, mt.row() );
	ASSERT_EQ( n, mt.column() );
	for ( unsigned int i = 0; i < n; ++i )
	{
		EXPECT_EQ( m.getValue( i, 0 ), mt.getValue( 0, i ) );
		EXPECT_EQ( m.getValue( i, 1 ), mt.getValue( 1, i ) );
		EXPECT_EQ( m.getValue( i, 2 ), mt.getValue( 2, i ) );
	}
}

TEST( TestAQLMatrix, UNIT_Resize_GrowPreservesOverlapAndZeroFillsTheRest )
{
	// [1 2]
	// [3 4]
	DoubleMatrix mat = { { 1.0, 2.0 },
	                     { 3.0, 4.0 } };
	AQLMatrix m( mat );

	m.resize( 3, 3 );
	ASSERT_EQ( 3u, m.row() );
	ASSERT_EQ( 3u, m.column() );

	// overlapping top-left 2x2 preserved
	EXPECT_EQ( 1.0, m.getValue( 0, 0 ) );
	EXPECT_EQ( 2.0, m.getValue( 0, 1 ) );
	EXPECT_EQ( 3.0, m.getValue( 1, 0 ) );
	EXPECT_EQ( 4.0, m.getValue( 1, 1 ) );

	// everything newly added is zero-filled
	EXPECT_EQ( 0.0, m.getValue( 0, 2 ) );
	EXPECT_EQ( 0.0, m.getValue( 1, 2 ) );
	EXPECT_EQ( 0.0, m.getValue( 2, 0 ) );
	EXPECT_EQ( 0.0, m.getValue( 2, 1 ) );
	EXPECT_EQ( 0.0, m.getValue( 2, 2 ) );
}

TEST( TestAQLMatrix, UNIT_Resize_ShrinkKeepsOnlyTheOverlap )
{
	DoubleMatrix mat = { { 1.0, 2.0, 3.0 },
	                     { 4.0, 5.0, 6.0 },
	                     { 7.0, 8.0, 9.0 } };
	AQLMatrix m( mat );

	m.resize( 2, 2 );
	ASSERT_EQ( 2u, m.row() );
	ASSERT_EQ( 2u, m.column() );
	EXPECT_EQ( 1.0, m.getValue( 0, 0 ) );
	EXPECT_EQ( 2.0, m.getValue( 0, 1 ) );
	EXPECT_EQ( 4.0, m.getValue( 1, 0 ) );
	EXPECT_EQ( 5.0, m.getValue( 1, 1 ) );
}

TEST( TestAQLMatrix, UNIT_MoveConstructor )
{
	DoubleMatrix mat = { { 1.0, 2.0 },
	                     { 3.0, 4.0 } };
	AQLMatrix source( mat );

	AQLMatrix moved( std::move( source ) );
	EXPECT_EQ( 2u, moved.row() );
	EXPECT_EQ( 2u, moved.column() );
	EXPECT_EQ( 1.0, moved.getValue( 0, 0 ) );
	EXPECT_EQ( 4.0, moved.getValue( 1, 1 ) );
}

TEST( TestAQLMatrix, UNIT_MoveAssignment )
{
	DoubleMatrix mat = { { 1.0, 2.0 },
	                     { 3.0, 4.0 } };
	AQLMatrix source( mat );
	AQLMatrix target( 1, 1 );

	target = std::move( source );
	EXPECT_EQ( 2u, target.row() );
	EXPECT_EQ( 2u, target.column() );
	EXPECT_EQ( 1.0, target.getValue( 0, 0 ) );
	EXPECT_EQ( 4.0, target.getValue( 1, 1 ) );
}

TEST( TestAQLMatrix, UNIT_CopyIsIndependentOfSource )
{
	// AQLMatrix keeps shared_ptr-based copy-on-write sharing (unlike AQLCalendar/AQLDataInstance,
	// which dropped it) because arithmetic operators return by value pervasively - a copy still
	// needs to behave like an independent value from the caller's point of view, i.e. mutating the
	// copy must not be visible through the original.
	DoubleMatrix mat = { { 1.0, 2.0 },
	                     { 3.0, 4.0 } };
	AQLMatrix original( mat );
	AQLMatrix copy( original );

	copy.setValue( 0, 0, 99.0 );
	EXPECT_EQ( 99.0, copy.getValue( 0, 0 ) );
	EXPECT_EQ( 1.0, original.getValue( 0, 0 ) );  // unaffected by the copy's mutation
}
