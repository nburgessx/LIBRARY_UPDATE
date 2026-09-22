#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLBasic.h"
#include "AQLCoreSystemError.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include <memory>
#include <vector>
#include <initializer_list>
#include <iosfwd>


/////////////////// Matrix class /////////////////////////////
/*!
    @brief Class declaration to define a matrix operation.
*/

class AQLNumericMatrix {
public:

	// constructor (n row, m column)
	AQLNumericMatrix(unsigned int n, unsigned int m);

	// constructor
	AQLNumericMatrix(const DoubleMatrix& mat);

	// constructor
	AQLNumericMatrix(const DoubleArray& array);

	//====================================================================
	// copy constructor
	AQLNumericMatrix(const AQLNumericMatrix& m);
	//====================================================================

	// move constructor - steals the other matrix's buffer outright, no allocation, no refcount
	// traffic at all. Worth having here specifically: every arithmetic operator below
	// (operator+, operator*, transpose(), inverseMatrix(), ...) returns AQLNumericMatrix by value, so a
	// move constructor lets the compiler avoid even the shared_ptr refcount bump in the
	// "build a temporary, return it" pattern all of them use.
	AQLNumericMatrix(AQLNumericMatrix&& m) noexcept;

	// brace-init constructor, e.g. AQLNumericMatrix m{ {1,2}, {3,4} } - purely ergonomic, for
	// tests and small hand-built matrices, so callers do not need a DoubleMatrix detour just to
	// write a literal matrix down. Every row must be the same length (ragged input throws -
	// silently zero-padding would hide what is almost always a typo at the call site).
	AQLNumericMatrix(std::initializer_list<std::initializer_list<double>> rows);

	// default constructor
	AQLNumericMatrix(void);

	// destructor
	~AQLNumericMatrix(void);

	
	//function to get row numbers
	unsigned int			row(void) const
								{return pData_ == NULL ? 0 : pData_->row();}
							
	// function to get column numbers
	unsigned int			column(void) const 
								{return pData_ == NULL ? 0 : pData_->col();}
							
	// function to get the value at element(i,j)
	double					getValue(unsigned int i, unsigned int j) const;
	
	// function to get the maximum value
	double					maxValue(void) const;
	
	// function to get the minimum value
	double					minValue(void) const;
	
	// function to return the ratio of the maximum and minimum of the singular value of the matrix
	double					conditionNumber(void) const;
	
	// function to return the rank
	int						rank(void) const;
	
	// function to judge symmetric or not
	bool					isSymmetric(void) const;
	
	// function to judge square or not
	bool					isSquare(void) const 
								{return pData_!= NULL && row() == column();}
	
	// function to judge (r,c) is within the matrix
	/*!
		@param[in] r row
		@param[in] c column
	*/
	bool					isWithin(unsigned int r, unsigned int c) const
							{ return (r < row() && c < column());}

	// Bounds-checked read/write element access, e.g. m(i,j) = x; x = m(i,j);. operator[] below
	// only ever offered a const read (a raw row pointer); every write had to go through
	// setValue(i,j,x) instead, an asymmetric read/write API that is easy to trip over. This is
	// the idiomatic C++ matrix accessor pair (Eigen, Boost.uBLAS both use it) and correctly
	// detaches from any COW sharer on the non-const path, exactly like setValue().
	double&					operator()(unsigned int i, unsigned int j);
	double					operator()(unsigned int i, unsigned int j) const;

	//=============================================
	// return row i as a plain vector<double> - a convenience for callers doing
	// numerical work (e.g. numerical integration: dotRow(i, weights) * step)
	// against one row/column at a time without going through operator[] twice.
	std::vector<double>		getRow(unsigned int i) const;
	
	// return column j as a plain vector<double>. Column extraction cannot use
	// a single contiguous copy the way getRow() can (a column is strided, not
	// contiguous, in this row-major layout) but is still O(rows), just with a
	// stride instead of a memcpy.
	std::vector<double>		getColumn(unsigned int j) const;
	
	// dot product of row i with v (v.size() must equal column()). The
	// workhorse for numerical integration against a weights/PDF vector -
	// e.g. a Gaussian-copula-style basket survival probability, or any
	// row-of-values . weights-vector reduction.
	double					dotRow(unsigned int i, const std::vector<double>& v) const;
	
	// dot product of column j with v (v.size() must equal row()).
	double					dotCol(unsigned int j, const std::vector<double>& v) const;

	// return the diagonal as a plain vector<double> (size = min(row(),column()) for a
	// non-square matrix). A common everyday need alongside getRow/getColumn - variance
	// vectors, calibration diagnostics - currently hand-rolled at every call site that needs it.
	std::vector<double>		getDiagonal(void) const;


	// function to set the value(double) to element(i,j)
	void					setValue(unsigned int i, unsigned int j, double value); 
	
