#pragma once

#ifdef __GNUG__
#pragma interface
#endif


#include "LABasic.h"
#include "LACoreSystemError.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include <vector>






/////////////////// Matrix class /////////////////////////////
/*! 
    @brief Class declaration to define a matrix operation.
*/

class LAMatrix {
public:
//	LIFECYCLE

	// constructor (n row, m column)
	LAMatrix(unsigned int n, unsigned int m);
	// constructor
	LAMatrix(const DoubleMatrix& mat);
	// constructor
	LAMatrix(const DoubleArray& array);
	//====================================================================
	// copy constructor
	LAMatrix(const LAMatrix& m);
	//====================================================================
	// default constructor
	LAMatrix(void);

	// destructor
	~LAMatrix(void);

//  QUERY
							//=============================================
							//function to get row numbers
	unsigned int			row(void) const 
								{return mpData == NULL ? 0 : mpData->row();}
							//=============================================
							// function to get column numbers
	unsigned int			column(void) const 
								{return mpData == NULL ? 0 : mpData->col();}
							//=============================================
							// function to get the value at element(i,j)
	double					getValue(unsigned int i, unsigned int j) const;
							//=============================================
							// function to get the maximum value
	double					maxValue(void) const;
							//=============================================
							// function to get the minimum value
	double					minValue(void) const;
							//=============================================
							// function to return the ratio of the maximum and minimum of the singular value of the matrix
	double					conditionNumber(void) const;
							//=============================================
							// function to return the rank
	int						rank(void) const;
							//=============================================
							// function to judge symmetric or not
	bool					isSymmetric(void) const;
							//=============================================
							// function to judge square or not
	bool					isSquare(void) const 
								{return mpData!= NULL && row() == column();}
							//=============================================
							// function to judge (r,c) is within the matrix
							/*!
								@param[in] r row
								@param[in] c column
							*/
	bool					isWithin(unsigned int r, unsigned int c) const 
							{ return (r < row() && c < column());}

//  OPERATION
							//==============================================
							// function to set the value(double) to element(i,j)
	void					setValue(unsigned int i, unsigned int j, double value); 
							//==============================================
							// function to set the value(double) to all elements
	void					setValue(double value);
							//==============================================
							// let all of elements of the matrix be 0
	LAMatrix&				clearValues(void);
							//==============================================
							// resize data, all data is cleared
	void					resize(unsigned int n, unsigned int m);
	
//  Matrix Operation	
							//==============================================
							// let set diagonal component by 0 and set other components 0. \n In case of not a square matrix,
							// we regard it with diagonal part that a small matrix created by minimum numbers among row and column numbers 
	LAMatrix&				IdentityMatrix(void);
							//==============================================
							// transpose
	LAMatrix				transpose(void) const;
							//==============================================
							// function to return submatrix with rs-re rows and with cs-ce columns
	LAMatrix				subMatrix(unsigned int rs, unsigned int re, 
												unsigned int cs, unsigned int ce) const;
							//==============================================
							// function that returns the inverse matrix (not square matrix is exceptions)
	LAMatrix				inverseMatrix(void) const;
							//==============================================
							// function that returns a result of the determinant
	double					determinant(void) const;
							//==============================================
							// function that returns the matrix after Cholesky decomposition (not a symmetric matrix is exceptions)
	LAMatrix				choleskyDecomposition(void) const;
							//	void luDecomp(LAMatrix& l, LAMatrix& u) const;
							//==============================================
							// function that performs the singular value decomposition, decompose the original function into u * w * v ^ T
	void					svDecomp(LAMatrix& u, LAMatrix& w, 
											LAMatrix& v) const;
							//==============================================
							// function to calculate the eigen value and eigen vectors, decompose the original function into vec^T * val * vec (not a symmetric matrix is exceptions)
	void					eigenMatrix(LAMatrix& vec, LAMatrix& val) const;

/// OPERATORS
							// assignment
	LAMatrix&				operator =(const LAMatrix& m);
							// product of the matrix
	LAMatrix				operator *(const LAMatrix&) const;
							// addition of a matrix
	LAMatrix				operator +(const LAMatrix&) const;
							// subtraction of the matrix
	LAMatrix				operator - (const LAMatrix&) const;
							// addition and assignment of a matrix
	LAMatrix&				operator +=(const LAMatrix&);
							// product and assignment of a matrix
	LAMatrix&				operator *=(const LAMatrix&);
							// subtraction and assignment of a matrix
	LAMatrix&				operator -=(const LAMatrix&);
							// multiple constant
	LAMatrix				operator *(const double& x) const;
							// multiple constant and assignment 
	LAMatrix&				operator *=(const double& x);
							// relational operator
	bool					operator ==(const LAMatrix&) const;
							// return the first pointer
	const double*			operator[](const int i) const 
											{return (*mpData)[i];}

//  DEBUG_METHODS
	
	void					print(const char* file="Matrix.csv") const;

private:
	/*! 
		@brief a subclass to hold the matrix data
	*/
	class LAMatrixData {
	public:
		// constructor (n row, m column)
		LAMatrixData(unsigned int n, unsigned int m);

		// destructor
		~LAMatrixData();

		// resize data
		void			resize(unsigned int n, unsigned int m);

		// function to get row numbers
		/*!
			@return row numbers
		*/
		unsigned int			row(void) const {return mRow;}

		// function to get column numbers
		/*!
			@return column numbers
		*/
		unsigned int			col(void) const {return mCol;}
		
		// function to get pointer
		/*!
			@return pointer
		*/
		double*			operator[](unsigned int n) {return mpData[n];}
		// function to get pointer
		/*!
			@return pointer (const)
		*/        
		const double*	operator[](unsigned int n) const {return mpData[n];}

	private:
		// pointer
		double** mpData;
		// row numbers
		unsigned int mRow;

		// column numbers
		unsigned int mCol;
	};

	// pointer of a matrix
	mutable LAMatrixData*	 mpData;
	
	// pointer
	mutable int*           mpRefCount;
    
	// 	
	void					makeUnShared(void) const;
	LAMatrix&				copy(const LAMatrix& tensor);
	void					clear(void);

	// LU Decomposition
	static void				ludcmp(LAMatrix& ret, std::vector<int>& indx, double& d);
	// LU Decomposition
	static void				lubksb(const LAMatrix& a, const std::vector<int>& indx, 
													std::vector<double>& b);
	static void				mprove(const LAMatrix& a, const LAMatrix& alud, 
												const std::vector<int>& indx, 
												const std::vector<double>& b, 
												std::vector<double>& x); 
	static void				svdcmp(LAMatrix& a, std::vector<double>& w, LAMatrix& v);
	static void				tred2(LAMatrix& a, LAMatrix& d, std::vector<double>& e);
	static void				tqli(LAMatrix& d, std::vector<double>& e,  LAMatrix& z);
#ifdef USE_QUANTLIB_SVD
	static void             SVD(int m_, int n_, std::vector<double>& _a, std::vector<double>& _u, std::vector<double>& s_, std::vector<double>& _v);
#endif

};

