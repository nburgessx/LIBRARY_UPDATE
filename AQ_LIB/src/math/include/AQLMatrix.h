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


/////////////////// Matrix class /////////////////////////////
/*!
    @brief Class declaration to define a matrix operation.
*/

class AQLMatrix {
public:

	// constructor (n row, m column)
	AQLMatrix(unsigned int n, unsigned int m);

	// constructor
	AQLMatrix(const DoubleMatrix& mat);

	// constructor
	AQLMatrix(const DoubleArray& array);

	//====================================================================
	// copy constructor
	AQLMatrix(const AQLMatrix& m);
	//====================================================================

	// move constructor - steals the other matrix's buffer outright, no allocation, no refcount
	// traffic at all. Worth having here specifically: every arithmetic operator below
	// (operator+, operator*, transpose(), inverseMatrix(), ...) returns AQLMatrix by value, so a
	// move constructor lets the compiler avoid even the shared_ptr refcount bump in the
	// "build a temporary, return it" pattern all of them use.
	AQLMatrix(AQLMatrix&& m) noexcept;

	// default constructor
	AQLMatrix(void);

	// destructor
	~AQLMatrix(void);

	
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

	
	// function to set the value(double) to element(i,j)
	void					setValue(unsigned int i, unsigned int j, double value); 
	
	// function to set the value(double) to all elements
	void					setValue(double value);
	
	// let all of elements of the matrix be 0
	AQLMatrix&				clearValues(void);
	
	// resize data, all data is cleared
	void					resize(unsigned int n, unsigned int m);
	
							
	// let set diagonal component by 0 and set other components 0. \n In case of not a square matrix,
	// we regard it with diagonal part that a small matrix created by minimum numbers among row and column numbers 
	AQLMatrix&				IdentityMatrix(void);
	
	// transpose
	AQLMatrix				transpose(void) const;
	
	// function to return submatrix with rs-re rows and with cs-ce columns
	AQLMatrix				subMatrix(unsigned int rs, unsigned int re, 
												unsigned int cs, unsigned int ce) const;
	
	// function that returns the inverse matrix (not square matrix is exceptions)
	AQLMatrix				inverseMatrix(void) const;
	
	// function that returns a result of the determinant
	double					determinant(void) const;
							
	// function that returns the matrix after Cholesky decomposition (not a symmetric matrix is exceptions)
	AQLMatrix				choleskyDecomposition(void) const;
							
	//	void luDecomp(AQLMatrix& l, AQLMatrix& u) const;
	
	// function that performs the singular value decomposition, decompose the original function into u * w * v ^ T
	void					svDecomp(AQLMatrix& u, AQLMatrix& w, AQLMatrix& v) const;
							
		// function to calculate the eigen value and eigen vectors, decompose the original function into vec^T * val * vec (not a symmetric matrix is exceptions)
	void					eigenMatrix(AQLMatrix& vec, AQLMatrix& val) const;

	// assignment
	AQLMatrix&				operator =(const AQLMatrix& m);
	
	// move assignment - same rationale as the move constructor above
	AQLMatrix&				operator =(AQLMatrix&& m) noexcept;
							
	// product of the matrix
	AQLMatrix				operator *(const AQLMatrix&) const;
							
	// addition of a matrix
	AQLMatrix				operator +(const AQLMatrix&) const;
							
	// subtraction of the matrix
	AQLMatrix				operator - (const AQLMatrix&) const;
							
	// addition and assignment of a matrix
	AQLMatrix&				operator +=(const AQLMatrix&);
							
	// product and assignment of a matrix
	AQLMatrix&				operator *=(const AQLMatrix&);
							
	// subtraction and assignment of a matrix
	AQLMatrix&				operator -=(const AQLMatrix&);
							
	// multiple constant
	AQLMatrix				operator *(const double& x) const;
							
	// multiple constant and assignment 
	AQLMatrix&				operator *=(const double& x);

	// relational operator
	bool					operator ==(const AQLMatrix&) const;
							
	// return the first pointer
	const double*			operator[](const int i) const {return (*pData_)[i];}

//  DEBUG_METHODS
	
	void					print(const char* file="Matrix.csv") const;

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
		    AQLMatrix.cpp that does (*pData_)[i][j] or pData_->row()/col() needed zero changes -
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
	// operator returns AQLMatrix by value, so O(1) sharing is actually earning its keep here.
	mutable std::shared_ptr<AQLMatrixData>	 pData_;

	void					makeUnShared(void) const;
	AQLMatrix&				copy(const AQLMatrix& tensor);
	void					clear(void);

	// LU Decomposition
	static void				ludcmp(AQLMatrix& ret, std::vector<int>& indx, double& d);
	
	// LU Decomposition
	static void				lubksb(const AQLMatrix& a, const std::vector<int>& indx, std::vector<double>& b);

	static void				mprove(const AQLMatrix& a, const AQLMatrix& alud, const std::vector<int>& indx, 
								   const std::vector<double>& b, std::vector<double>& x); 
	
	static void				svdcmp(AQLMatrix& a, std::vector<double>& w, AQLMatrix& v);
	static void				tred2(AQLMatrix& a, AQLMatrix& d, std::vector<double>& e);
	static void				tqli(AQLMatrix& d, std::vector<double>& e,  AQLMatrix& z);

#ifdef USE_QUANTLIB_SVD
	static void             SVD(int m_, int n_, std::vector<double>& _a, std::vector<double>& _u, std::vector<double>& s_, std::vector<double>& _v);
#endif

};