	// function to set the value(double) to all elements
	void					setValue(double value);
	
	// let all of elements of the matrix be 0
	AQLNumericMatrix&				clearValues(void);
	
	// resize data, all data is cleared
	void					resize(unsigned int n, unsigned int m);
	
							
	// let set diagonal component by 0 and set other components 0. \n In case of not a square matrix,
	// we regard it with diagonal part that a small matrix created by minimum numbers among row and column numbers 
	AQLNumericMatrix&				IdentityMatrix(void);

	// Static factory for the everyday "give me a fresh identity matrix" need, without first
	// having to construct-then-mutate via the in-place IdentityMatrix() above.
	static AQLNumericMatrix		identity(unsigned int n);

	// transpose
	AQLNumericMatrix				transpose(void) const;
	
	// function to return submatrix with rs-re rows and with cs-ce columns
	AQLNumericMatrix				subMatrix(unsigned int rs, unsigned int re, 
												unsigned int cs, unsigned int ce) const;
	
	// function that returns the inverse matrix (not square matrix is exceptions)
	AQLNumericMatrix				inverseMatrix(void) const;
	
	// function that returns a result of the determinant
	double					determinant(void) const;

	// sum of the diagonal elements (not square is an exception, same convention as
	// choleskyDecomposition()/isSymmetric() below)
	double					trace(void) const;

	// Frobenius norm: sqrt(sum of every element squared). Common convergence/scale check in
	// calibration code - e.g. "has this Jacobian update stopped changing" - currently absent,
	// so every caller that needs it hand-rolls the double loop.
	double					norm(void) const;

	// function that returns the matrix after Cholesky decomposition (not a symmetric matrix is exceptions)
	AQLNumericMatrix				choleskyDecomposition(void) const;
							
	//	void luDecomp(AQLNumericMatrix& l, AQLNumericMatrix& u) const;
	
	// function that performs the singular value decomposition, decompose the original function into u * w * v ^ T
	void					svDecomp(AQLNumericMatrix& u, AQLNumericMatrix& w, AQLNumericMatrix& v) const;
							
		// function to calculate the eigen value and eigen vectors, decompose the original function into vec^T * val * vec (not a symmetric matrix is exceptions)
	void					eigenMatrix(AQLNumericMatrix& vec, AQLNumericMatrix& val) const;

	// assignment
	AQLNumericMatrix&				operator =(const AQLNumericMatrix& m);
	
	// move assignment - same rationale as the move constructor above
	AQLNumericMatrix&				operator =(AQLNumericMatrix&& m) noexcept;
							
	// product of the matrix
	AQLNumericMatrix				operator *(const AQLNumericMatrix&) const;
							
	// addition of a matrix
	AQLNumericMatrix				operator +(const AQLNumericMatrix&) const;
							
	// subtraction of the matrix
	AQLNumericMatrix				operator - (const AQLNumericMatrix&) const;
							
	// addition and assignment of a matrix
	AQLNumericMatrix&				operator +=(const AQLNumericMatrix&);
							
	// product and assignment of a matrix
	AQLNumericMatrix&				operator *=(const AQLNumericMatrix&);
							
	// subtraction and assignment of a matrix
	AQLNumericMatrix&				operator -=(const AQLNumericMatrix&);
							
	// multiple constant
	AQLNumericMatrix				operator *(const double& x) const;
							
	// multiple constant and assignment 
	AQLNumericMatrix&				operator *=(const double& x);

	// relational operator - exact element-by-element double comparison. Genuinely wanted for
	// identity/reference checks (e.g. "is this literally the same data I passed in"), but after
	// any arithmetic (inverseMatrix(), operator*, a decomposition) two matrices that are
	// numerically equal will almost never compare exactly equal - use equals() below for that.
	bool					operator ==(const AQLNumericMatrix&) const;

	// inequality - the natural complement to operator== above, same exact-comparison caveat.
	bool					operator !=(const AQLNumericMatrix& other) const { return !(*this == other); }

	// tolerance-based comparison for numerical work (tests, convergence checks) where exact
	// double equality is the wrong question to ask. |a(i,j) - b(i,j)| <= tolerance for every
	// element; dimension mismatch is always "not equal", never an exception.
	bool					equals(const AQLNumericMatrix& other, double tolerance) const;

	// return the first pointer
	const double*			operator[](const int i) const {return (*pData_)[i];}

	// Round-trips back out to the marshalling-friendly nested-vector shape - the reverse of the
	// DoubleMatrix constructor above. Every caller that needs this today hand-rolls the double
	// loop at the call site.
	DoubleMatrix			toDoubleMatrix(void) const;

//  DEBUG_METHODS

	void					print(const char* file="Matrix.csv") const;

