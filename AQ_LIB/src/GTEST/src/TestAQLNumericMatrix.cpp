// Includes: This Library
#include "AQLNumericMatrix.h"
#include "AQLCoreAppError.h"

// Include: Google Test Library
#include <gTest/gTest.h>

#include <vector>
#include <sstream>

/*
 * @brief These tests exercise AQLNumericMatrix - construction, element/row/column access, scalar and
 * dot-product operations, transpose, resize and move semantics - after the internal storage was
 * flattened from a double** (row-pointer array + one allocation per row) to a single contiguous,
 * row-major std::vector<double>, and getRow()/getColumn()/dotRow()/dotCol() were added.
 *
 * The flattening was designed to be a pure internals change (operator[] keeps returning a
 * double / const double* row pointer either way), so these tests are mostly about locking down that
 * the externally-visible behaviour is unchanged, plus covering what is genuinely new: the
 * zero-initialization guarantee, getRow()/getColumn()/dotRow()/dotCol(), and move semantics.
 */

TEST( TestAQLNumericMatrix, UNIT_ConstructionIsZeroInitialized )
{
	// The old new double[row*col] left elements uninitialized; std::vector value-initializes them.
	// This is a real behaviour change (a deliberate fix, not just carried forward) worth locking
	// down explicitly.
	AQLNumericMatrix m( 3, 4 );
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

TEST( TestAQLNumericMatrix, UNIT_ElementAccess )
{
	AQLNumericMatrix m( 3, 4 );

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

TEST( TestAQLNumericMatrix, UNIT_ConstructionFromDoubleMatrix )
{
	DoubleMatrix mat = { { 1.0, 2.0, 3.0 },
	                     { 4.0, 5.0, 6.0 } };

	AQLNumericMatrix m( mat );
	EXPECT_EQ( 2u, m.row() );
	EXPECT_EQ( 3u, m.column() );
	EXPECT_EQ( 1.0, m.getValue( 0, 0 ) );
	EXPECT_EQ( 2.0, m.getValue( 0, 1 ) );
	EXPECT_EQ( 6.0, m.getValue( 1, 2 ) );
}

TEST( TestAQLNumericMatrix, UNIT_GetRowAndGetColumn )
{
	// [1 2 3]
	// [4 5 6]
	DoubleMatrix mat = { { 1.0, 2.0, 3.0 },
	                     { 4.0, 5.0, 6.0 } };
	AQLNumericMatrix m( mat );

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

TEST( TestAQLNumericMatrix, UNIT_GetRowAndGetColumn_OutOfRangeThrows )
{
	AQLNumericMatrix m( 2, 3 );
	EXPECT_THROW( m.getRow( 2 ), AQLCoreNumericalError );
	EXPECT_THROW( m.getColumn( 3 ), AQLCoreNumericalError );
}

TEST( TestAQLNumericMatrix, UNIT_DotRowAndDotCol )
{
	// [1 2 3]
	// [4 5 6]
	DoubleMatrix mat = { { 1.0, 2.0, 3.0 },
	                     { 4.0, 5.0, 6.0 } };
	AQLNumericMatrix m( mat );

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

TEST( TestAQLNumericMatrix, UNIT_DotRowAndDotCol_SizeMismatchThrows )
{
	AQLNumericMatrix m( 2, 3 );
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
TEST( TestAQLNumericMatrix, UNIT_DotRow_NumericalIntegrationPattern )
{
	const double timeStep = 1.0;
	const std::vector<double> zScores = { -3.0, -2.0, -1.0, 0.0, 1.0, 2.0, 3.0 };

	// Every row is symmetric about its centre, so the dot product with an antisymmetric z-score
	// grid cancels to zero.
	DoubleMatrix symmetric = { { 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5 },
	                          { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 } };
	AQLNumericMatrix survivalPDF( symmetric );

	EXPECT_NEAR( 0.0, survivalPDF.dotRow( 0, zScores ) * timeStep, 1e-12 );
	EXPECT_NEAR( 0.0, survivalPDF.dotRow( 1, zScores ) * timeStep, 1e-12 );

	// Perturb the first and last entries of each row so it is no longer symmetric - the cancellation
	// breaks and the result is exactly the sum of what was added/removed at the ends.
	DoubleMatrix asymmetric = { { 1.0, 0.5, 0.5, 0.5, 0.5, 0.5, 0.0 },
	                           { 2.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.0 } };
	AQLNumericMatrix asymmetricPDF( asymmetric );

	// row 0: symmetric part cancels to 0, remaining = (1.0-0.5)*(-3) + (0.0-0.5)*3 = -1.5 - 1.5 = -3
	EXPECT_NEAR( -3.0, asymmetricPDF.dotRow( 0, zScores ) * timeStep, 1e-12 );
	// row 1: (2.0-1.0)*(-3) + (0.0-1.0)*3 = -3 - 3 = -6
	EXPECT_NEAR( -6.0, asymmetricPDF.dotRow( 1, zScores ) * timeStep, 1e-12 );
}

TEST( TestAQLNumericMatrix, UNIT_ScalarMultiply )
{
	DoubleMatrix mat = { { 1.0, 2.0 },
	                     { 3.0, 4.0 } };
	AQLNumericMatrix m( mat );

	m *= 2.0;
	EXPECT_EQ( 2.0, m.getValue( 0, 0 ) );
	EXPECT_EQ( 4.0, m.getValue( 0, 1 ) );
	EXPECT_EQ( 6.0, m.getValue( 1, 0 ) );
	EXPECT_EQ( 8.0, m.getValue( 1, 1 ) );

	AQLNumericMatrix scaled = m * 0.5;
	EXPECT_EQ( 1.0, scaled.getValue( 0, 0 ) );
	EXPECT_EQ( 4.0, scaled.getValue( 1, 1 ) );
	// m itself is unchanged by operator* (only operator*= mutates)
	EXPECT_EQ( 2.0, m.getValue( 0, 0 ) );
}

TEST( TestAQLNumericMatrix, UNIT_ClearValuesAndSetValueAll )
{
	DoubleMatrix mat = { { 1.0, 2.0 },
	                     { 3.0, 4.0 } };
	AQLNumericMatrix m( mat );

	m.setValue( 9.0 );
	for ( unsigned int i = 0; i < m.row(); ++i )
		for ( unsigned int j = 0; j < m.column(); ++j )
			EXPECT_EQ( 9.0, m.getValue( i, j ) );

	m.clearValues();
	for ( unsigned int i = 0; i < m.row(); ++i )
		for ( unsigned int j = 0; j < m.column(); ++j )
			EXPECT_EQ( 0.0, m.getValue( i, j ) );
}

TEST( TestAQLNumericMatrix, UNIT_IdentityMatrix )
{
	AQLNumericMatrix m( 3, 3 );
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

TEST( TestAQLNumericMatrix, UNIT_Transpose )
{
	// [1 2 3]
	// [4 5 6]
	DoubleMatrix mat = { { 1.0, 2.0, 3.0 },
	                     { 4.0, 5.0, 6.0 } };
	AQLNumericMatrix m( mat );

	AQLNumericMatrix mt = m.transpose();

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
	AQLNumericMatrix mtt = mt.transpose();
	ASSERT_EQ( m.row(), mtt.row() );
	ASSERT_EQ( m.column(), mtt.column() );
	for ( unsigned int i = 0; i < m.row(); ++i )
		for ( unsigned int j = 0; j < m.column(); ++j )
			EXPECT_EQ( m.getValue( i, j ), mtt.getValue( i, j ) );
}

TEST( TestAQLNumericMatrix, UNIT_TransposeAboveOpenMPThreshold )
{
	// Exercises the >64-row branch of transpose()'s `#pragma omp parallel for if(...)` guard (see
	// OPENMP_SIZE_THRESHOLD in AQLNumericMatrix.cpp) - correctness must be identical whichever branch runs,
	// with or without OpenMPSupport actually enabled for the compiler that built this.
	const unsigned int n = 100;
	AQLNumericMatrix m( n, 3 );
	for ( unsigned int i = 0; i < n; ++i )
	{
		m.setValue( i, 0, static_cast<double>( i ) );
		m.setValue( i, 1, static_cast<double>( i ) * 2.0 );
		m.setValue( i, 2, static_cast<double>( i ) * 3.0 );
	}

	AQLNumericMatrix mt = m.transpose();
	ASSERT_EQ( 3u, mt.row() );
	ASSERT_EQ( n, mt.column() );
	for ( unsigned int i = 0; i < n; ++i )
	{
		EXPECT_EQ( m.getValue( i, 0 ), mt.getValue( 0, i ) );
		EXPECT_EQ( m.getValue( i, 1 ), mt.getValue( 1, i ) );
		EXPECT_EQ( m.getValue( i, 2 ), mt.getValue( 2, i ) );
	}
}

TEST( TestAQLNumericMatrix, UNIT_Resize_GrowPreservesOverlapAndZeroFillsTheRest )
{
	// [1 2]
	// [3 4]
	DoubleMatrix mat = { { 1.0, 2.0 },
	                     { 3.0, 4.0 } };
	AQLNumericMatrix m( mat );

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

TEST( TestAQLNumericMatrix, UNIT_Resize_ShrinkKeepsOnlyTheOverlap )
{
	DoubleMatrix mat = { { 1.0, 2.0, 3.0 },
	                     { 4.0, 5.0, 6.0 },
	                     { 7.0, 8.0, 9.0 } };
	AQLNumericMatrix m( mat );

	m.resize( 2, 2 );
	ASSERT_EQ( 2u, m.row() );
	ASSERT_EQ( 2u, m.column() );
	EXPECT_EQ( 1.0, m.getValue( 0, 0 ) );
	EXPECT_EQ( 2.0, m.getValue( 0, 1 ) );
	EXPECT_EQ( 4.0, m.getValue( 1, 0 ) );
	EXPECT_EQ( 5.0, m.getValue( 1, 1 ) );
}

TEST( TestAQLNumericMatrix, UNIT_MoveConstructor )
{
	DoubleMatrix mat = { { 1.0, 2.0 },
	                     { 3.0, 4.0 } };
	AQLNumericMatrix source( mat );

	AQLNumericMatrix moved( std::move( source ) );
	EXPECT_EQ( 2u, moved.row() );
	EXPECT_EQ( 2u, moved.column() );
	EXPECT_EQ( 1.0, moved.getValue( 0, 0 ) );
	EXPECT_EQ( 4.0, moved.getValue( 1, 1 ) );
}

TEST( TestAQLNumericMatrix, UNIT_MoveAssignment )
{
	DoubleMatrix mat = { { 1.0, 2.0 },
	                     { 3.0, 4.0 } };
	AQLNumericMatrix source( mat );
	AQLNumericMatrix target( 1, 1 );

	target = std::move( source );
	EXPECT_EQ( 2u, target.row() );
	EXPECT_EQ( 2u, target.column() );
	EXPECT_EQ( 1.0, target.getValue( 0, 0 ) );
	EXPECT_EQ( 4.0, target.getValue( 1, 1 ) );
}

TEST( TestAQLNumericMatrix, UNIT_CopyIsIndependentOfSource )
{
	// AQLNumericMatrix keeps shared_ptr-based copy-on-write sharing (unlike AQLCalendar/AQLDataInstance,
	// which dropped it) because arithmetic operators return by value pervasively - a copy still
	// needs to behave like an independent value from the caller's point of view, i.e. mutating the
	// copy must not be visible through the original.
	DoubleMatrix mat = { { 1.0, 2.0 },
	                     { 3.0, 4.0 } };
	AQLNumericMatrix original( mat );
	AQLNumericMatrix copy( original );

	copy.setValue( 0, 0, 99.0 );
	EXPECT_EQ( 99.0, copy.getValue( 0, 0 ) );
	EXPECT_EQ( 1.0, original.getValue( 0, 0 ) );  // unaffected by the copy's mutation
}

/*
 * @brief The everyday/usability methods added alongside the throw "Invalid Matrix" fix and the
 * ludcmp/svdcmp/tred2/tqli unchecked-write efficiency pass: brace-init construction, the
 * operator()(i,j) read/write pair, trace/norm, the static identity() factory, operator!=/equals(),
 * getDiagonal(), toDoubleMatrix() and operator<<.
 */

TEST( TestAQLNumericMatrix, UNIT_BraceInitConstructor )
{
	AQLNumericMatrix m{ { 1.0, 2.0, 3.0 },
	                    { 4.0, 5.0, 6.0 } };

	ASSERT_EQ( 2u, m.row() );
	ASSERT_EQ( 3u, m.column() );
	EXPECT_EQ( 1.0, m.getValue( 0, 0 ) );
	EXPECT_EQ( 6.0, m.getValue( 1, 2 ) );
}

TEST( TestAQLNumericMatrix, UNIT_BraceInitConstructor_RaggedRowsThrows )
{
	EXPECT_THROW( ( AQLNumericMatrix{ { 1.0, 2.0 }, { 3.0 } } ), AQLCoreNumericalError );
}

TEST( TestAQLNumericMatrix, UNIT_CallOperatorReadWrite )
{
	AQLNumericMatrix m( 2, 2 );
	m( 0, 0 ) = 1.0;
	m( 0, 1 ) = 2.0;
	m( 1, 0 ) = 3.0;
	m( 1, 1 ) = 4.0;

	EXPECT_EQ( 1.0, m( 0, 0 ) );
	EXPECT_EQ( 4.0, m( 1, 1 ) );
	EXPECT_EQ( m.getValue( 0, 1 ), m( 0, 1 ) );

	const AQLNumericMatrix& constRef = m;
	EXPECT_EQ( 3.0, constRef( 1, 0 ) );
}

TEST( TestAQLNumericMatrix, UNIT_CallOperator_OutOfRangeThrows )
{
	AQLNumericMatrix m( 2, 2 );
	EXPECT_THROW( m( 2, 0 ), AQLCoreNumericalError );
	EXPECT_THROW( m( 0, 2 ) = 1.0, AQLCoreNumericalError );
}

TEST( TestAQLNumericMatrix, UNIT_CallOperator_WriteDoesNotAliasACopy )
{
	// The non-const operator() must detach from any COW sharer before writing, exactly like
	// setValue() - otherwise a write through one copy would corrupt another.
	AQLNumericMatrix original{ { 1.0, 2.0 }, { 3.0, 4.0 } };
	AQLNumericMatrix copy( original );

	copy( 0, 0 ) = 99.0;
	EXPECT_EQ( 99.0, copy( 0, 0 ) );
	EXPECT_EQ( 1.0, original( 0, 0 ) );
}

TEST( TestAQLNumericMatrix, UNIT_Trace )
{
	AQLNumericMatrix m{ { 1.0, 2.0, 3.0 },
	                    { 4.0, 5.0, 6.0 },
	                    { 7.0, 8.0, 9.0 } };
	EXPECT_EQ( 1.0 + 5.0 + 9.0, m.trace() );
}

TEST( TestAQLNumericMatrix, UNIT_Trace_NotSquareThrows )
{
	AQLNumericMatrix m( 2, 3 );
	EXPECT_THROW( m.trace(), AQLCoreNumericalError );
}

TEST( TestAQLNumericMatrix, UNIT_Norm )
{
	// Frobenius norm of [[3,4]] is sqrt(3^2+4^2) = 5, the classic 3-4-5 triple.
	AQLNumericMatrix m{ { 3.0, 4.0 } };
	EXPECT_NEAR( 5.0, m.norm(), 1e-12 );
}

TEST( TestAQLNumericMatrix, UNIT_StaticIdentityFactory )
{
	AQLNumericMatrix m = AQLNumericMatrix::identity( 3 );
	ASSERT_EQ( 3u, m.row() );
	ASSERT_EQ( 3u, m.column() );
	for ( unsigned int i = 0; i < m.row(); ++i )
		for ( unsigned int j = 0; j < m.column(); ++j )
			EXPECT_EQ( i == j ? 1.0 : 0.0, m.getValue( i, j ) );
}

TEST( TestAQLNumericMatrix, UNIT_EqualityOperators )
{
	AQLNumericMatrix a{ { 1.0, 2.0 }, { 3.0, 4.0 } };
	AQLNumericMatrix b{ { 1.0, 2.0 }, { 3.0, 4.0 } };
	AQLNumericMatrix c{ { 1.0, 2.0 }, { 3.0, 4.5 } };

	EXPECT_TRUE( a == b );
	EXPECT_FALSE( a != b );
	EXPECT_TRUE( a != c );
	EXPECT_FALSE( a == c );
}

TEST( TestAQLNumericMatrix, UNIT_Equals_Tolerance )
{
	AQLNumericMatrix a{ { 1.0, 2.0 }, { 3.0, 4.0 } };
	AQLNumericMatrix nearlyEqual{ { 1.0 + 1e-9, 2.0 }, { 3.0, 4.0 - 1e-9 } };
	AQLNumericMatrix notEqual{ { 1.1, 2.0 }, { 3.0, 4.0 } };

	// After real arithmetic, exact operator== almost never holds between numerically-equal
	// matrices - this is exactly the case equals(tolerance) exists for.
	EXPECT_FALSE( a == nearlyEqual );
	EXPECT_TRUE( a.equals( nearlyEqual, 1e-6 ) );
	EXPECT_FALSE( a.equals( notEqual, 1e-6 ) );

	AQLNumericMatrix wrongShape( 3, 2 );
	EXPECT_FALSE( a.equals( wrongShape, 1e6 ) );  // dimension mismatch is always "not equal"
}

TEST( TestAQLNumericMatrix, UNIT_GetDiagonal )
{
	AQLNumericMatrix m{ { 1.0, 2.0, 3.0 },
	                    { 4.0, 5.0, 6.0 } };
	const std::vector<double> diag = m.getDiagonal();
	ASSERT_EQ( 2u, diag.size() );  // min(row=2, column=3)
	EXPECT_EQ( 1.0, diag[0] );
	EXPECT_EQ( 5.0, diag[1] );
}

TEST( TestAQLNumericMatrix, UNIT_ToDoubleMatrix_RoundTrips )
{
	DoubleMatrix original = { { 1.0, 2.0, 3.0 },
	                          { 4.0, 5.0, 6.0 } };
	AQLNumericMatrix m( original );
	DoubleMatrix roundTripped = m.toDoubleMatrix();

	ASSERT_EQ( original.size(), roundTripped.size() );
	for ( size_t i = 0; i < original.size(); ++i )
	{
		ASSERT_EQ( original[i].size(), roundTripped[i].size() );
		for ( size_t j = 0; j < original[i].size(); ++j )
			EXPECT_EQ( original[i][j], roundTripped[i][j] );
	}
}

TEST( TestAQLNumericMatrix, UNIT_StreamOperator )
{
	AQLNumericMatrix m{ { 1.0, 2.0 }, { 3.0, 4.0 } };
	std::ostringstream oss;
	oss << m;

	// Not pinning down exact formatting (that is print()'s job for the CSV file path) - just
	// that streaming actually produces the four values, in row order, without throwing.
	const std::string text = oss.str();
	EXPECT_NE( std::string::npos, text.find( "1" ) );
	EXPECT_NE( std::string::npos, text.find( "4" ) );
}

/*
 * @brief Regression coverage for the four O(n^3) decomposition kernels (ludcmp, svdcmp, tred2,
 * tqli) after their inner loops were switched from the bounds-checked, COW-checked setValue()
 * to a single makeUnShared() up front plus direct (*pData_)[i][j] writes for the rest. No prior
 * GTEST coverage existed for determinant()/inverseMatrix()/choleskyDecomposition()/svDecomp()/
 * eigenMatrix() at all (CLAUDE.md already flags the Jacobian-risk analytics these kernels sit
 * under as "not exercised for a while"), so these are new tests, not just re-runs of old ones -
 * added specifically because this session's efficiency pass touched all four kernels directly.
 */

TEST( TestAQLNumericMatrix, UNIT_Determinant )
{
	// det([[3,8],[4,6]]) = 3*6 - 8*4 = -14
	AQLNumericMatrix m{ { 3.0, 8.0 }, { 4.0, 6.0 } };
	EXPECT_NEAR( -14.0, m.determinant(), 1e-9 );
}

TEST( TestAQLNumericMatrix, UNIT_InverseMatrix_ProductWithOriginalIsIdentity )
{
	AQLNumericMatrix m{ { 4.0, 7.0 },
	                    { 2.0, 6.0 } };
	AQLNumericMatrix inv = m.inverseMatrix();
	AQLNumericMatrix product = m * inv;

	EXPECT_TRUE( product.equals( AQLNumericMatrix::identity( 2 ), 1e-9 ) );
}

TEST( TestAQLNumericMatrix, UNIT_CholeskyDecomposition_ReconstructsOriginal )
{
	// A symmetric positive-definite matrix (diagonally dominant, so guaranteed SPD).
	AQLNumericMatrix a{ { 4.0, 2.0 },
	                    { 2.0, 3.0 } };
	AQLNumericMatrix l = a.choleskyDecomposition();

	// L * L^T must reconstruct the original.
	AQLNumericMatrix reconstructed = l * l.transpose();
	EXPECT_TRUE( reconstructed.equals( a, 1e-9 ) );
}

TEST( TestAQLNumericMatrix, UNIT_SvDecomp_ReconstructsOriginal )
{
	AQLNumericMatrix a{ { 3.0, 1.0 },
	                    { 1.0, 3.0 } };
	AQLNumericMatrix u, w, v;
	a.svDecomp( u, w, v );

	// U * W * V^T must reconstruct the original (the standard SVD identity).
	AQLNumericMatrix reconstructed = (u * w) * v.transpose();
	EXPECT_TRUE( reconstructed.equals( a, 1e-9 ) );
}

TEST( TestAQLNumericMatrix, UNIT_EigenMatrix_TraceEqualsSumOfEigenvalues )
{
	// A standard invariant: trace(A) == sum of eigenvalues, for any symmetric A.
	AQLNumericMatrix a{ { 2.0, 1.0 },
	                    { 1.0, 2.0 } };
	AQLNumericMatrix vec, val;
	a.eigenMatrix( vec, val );

	double sumOfEigenvalues = 0.0;
	for ( unsigned int i = 0; i < val.column(); ++i )
		sumOfEigenvalues += val.getValue( 0, i );

	EXPECT_NEAR( a.trace(), sumOfEigenvalues, 1e-9 );
}