	// Stream a human-readable rendering, e.g. for a log line or an exception message, without
	// going via print()'s file-on-disk path. Declared as a free function (not a member) per the
	// usual operator<< convention, but listed here so it is discoverable alongside the class.
	friend std::ostream&	operator<<(std::ostream& os, const AQLNumericMatrix& m);

private:
	/*!
		@brief a subclass to hold the matrix data

		Storage is a single, contiguous std::vector<double> in row-major order
		(data_[row * col_ + col]) rather than the old double** (one heap allocation for the row
		pointer array, plus a second, separate allocation per row). Two things that buys:
		  - One allocation total instead of 1 + row_, and everything is packed together, so
		    row-major traversal (which is how every algorithm here - ludcmp, svdcmp, tred2, tqli,
		    transpose, the scalar/reduction ops - actually walks the matrix) hits contiguous
		    memory instead of chasing a separate pointer per row. That is a real cache-locality
		    win for anything past the smallest matrices, not a micro-optimisation.
		  - operator[](n) still returns a double* (or const double*) pointing at the start of row n
		    (now data_.data() + n*col_ instead of pData_[n]), so every existing call site in
		    AQLNumericMatrix.cpp that does (*pData_)[i][j] or pData_->row()/col() needed zero changes -
		    the row-pointer *contract* is unchanged, only what is behind it.
	*/
	class AQLMatrixData {
	public:
		// constructor (n row, m column) - zero-initializes all elements (matches std::vector's
		// value-initialization; the old new double[row*col] left elements uninitialized, which
		// is a real footgun a caller could silently read garbage from - deliberately fixed here,
		// not just carried forward, since it can only make behaviour more correct, never less)
		AQLMatrixData(unsigned int n, unsigned int m);

		// destructor
		~AQLMatrixData();

		// resize data
		void			resize(unsigned int n, unsigned int m);

		// function to get row numbers
		/*!
			@return row numbers
		*/
		unsigned int			row(void) const {return row_;}

		// function to get column numbers
		/*!
			@return column numbers
		*/
		unsigned int			col(void) const {return col_;}

		// function to get pointer to the start of row n
		/*!
			@return pointer
		*/
		double*			operator[](unsigned int n) {return data_.data() + static_cast<std::size_t>(n) * col_;}
		// function to get pointer to the start of row n (const)
		/*!
			@return pointer (const)
		*/
		const double*	operator[](unsigned int n) const {return data_.data() + static_cast<std::size_t>(n) * col_;}

	private:
		// flattened, row-major storage: data_[row * col_ + col]
		std::vector<double> data_;
		// row numbers
		unsigned int row_;

		// column numbers
		unsigned int col_;
	};

	// Copy-on-write payload. This used to be a raw AQLMatrixData* plus a separate, independently
	// allocated, plain (non-atomic) int* refcount - the same "looks thread-safe, isn't" shape
	// AQLString and AQLCalendar both had, except this one's count wasn't even atomic, so the count
	// itself could race, not just the payload it counted references to (see AQLString.h's
	// stringData_ field comment for the full account of why that pattern is unsafe).
	// std::shared_ptr's control block has the correct acquire/release pairing to make sharing safe,
	// and collapses the two heap allocations into one via make_shared. Kept as shared_ptr rather
	// than moving to always-deep-copy (the way AQLString's internals ultimately did) because,
	// unlike AQLString/AQLCalendar, this class's usage is genuinely copy-heavy: every arithmetic
	// operator returns AQLNumericMatrix by value, so O(1) sharing is actually earning its keep here.
	mutable std::shared_ptr<AQLMatrixData>	 pData_;

	void					makeUnShared(void) const;
	AQLNumericMatrix&				copy(const AQLNumericMatrix& tensor);
	void					clear(void);

	// LU Decomposition
	static void				ludcmp(AQLNumericMatrix& ret, std::vector<int>& indx, double& d);
	
	// LU Decomposition
	static void				lubksb(const AQLNumericMatrix& a, const std::vector<int>& indx, std::vector<double>& b);

	static void				mprove(const AQLNumericMatrix& a, const AQLNumericMatrix& alud, const std::vector<int>& indx, 
								   const std::vector<double>& b, std::vector<double>& x); 
	
	static void				svdcmp(AQLNumericMatrix& a, std::vector<double>& w, AQLNumericMatrix& v);
	static void				tred2(AQLNumericMatrix& a, AQLNumericMatrix& d, std::vector<double>& e);
	static void				tqli(AQLNumericMatrix& d, std::vector<double>& e,  AQLNumericMatrix& z);

#ifdef USE_QUANTLIB_SVD
	static void             SVD(int m_, int n_, std::vector<double>& _a, std::vector<double>& _u, std::vector<double>& s_, std::vector<double>& _v);
#endif

};

