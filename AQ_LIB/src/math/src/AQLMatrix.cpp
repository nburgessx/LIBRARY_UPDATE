/*! @file
    @brief Implementation to define a matrix operation.
*/


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "AQLMatrix.h"
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string.h>
#include <algorithm>

#ifdef _OPENMP
#include <omp.h>
#endif

using namespace std;

// maximum number of iterations
#define ITERATION 30

// Below this many rows/elements, a parallel region's thread-pool spin-up cost is likely to exceed
// the actual work being parallelized - this library's typical matrices (curve calibration: dozens
// to low hundreds of nodes) are often below it, so the `if()` clause on every `#pragma omp
// parallel for` below keeps them running the plain sequential loop with zero threading overhead,
// and only hands larger matrices (e.g. a fine-grained numerical integration grid) to OpenMP. One
// named constant, not a magic number scattered per call site, so it is a single place to retune
// once this has actually been profiled against a real large-matrix workload.
static const int OPENMP_SIZE_THRESHOLD = 64;

AQLMatrix::AQLMatrixData::AQLMatrixData(unsigned int row, unsigned int col)
{
    if (row == 0 || col == 0)
    {
        row_ = col_ = 0;
        return;
    }
    try
    {
        // std::vector value-initializes to 0.0 - unlike the old new double[row*col], which left
        // every element uninitialized until explicitly written.
        data_.resize(static_cast<std::size_t>(row) * col, 0.0);
        row_ = row;
        col_ = col;
    }
    catch (const bad_alloc& e)
    {
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief destructor
*/
AQLMatrix::AQLMatrixData::~AQLMatrixData()
{
}

void
AQLMatrix::AQLMatrixData::resize(unsigned int row, unsigned int col)
{
    if (row == 0 || col == 0)
    {
        data_.clear();
        row_ = col_ = 0;
        return;
    }

    // Row stride is changing (col_ -> col), so the overlapping top-left submatrix can't be moved
    // with a single contiguous copy the way a same-width resize could - copy it row by row into a
    // freshly allocated, zero-initialized buffer, then swap it in.
    std::vector<double> newData;
    try
    {
        newData.resize(static_cast<std::size_t>(row) * col, 0.0);
    }
    catch (const bad_alloc& e)
    {
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }

    const unsigned int rowsToCopy = (row < row_) ? row : row_;
    const unsigned int colsToCopy = (col < col_) ? col : col_;
    for (unsigned int i = 0; i < rowsToCopy; ++i)
    {
        const double* oldRow = data_.data() + static_cast<std::size_t>(i) * col_;
        double*       newRow = newData.data() + static_cast<std::size_t>(i) * col;
        for (unsigned int j = 0; j < colsToCopy; ++j)
        {
            newRow[j] = oldRow[j];
        }
    }

    data_ = std::move(newData);
    row_ = row;
    col_ = col;
}
/*!*********************************************************/
/*! public methods                                                                                                             */
/*!*********************************************************/

AQLMatrix::AQLMatrix(unsigned int n, unsigned int m) :
    pData_(NULL)
{
    pData_ = std::make_shared<AQLMatrixData>(n, m);
}

AQLMatrix::AQLMatrix(const DoubleMatrix& mat) :
    pData_(NULL)
{
    try
    {
        size_t rows = mat.size();
        size_t cols = rows == 0 ? 0 : mat[0].size(); // Access Violation Guard for mat[0]
        pData_ = std::make_shared<AQLMatrixData>(rows, cols);
		unsigned int i, j;
		for (i = 0; i < row(); i++)
		{
			for (j = 0; j < column(); j++)
			{
				(*pData_)[i][j] = mat[i][j];
			}
		}
	}
    catch (...)
    {
        throw "Invalid Matrix";
    }
}

AQLMatrix::AQLMatrix(const DoubleArray& array) :
    pData_(NULL)
{
    pData_ = std::make_shared<AQLMatrixData>(array.size(), 1);
	unsigned int i;
	for (i = 0; i < row(); i++)
	{
		(*pData_)[i][0] = array[i];
	}
}

AQLMatrix::AQLMatrix(const AQLMatrix& m)
{
    copy(m);
}

// Move constructor - steals m's shared_ptr outright: no allocation, no refcount traffic.
AQLMatrix::AQLMatrix(AQLMatrix&& m) noexcept
    : pData_(std::move(m.pData_))
{
}

AQLMatrix::AQLMatrix(void)
{
    pData_ = std::make_shared<AQLMatrixData>(0, 0);
}

/*!
    @brief destructor
*/
AQLMatrix::~AQLMatrix(void)
{
    clear();
}

double 
AQLMatrix::getValue(unsigned int i, unsigned int j) const
{
    if (!isWithin(i, j)) 
        throw AQLCoreNumericalError("Boundary Error", __FILE__, __LINE__);
    return (*pData_)[i][j]; 
}

double 
AQLMatrix::maxValue(void) const
{
    if (!isWithin(0, 0)) 
        throw AQLCoreNumericalError("Boundary Error", __FILE__, __LINE__);
    double max = (*pData_)[0][0];
    double ret = max;
    unsigned int i, j;
    for (i = 0; i < pData_->row(); ++i) {
        for (j = 0; j < pData_->col(); ++j) {
            if (max < AQLMath::abs((*pData_)[i][j])) {
                max = AQLMath::abs((*pData_)[i][j]);
                ret = (*pData_)[i][j];
            }
        }
    }
    return ret;
}

std::vector<double>
AQLMatrix::getRow(unsigned int i) const
{
    if (!isWithin(i, 0))
        throw AQLCoreNumericalError("Boundary Error", __FILE__, __LINE__);

    const unsigned int colCount = column();
    std::vector<double> result(colCount);
    const double* rowPtr = (*pData_)[i];
    // A single contiguous copy - row i is contiguous in the flattened, row-major storage.
    std::copy(rowPtr, rowPtr + colCount, result.begin());
    return result;
}

std::vector<double>
AQLMatrix::getColumn(unsigned int j) const
{
    if (!isWithin(0, j))
        throw AQLCoreNumericalError("Boundary Error", __FILE__, __LINE__);

    const int rowCount = static_cast<int>(row());
    std::vector<double> result(static_cast<std::size_t>(rowCount));
    // Strided, not contiguous (a column cuts across rows) - still O(rows), independent per
    // iteration, safe to parallelize for large matrices.
#ifdef _OPENMP
    #pragma omp parallel for if(rowCount > OPENMP_SIZE_THRESHOLD)
#endif
    for (int i = 0; i < rowCount; ++i)
    {
        result[static_cast<std::size_t>(i)] = (*pData_)[static_cast<unsigned int>(i)][j];
    }
    return result;
}

double
AQLMatrix::dotRow(unsigned int i, const std::vector<double>& v) const
{
    if (!isWithin(i, 0))
        throw AQLCoreNumericalError("Boundary Error", __FILE__, __LINE__);
    if (v.size() != column())
        throw AQLCoreNumericalError("dotRow: vector size does not match column count", __FILE__, __LINE__);

    const double* rowPtr = (*pData_)[i];
    const int colCount = static_cast<int>(column());
    double sum = 0.0;
    // `reduction(+:sum)` is OpenMP 2.0 (the version MSVC's classic /openmp implements) - safe to
    // rely on, unlike a min/max reduction which OpenMP 2.0 does not support.
#ifdef _OPENMP
    #pragma omp parallel for reduction(+:sum) if(colCount > OPENMP_SIZE_THRESHOLD)
#endif
    for (int j = 0; j < colCount; ++j)
    {
        sum += rowPtr[j] * v[static_cast<std::size_t>(j)];
    }
    return sum;
}

double
AQLMatrix::dotCol(unsigned int j, const std::vector<double>& v) const
{
    if (!isWithin(0, j))
        throw AQLCoreNumericalError("Boundary Error", __FILE__, __LINE__);
    if (v.size() != row())
        throw AQLCoreNumericalError("dotCol: vector size does not match row count", __FILE__, __LINE__);

    const int rowCount = static_cast<int>(row());
    double sum = 0.0;
#ifdef _OPENMP
    #pragma omp parallel for reduction(+:sum) if(rowCount > OPENMP_SIZE_THRESHOLD)
#endif
    for (int i = 0; i < rowCount; ++i)
    {
        sum += (*pData_)[static_cast<unsigned int>(i)][j] * v[static_cast<std::size_t>(i)];
    }
    return sum;
}

double
AQLMatrix::minValue(void) const
{
    if (!isWithin(0, 0)) 
        throw AQLCoreNumericalError("Boundary Error", __FILE__, __LINE__);
    double min = (*pData_)[0][0];
    double ret = min;
    unsigned int i, j;
    for (i = 0; i < pData_->row(); ++i) {
        for (j = 0; j < pData_->col(); ++j) {
            if (min > AQLMath::abs((*pData_)[i][j])) {
                min = AQLMath::abs((*pData_)[i][j]);
                ret = (*pData_)[i][j];
            }
        }
    }
    return ret;
}

double 
AQLMatrix::conditionNumber(void) const
{
    double min, max;
    unsigned int i;
    AQLMatrix u;
    AQLMatrix w;
    AQLMatrix v;
    
    svDecomp(u, w, v);
    for (i = 0, max =double(0); i < w.row(); i++) 
    {
        max = AQLMath::absMax(max, w[i][i]);
    }
    for (i = 0, min = max; i < w.row(); i++) 
    {
        min = AQLMath::absMin(min, w[i][i]);
    }
    return  max + min == max ? 
        double(1) / (AQLMath::epsValue(max)) : max/min;
} 

int 
AQLMatrix::rank(void) const
{
    int ret;
    unsigned int i;
    double max, wk;
    AQLMatrix u;
    AQLMatrix w;
    AQLMatrix v;
    
    svDecomp(u, w, v);
    for (i = 0, max =double(0); i < w.row(); i++)  {
        max = AQLMath::absMax(max, w[i][i]);
    }
    max *= AQLMath::epsValue(wk);
    for (i = 0, ret = 0/*1*/; i < w.row(); i++)  {
        if (w[i][i] > max) ret++;   
    }
    return ret;
} 

bool 
AQLMatrix::isSymmetric(void) const
{
    if (!isSquare()) return false;

    unsigned  int i, j;
    for (i = 0; i < row(); i++) {
        for (j = 0; j < i; j++) {
            if ((*pData_)[i][j] != (*pData_)[j][i]) {
                return false;
            }
        }
    }
    return true;
}

void 
AQLMatrix::setValue(unsigned int i, unsigned int j, double value)
{
    if (!isWithin(i, j)) 
        throw AQLCoreNumericalError("Boundary Error", __FILE__, __LINE__);

    makeUnShared();
    (*pData_)[i][j] = value;
} 		

void
AQLMatrix::setValue(double value)
{
    makeUnShared();
    const int rowCount = static_cast<int>(row());
    const unsigned int colCount = column();
    // Each outer (parallel) iteration touches only its own row - fully independent, safe to
    // parallelize. Loop counter is a signed int, not unsigned, because MSVC's OpenMP (2.0) requires
    // a signed canonical loop variable in a `parallel for`.
#ifdef _OPENMP
    #pragma omp parallel for if(rowCount > OPENMP_SIZE_THRESHOLD)
#endif
    for (int i = 0; i < rowCount; ++i)
    {
        double* rowPtr = (*pData_)[static_cast<unsigned int>(i)];
        for (unsigned int j = 0; j < colCount; ++j)
        {
            rowPtr[j] = value;
        }
    }
}

AQLMatrix&
AQLMatrix::clearValues(void)
{
    makeUnShared();
    const int rowCount = static_cast<int>(row());
    const unsigned int colCount = column();
#ifdef _OPENMP
    #pragma omp parallel for if(rowCount > OPENMP_SIZE_THRESHOLD)
#endif
    for (int i = 0; i < rowCount; ++i)
    {
        double* rowPtr = (*pData_)[static_cast<unsigned int>(i)];
        for (unsigned int j = 0; j < colCount; ++j)
        {
            rowPtr[j] = double(0);
        }
    }
    return *this;
}

void 
AQLMatrix::resize(unsigned int n, unsigned int m)
{
    makeUnShared();
    pData_->resize(n, m);
}

AQLMatrix& 
AQLMatrix::IdentityMatrix(void)
{
    unsigned int i;
    unsigned int min = static_cast<unsigned int>(AQLMath::min(row(), column()));
    clearValues(); // Unshared
    for (i = 0; i  < min; i++) {
        (*pData_)[i][i] = double(1);
    }
    return *this;
}

AQLMatrix
AQLMatrix::transpose(void) const
{
    AQLMatrix ret(column(), row());
    const int newRowCount = static_cast<int>(ret.row());   // == column()
    const unsigned int newColCount = ret.column();          // == row()
    // Parallelized over the *result's* rows (not the source's) so each thread writes a contiguous
    // row of ret - the source read is strided either way (transpose can't make both directions
    // contiguous at once), but write locality is the one that's free to choose, so it's worth
    // choosing. Each iteration writes a disjoint row of ret - safe to parallelize.
#ifdef _OPENMP
    #pragma omp parallel for if(newRowCount > OPENMP_SIZE_THRESHOLD)
#endif
    for (int i = 0; i < newRowCount; ++i)
    {
        double* dstRow = (*ret.pData_)[static_cast<unsigned int>(i)];
        for (unsigned int j = 0; j < newColCount; ++j)
        {
            // ret(i, j) = (*this)(j, i)
            dstRow[j] = (*pData_)[j][static_cast<unsigned int>(i)];
        }
    }
    return ret;
}

AQLMatrix 
AQLMatrix::subMatrix(unsigned int rs, unsigned int re, unsigned int cs, unsigned int ce) const
{
    AQLMatrix ret(re - rs + 1, ce - cs + 1);
    if (!isWithin(rs, cs) || !isWithin(re, ce)) {
        throw AQLCoreNumericalError("Boundary Error", __FILE__, __LINE__);
    } 
    unsigned int j, i;
    for (i = rs; i <= re; i++) {
        for (j = cs; j <= cs; j++) {
            (*ret.pData_)[i-rs][j-cs] = (*pData_)[i][j];
        }
    }
    return ret;
}
/*! 
    @brief ()
    @return AQLMatrix
*/
AQLMatrix 
AQLMatrix::inverseMatrix(void) const
{
    if (!isSquare()) {
        throw AQLCoreNumericalError("Matrix is not square", __FILE__, __LINE__);
    }
    AQLMatrix ret(*this);
    vector<int> indx;
    try {
        indx.resize(row());
    }
    catch (...)
    {
        throw AQLCoreSystemError(__FILE__,__LINE__);
    }
    AQLMatrix inv(row(), column());

    vector<double> col(row());
    vector<double> col1(row());
    double d;
    unsigned int i;
    
    ludcmp(ret, indx, d);
    for (unsigned int j = 0; j < row(); j++) {
        for (i=0; i < row(); i++) {
            col1[i] = col[i] = double(0);
        }
        col1[j] = col[j] = double(1);
        lubksb(ret, indx, col);
        mprove(*this, ret, indx, col1, col);
        for (i = 0; i < row(); i++) inv.setValue(i, j, col[i]);
    } 
    return inv;
}

/*! 
    @brief Function to return the calc result for Matrix calcuration
    @return calc result(double type)
*/
double 
AQLMatrix::determinant(void) const
{
    AQLMatrix a(*this);
    vector<int> indx;
    try {
        indx.resize(row());
    }
    catch (...)
    {
        throw AQLCoreSystemError(__FILE__,__LINE__);
    }
    double d;
     
    ludcmp(a, indx, d);
    for (unsigned int j = 0; j < row(); j++) d *= a[j][j];
    return d;
}

/*! 

    @brief Function to return Matrix after Cholesky decomposition
    @return AQLMatrix class after it is resolved
*/
AQLMatrix 
AQLMatrix::choleskyDecomposition(void) const
{
    if (!isSymmetric()) {
        throw AQLCoreNumericalError(
            "Matrix is not symmetric", __FILE__, __LINE__);
    }
    
    AQLMatrix ret(row(), row());
    ret.clearValues();
#if 1   
    unsigned int i, j, k;
    long double ik, ikjk;
    
    for (i = 0; i < row(); i++) {
        for (j = 0; j < i; j++) {
            ikjk = double(0);
            for (k = 0; k < j; k++) {
                ikjk -= (*ret.pData_)[i][k] * (*ret.pData_)[j][k] ;
            }
            ikjk += (*pData_)[i][j];
            (*ret.pData_)[i][j] = (double)(ikjk / (*ret.pData_)[j][j]); 
        }
        
        ik = 0;
        for (k = 0; k < i; k++) {
            ik -= (*ret.pData_)[i][k] * (*ret.pData_)[i][k]; 
        }
        ik += (*pData_)[i][i];

        if (ik <= 0.0) { 
            double nik =(ik == 0.0 ? 0.0000000001 : -double(ik));   
            char_t msg[128];
            SPRINTF(msg, 
                "I change value in choleskyDecomposition %Lf to %f at %d",
                    ik, nik, i);
            ik = nik;
        }
        (*ret.pData_)[i][i] = AQLMath::sqrt((double)ik);
    }

#else // Correct Cholesky decomposition 
    AQLMatrix w(row(), row());
    w.clearValues();
    double* d = new double[row()];
    double sum;
    int i, j, k;
    for (k = 0; k < row(); ++k)
    {
        for (i = 0; i < k; ++i)
        {
            sum=0;
            for (j = 0; j < i; ++j)
                sum += (*w.pData_)[k][j] * (*ret.pData_)[i][j];

            (*w.pData_)[k][i] = (*pData_)[k][i] - sum;
            (*ret.pData_)[k][i] = (*w.pData_)[k][i] / d[i];
        }
        sum = 0;
        for (j = 0; j < k; ++j)
            sum += (*w.pData_)[k][j] * (*ret.pData_)[k][j];
        
        (*ret.pData_)[k][k] = 1;
        d[k] = (*pData_)[k][k] - sum;
    
    }
    for (i = 0; i < row(); ++i)
    {
        if (d[i] <= 0.0)
        {
            double nik = 0.0000000001;
            PBString msg("I change value in choleskyDecomposition ");
            msg += PBString(d[i]) + " to " + PBString(nik) + 
                             " at " + PBString(int(i)); 
            AQLCoreError(msg.getCString(), __FILE__, __LINE__);
            d[k] = nik;
        }
        sum = AQLMath::sqrt(d[i]);
        for (j = i; j < row(); ++j)
            (*ret.pData_)[j][i] *= sum;
    }
#endif

    return ret;
}
/*! 
    @brief Function to resolve base Matrix to u * w * v^T
    @param[in] u AQLMatrix class after it is resolved
    @param[in] w AQLMatrix class after it is resolved
    @param[in] v AQLMatrix class after it is resolved
*/
void 
AQLMatrix:: svDecomp(AQLMatrix& u, AQLMatrix& w, AQLMatrix& v) const
{
#ifdef USE_QUANTLIB_SVD
    unsigned int is_qlib_used = 1;

    // decide which SVD is used, M-Lib or Q-Lib.
    {
        char *svdmode = getenv("SVDMODE");
        if(svdmode != NULL && strcmp(svdmode, "AQ") == 0){
            printf("!!!M-Lib SVD is used.\n");
            is_qlib_used = 0;
        }
    }
#endif
    
    if (row() >= column()) {
        u = AQLMatrix(*this);
    } else {
//      cout << "Matrix's Row is not larger than Column\n";
        u = AQLMatrix(column(), column());
        u.clearValues();
        for (unsigned int i = 0; i < row(); i++) {
            for (unsigned int j = 0; j < column(); j++) {
                u.setValue(i, j, (*this)[i][j]);
            }
        }
    }
    w = AQLMatrix(column(), column()).clearValues();
    v = AQLMatrix(column(), column()).clearValues();
    vector<double> w1;
    try {
        w1.resize(column());
    } 
    catch (...)
    {
        throw AQLCoreSystemError(__FILE__,__LINE__);
    }

#ifndef USE_QUANTLIB_SVD
	svdcmp(u, w1, v);
	for (unsigned int i = 0; i < column(); i++) {
		w.setValue(i, i, w1[i]);
	}
#else
    if(!is_qlib_used){
        svdcmp(u, w1, v);
        for (unsigned int i = 0; i < column(); i++) {
            w.setValue(i, i, w1[i]);
        }
    }else{
	vector<double> _a;
	vector<double> _u;
	vector<double> _v;
	vector<double> s_;
	_a.resize(column() * column());
	_u.resize(column() * column());
	_v.resize(column() * column());
	s_.resize(column());

    for (unsigned int i = 0; i < row(); i++) {
        for (unsigned int j = 0; j < column(); j++) {
			_a[i * column() + j] = u.getValue(i, j);
        }
    }

    SVD(row(), column(), _a, _u, s_, _v);

    for (unsigned int i = 0; i < row(); i++) {
        w.setValue(i, i, s_[i]);
        for (unsigned int j = 0; j < column(); j++) {
			u.setValue(i, j, _u[i * column() + j]);
			v.setValue(i, j, _v[i * column() + j]);
        }
    }
    }
#endif
}

/*! 
    @brief Function to calculate eigenvalue and eigenvector \n 
    resolves base Matrix to vec^T * val * vec.
    @param[in] vec AQLMatrix class after it is resolved
    @param[in] val AQLMatrix class after it is resolved
*/
void 
AQLMatrix::eigenMatrix(AQLMatrix& vec, AQLMatrix& val) const
{
    if (! isSymmetric()) 
    {
        throw AQLCoreNumericalError("matrix is Not symmetric one", 
                                        __FILE__, __LINE__);
    }

    AQLMatrix d(row(), column());
    d.clearValues();

    vector<double> e;
    try {
        e.resize(column());
    } 
    catch (...)
    {
        throw AQLCoreSystemError(__FILE__,__LINE__);
    }
    vec = AQLMatrix(*this);
    val = AQLMatrix(1, column());
    val.clearValues();

    // transpose to tri-diagonal
    tred2(vec, d, e);
    
    tqli(d, e, vec);
    for (unsigned int i = 0; i < column(); i++) {
        val.setValue(0, i, d[i][i]);
    }
//  eigsrt(val, vec);
    return;
}

/*! 
    @brief Input
*/
AQLMatrix&
AQLMatrix::operator =(const AQLMatrix& other)
{
    return copy(other);
}

/*! 
    @brief Product Matrix
*/
AQLMatrix 
AQLMatrix::operator *(const AQLMatrix &other) const
{
    if (column() != other.row()) 
    {
        throw 
        AQLCoreNumericalError("Can not multiply", __FILE__, __LINE__);
    }
    AQLMatrix ret(row(), other.column());
    unsigned int i, j, k;
    
    for (i = 0; i < row(); i++) {
        for (j = 0; j < other.column(); j++) {
            (*ret.pData_)[i][j] = 0.0;
            for (k = 0; k < column(); k++) {
                (*ret.pData_)[i][j] += 
                    (*pData_)[i][k] * (*other.pData_)[k][j];
            }
        }
    }
    return ret;
}
/*! 
    @brief Operator to add for each Matrix
*/
AQLMatrix 
AQLMatrix::operator +(const AQLMatrix &other) const
{
    if (row() != other.row() || 
        column() != other.column()) 
    {
        throw 
            AQLCoreNumericalError("Can not add", __FILE__, __LINE__);
    }
    AQLMatrix ret(row(),  column());
    unsigned int i, j;
    
    for (i = 0; i < row(); i++) {
        for (j = 0; j < column(); j++) {
            (*ret.pData_)[i][j] = 
                (*pData_)[i][j] + (*other.pData_)[i][j];
        }
    }   
    return ret;
}

/*! 
    @brief Operator to Subtract for each Matrix
*/
AQLMatrix 
AQLMatrix::operator -(const AQLMatrix& other) const
{
    if (row() != other.row() || 
        column() != other.column()) 
    {
        throw 
            AQLCoreNumericalError("Can not subtract", __FILE__, __LINE__);
    }
    AQLMatrix ret(row(),  column());
    unsigned int i, j;
    
    for (i = 0; i < row(); i++) {
        for (j = 0; j < column(); j++) {
            (*ret.pData_)[i][j] = 
                (*pData_)[i][j] - (*other.pData_)[i][j];
        }
    }   
    return ret;
}
/*! 
    @brief input operation to malutiplier for each Matrix
*/
AQLMatrix& 
AQLMatrix::operator *=(const AQLMatrix &other)
{
    if (column() != other.row()) {
        throw 
        AQLCoreError("Can not multiply", __FILE__, __LINE__);
    }
    makeUnShared();
    // Keep the old buffer alive via a second shared_ptr (a cheap refcount bump) while computing
    // into a freshly allocated one - each output element needs a full row/column of the original,
    // so this can't be done in place. If make_shared throws, pData_ is simply never reassigned
    // (shared_ptr's own exception guarantee) - no manual try/catch/restore needed the way the raw
    // new/delete version required, and no manual delete needed either: tmp cleans itself up when
    // it goes out of scope.
    std::shared_ptr<AQLMatrixData> tmp = pData_;
    pData_ = std::make_shared<AQLMatrixData>(row(), other.column());
    unsigned int i, j, k;

    for (i = 0; i < row(); i++) {
        for (j = 0; j < other.column(); j++) {
            (*pData_)[i][j] = 0.0;
            for (k = 0; k <  other.row(); k++) {
                (*pData_)[i][j] += (*tmp)[i][k] * (*other.pData_)[k][j];
            }
        }
    }
    return *this;
}

/*! 
    @brief input operation to add for each Matrix
*/
AQLMatrix& 
AQLMatrix::operator +=(const AQLMatrix &other)
{
    if (column() != other.column() || row() != other.row()) {
        throw 
        AQLCoreNumericalError("Can not add", __FILE__, __LINE__);
    }
    makeUnShared();
    unsigned  int i, j;
    
    for (i = 0; i < row(); i++) {
        for (j = 0; j < column(); j++) {
            (*pData_)[i][j] += (*other.pData_)[i][j];
        }
    }   
    return *this;
}
/*! 
    @brief input operation to Subtract for each Matrix

*/
AQLMatrix& 
AQLMatrix::operator-=(const AQLMatrix& other)
{
    if (column() != other.row()) 
    {
        throw 
        AQLCoreNumericalError("Can not subtract", __FILE__, __LINE__);
    }
    makeUnShared();
    unsigned int i, j;

    for (i = 0; i < row(); i++) {
        for (j = 0; j < column(); j++) {
            (*pData_)[i][j] -= (*other.pData_)[i][j];
        }
    }   

    return *this;
}

/*! 
    @brief the Function to calculate eigenvalue and eigenvector 
*/
AQLMatrix 
AQLMatrix::operator *(const double& x) const
{
    AQLMatrix ret(*this);
    ret *= x;
    return ret;
}

/*! 
    @brief malutiplier the constant to Matrix
*/
AQLMatrix& AQLMatrix::operator *=(const double& x)
{
    makeUnShared();
    const int rowCount = static_cast<int>(row());
    const unsigned int colCount = column();
#ifdef _OPENMP
    #pragma omp parallel for if(rowCount > OPENMP_SIZE_THRESHOLD)
#endif
    for (int i = 0; i < rowCount; ++i)
    {
        double* rowPtr = (*pData_)[static_cast<unsigned int>(i)];
        for (unsigned int j = 0; j < colCount; ++j)
        {
            rowPtr[j] *= x;
        }
    }
    return *this;
}

/*! 
    @brief Comparing operation for Matrix
*/
bool 
AQLMatrix::operator ==(const AQLMatrix &other) const 
{
    if (row() != other.row() || 
        column() != other.column()) {
        return false;
    }
    unsigned int i, j;
    
    for (i = 0; i < row(); i++) {
        for (j = 0; j < column(); j++) {
            if ((*pData_)[i][j] != (*other.pData_)[i][j]) {
                return false;
            }
        }
    }   
    return true;
}
/*! 
    @brief the Function to calculate eigenvalue and eigenvector \n 
*/
void 
AQLMatrix::print(const char* file) const
{
    unsigned int i, j;

    FILE* fp = fopen(file, "w");
    if (fp == NULL)
    {
        throw AQLCoreSystemError(__FILE__,__LINE__);
    }
    
    for (i = 0; i < row()  - 1; i++) {
        for (j = 0; j < column() - 1; j++) {
            FPRINTF(fp, "%.18f,", (*pData_)[i][j]);
        }
        FPRINTF(fp, "%.18f\n", (*pData_)[i][j]);
    }
    for (j = 0; j < column() - 1; j++) {
        FPRINTF(fp, "%.18f,", (*pData_)[i][j]);
    }

    FPRINTF(fp, "%.18f\n", (*pData_)[i][j]);
    fclose(fp);
}
///////////// PRIVATE METHODS ////////////////////////////
// When data is shared, an original area is secured.
void
AQLMatrix::makeUnShared() const
{
    if (!pData_ || pData_.use_count() == 1) return;
    std::shared_ptr<AQLMatrixData> wk = pData_;
    pData_ = std::make_shared<AQLMatrixData>(wk->row(), wk->col());
    unsigned int i,j;
    for (i = 0; i < row(); ++i)
    {
        for (j = 0; j < column(); ++j)
        {
            (*pData_)[i][j] = (*wk)[i][j];
        }
    }
}
//
void
AQLMatrix::clear(void) // Call from destructor
{
    pData_.reset();
}

AQLMatrix&
AQLMatrix::copy(const AQLMatrix& mat)
{
    if (this != &mat)
	{
		pData_ = mat.pData_;
	}
    return *this;
}

// Move assignment - same rationale as the move constructor (AQLMatrix.h's declaration comment).
AQLMatrix&
AQLMatrix::operator=(AQLMatrix&& mat) noexcept
{
    if (this != &mat)
    {
        pData_ = std::move(mat.pData_);
    }
    return *this;
}
/*! 
    @brief Matrix * constant 
    @param[in] 
    @param[in]
    @param[in]
*/
// ret is copy of the Matrix. indx is the Matrix(memory size is itsRow)
void 
AQLMatrix::ludcmp(AQLMatrix& ret, std::vector<int>& indx, double& d)
{
    unsigned int i,imax = 0,j,k;
    double big,dum,sum,temp;
    vector<double> vv;
    try
    {
        vv.resize(ret.row());
    }
    catch(...)
    {
        throw AQLCoreSystemError(__FILE__,__LINE__);
    }

    d=double(1);
    for (i=0;i<ret.row();i++)
    {
        big=double(0);
        for (j=0;j<ret.row();j++)
        {
            if ((temp=double(AQLMath::abs(ret[i][j]))) > big) big=temp;
        }
        if (big == double(0))
        {
            throw AQLCoreNumericalError("Invalid Data Configuration: Bad Config and/or Bad Data has lead to a Numerical Error; Singlular matrix in LU Decomposition routine.", __FILE__, __LINE__);
        }
        vv[i]=double(1)/big;
    }
    for (j=0;j<ret.row();j++)
    {
        for (i=0;i<j;i++)
        {
            sum=ret[i][j];
            for (k=0;k<i;k++) sum -= ret[i][k]*ret[k][j];
            ret.setValue(i, j, sum);
        }
        big=double(0);
        for (i=j;i<ret.row();i++)
        {
            sum=ret[i][j];
            for (k=0;k<j;k++) sum -= ret[i][k]*ret[k][j];
            ret.setValue(i, j, sum);
            if ( (dum=vv[i]*double(AQLMath::abs(double(sum)))) >= big)
            {
                big=dum;
                imax=i;
            }
        }
        if (j != imax)
        {
            for (k=0;k<ret.row();k++)
            {
                dum=ret[imax][k];
                ret.setValue(imax, k, ret[j][k]);
                ret.setValue(j, k, dum);
            }
            d = -(d);
            vv[imax]=vv[j];
        }
        indx[j] = imax;
        if (ret[j][j] == 0.0)
        {
            double r = ret[j][j];
            AQLMath::minValue(r);
            ret.setValue(j, j, r);
        }

        if (j != ret.row())
        {
            dum=double(1)/(ret[j][j]);
            for (i=j+1;i<ret.row();i++) ret.setValue(i, j, ret[i][j] * dum);
        }
    }
}

// input ret of ludcmp to a, input indx to indx, b(1. itsRow) is the answer
void 
AQLMatrix::lubksb(const AQLMatrix& a, const std::vector<int>& indx,std::vector<double>& b)
{
    /*unsigned */int i,ii=0,ip,j;
    double sum;
    
    for (i=0;i<(int)a.row(); i++) {
        ip=indx[i];
        sum=b[ip];
        b[ip]=b[i];
        if (ii) {
            for (j=ii-1;j<=i-1;j++) sum -= a[i][j]*b[j];
        } else if (sum) ii=i+1;
        b[i]=sum;
    }
    for (i=(int)a.row()-1;i>=0;i--) {
        sum=b[i];
        for (j=i+1;j<(int)a.column();j++) sum -= a[i][j]*b[j];
        b[i]=sum/a[i][i];
    }
}

void 
AQLMatrix::mprove(const AQLMatrix& a, const AQLMatrix& alud, 
												const std::vector<int>& indx, 
												const std::vector<double>& b, 
												std::vector<double>& x)
{
    unsigned int j,i;
    double sdp;
    vector<double> r;
    try {
        r.resize(a.row());
    }
    catch(...)
    {
        throw AQLCoreSystemError(__FILE__, __LINE__);
    }

    for (i=0;i<a.row();i++) {
        sdp = -b[i];
        for (j=0;j<a.row();j++) {
            sdp += a[i][j]*x[j];
        }
        r[i]=sdp;
    }
    lubksb(alud,indx,r);
    for (i=0;i<a.row();i++) {
        x[i] -= r[i];
    }
}

void 
AQLMatrix::svdcmp(AQLMatrix& a, std::vector<double>& w, AQLMatrix& v)
{

    /*unsigned */int flag,i,its,j,jj,k,l=0,nm=0;
    double anorm,c,f,g,h,s,x,y,z;
    double scale;
    vector<double> rv1;
    rv1.resize(a.column());
    
    g=scale=anorm=0.0;

    for (i=0;i<(int)a.column();i++) { 
        l=i+1;
        rv1[i]=scale*g;
        g=s=double(0);
        scale=0.0;

        if (i < (int)a.row()) {
            for (k=i;k< (int)a.row();k++) {
                scale += AQLMath::abs(double(a[k][i]));
            }
            if (scale) {
                for (k=i;k<(int)a.row();k++) {
                    a.setValue(k, i, a[k][i] / double(scale));
                    s += a[k][i]*a[k][i];
                }
                f=a[i][i];
                g = -AQLMath::sign(AQLMath::sqrt(double(s)), f);
                h=f*g-s;
                a.setValue(i, i, f-g);
                for (j=l;j<(int)a.column();j++) {
                    for (s=double(0),k=i;k<(int)a.row();k++) {
                        s += a[k][i]*a[k][j];
                    }
                    f=s/h;
                    for (k=i;k<(int)a.row();k++) {
                        a.setValue(k, j, a[k][j] + f*a[k][i]);
                    }
                }
                for (k=i;k<(int)a.row();k++) {
                    a.setValue(k, i, a[k][i] * double(scale));
                }
            }
        }

        w[i]=double(scale) * g;
        g=s=double(0);
        scale=0.0;
        if (i < (int)a.row() && i != (int)a.column() - 1) {
            for (k=l;k<(int)a.column();k++) {
                scale += AQLMath::abs(double(a[i][k]));
            }
            if (scale) {
                for (k=l;k<(int)a.column();k++) {
                    a.setValue(i, k, a[i][k] / double(scale));
                    s += a[i][k]*a[i][k];
                }
                f=a[i][l];
                g = -AQLMath::sign(AQLMath::sqrt(double(s)),f);
                h=f*g-s;
                a.setValue(i, l, f-g);
                for (k=l;k<(int)a.column();k++) rv1[k]=a[i][k]/h;
                for (j=l;j<(int)a.row();j++) {
                    for (s=double(0),k=l;k<(int)a.column();k++) {
                        s += a[j][k]*a[i][k];
                    }
                    for (k=l;k<(int)a.column();k++) {
                        a.setValue(j,k, a[j][k] + s*rv1[k]);
                    }
                }
                for (k=l;k<(int)a.column();k++) {
                    a.setValue(i, k, a[i][k] * double(scale));
                }
            }
        }
        anorm=AQLMath::max(anorm,
            double(AQLMath::abs(double(w[i]))+
                   AQLMath::abs(double(rv1[i]))));
    }

    for (i=(int)a.column() - 1;i>=0;i--) {
        if (i < (int)a.column() - 1) {
            if (g) {
                for (j=l;j<(int)a.column();j++) {
                    v.setValue(j, i, (a[i][j]/a[i][l])/g);
                }
                for (j=l;j<(int)a.column();j++) {
                    for (s=double(0),k=l;k< (int)a.column();k++) 
                        s += a[i][k]*v[k][j];
                    for (k=l;k<(int)a.column();k++) {
                        v.setValue(k, j, v[k][j] + s*v[k][i]);
                    }
                }
            }
            for (j=l;j<(int)a.column();j++) {
                v.setValue(i, j, 0.0);
                v.setValue(j, i, 0.0);
            }
        }
        v.setValue(i, i, double(1));
        g=rv1[i];
        l=i;
    }

    for (i=int(AQLMath::min(a.row(),a.column())) - 1;i>=0;i--) { 
        l=i+1;
        g=w[i];
        for (j=l;j<(int)a.column();j++) a.setValue(i, j, double(0));
        if (g) {
            g=double(1)/g;
            for (j=l;j<(int)a.column();j++) {
                for (s=double(0),k=l;k<(int)a.row();k++) {
                    s += a[k][i]*a[k][j];
                }
                f=(s/a[i][i])*g;
                for (k=i;k<(int)a.row();k++) {
                    a.setValue(k, j, a[k][j] + f*a[k][i]);
                }
            }
            for (j=i;j<(int)a.row();j++) a.setValue(j, i, a[j][i] * g);
        } else for (j=i;j<(int)a.row();j++) a.setValue(j, i, 0.0);
        a.setValue(i, i, a[i][i]+1);
    }

    for (k=(int)a.column() - 1;k>=0;k--) {
        for (its=1;its<=ITERATION;its++) {
            flag=1;
            for (l=k;l>=0;l--) {
                nm=l-1;
                if ((double)(AQLMath::abs(double(rv1[l]))+anorm) == anorm) {
                    flag=0;
                    break;
                }
                if ((double)(AQLMath::abs(double(w[nm]))+anorm) == anorm) break;
            }
            if (flag) {
                c=double(0);
                s=double(1);
                for (i=(int)l;i<=(int)k;i++) {
                    f=s*rv1[i];
                    rv1[i]=c*rv1[i];
                    if ((double)(AQLMath::abs(double(f))+anorm) == anorm) break;
                    g=w[i];
                    h=AQLMath::pythag(f,g);
                    w[i]=h;
                    h=double(1)/h;
                    c=g*h;
                    s = -f*h;
                    for (j=0;j<(int)a.row();j++) {
                        y=a[j][nm];
                        z=a[j][i];
                        a.setValue(j, nm, y*c+z*s);
                        a.setValue(j, i, z*c-y*s);
                    }
                }
            }
            z=w[k];
            if ((unsigned int)l == (unsigned int)k) {
                if (z < double(0)) {
                    w[k] = -z;
                    for (j=0;j<(int)a.column();j++) {
                        v.setValue(j, k, -v[j][k]);
                    }
                }
                break;
            }

            if (its == ITERATION) {
                throw AQLCoreNumericalError(
                        "Error in svdcmp. Too many iterations", 
                            __FILE__, __LINE__);
            }

            x=w[l];
            nm=k-1;
            y=w[nm];
            g=rv1[nm];
            h=rv1[k];
            f=((y-z)*(y+z)+(g-h)*(g+h))/(double(2)*h*y);
            g=AQLMath::pythag(f,double(1));
            f=((x-z)*(x+z)+h*((y/(f+AQLMath::sign(g,f)))-h))/x;
            c=s=double(1);
            for (j=l;j<=nm;j++) {
                i=j+1;
                g=rv1[i];
                y=w[i];
                h=s*g;
                g=c*g;
                z=AQLMath::pythag(f,h);
                rv1[j]=z;
                c=f/z;
                s=h/z;
                f=x*c+g*s;
                g = g*c-x*s;
                h=y*s;
                y *= c;
                for (jj=0;jj<(int)a.column();jj++) {
                    x=v[jj][j];
                    z=v[jj][i];
                    v.setValue(jj, j, x*c+z*s);
                    v.setValue(jj, i, z*c-x*s);
                }
                z=AQLMath::pythag(f,h);
                w[j]=z;
                if (z) {
                    z=1.0/z;
                    c=f*z;
                    s=h*z;
                }
                f=c*g+s*y;
                x=c*y-s*g;
                for (jj=0;jj<(int)a.row();jj++) {
                    y=a[jj][j];
                    z=a[jj][i];
                    a.setValue(jj, j, y*c+z*s);
                    a.setValue(jj, i, z*c-y*s);
                }
            }
            rv1[l]=0.0;
            rv1[k]=f;
            w[k]=x;
        }
    }
}

void 
AQLMatrix::tred2(AQLMatrix& a, AQLMatrix& d, std::vector<double>& e)
{
    int l, k, j, i;

    double scale,hh,h,g,f;

    for (i = (int)a.row() - 1; i > 0; i--) 
	{
        l = i - 1;
        h = scale = double(0);
        if (l > 0) {
            for (k = 0; k <= l; k++)
                scale += double(AQLMath::abs(double(a[i][k])));
            if (scale == 0.0)
                e[i] = a[i][l];
            else {
                for (k = 0; k <= l; k++) {
                    a.setValue(i, k, a[i][k] / scale);
                    h += a[i][k]*a[i][k];
                }
                f = a[i][l];
                g = double(f >= double(0) ? -AQLMath::sqrt(double(h)) : 
                                            AQLMath::sqrt(double(h)));
                e[i] = scale * g;
                h -= f * g;
                a.setValue(i, l, f-g);
                f = double(0);
                for (j = 0; j <= l; j++) {
                    a.setValue(j, i, a[i][j] / h);
                    g = double(0);
                    for (k = 0; k <= j; k++)
                        g += a[j][k] * a[i][k];
                    for (k = j + 1; k <= l; k++)
                        g += a[k][j] * a[i][k];
                    e[j] = g / h;
                    f += e[j] * a[i][j];
                }
                hh = f / (h + h);
                for (j = 0; j <= l; j++) {
                    f = a[i][j];
                    g = e[j] - hh * f;
                    e[j] = g;
                    for (k = 0; k <= j; k++)
                        a.setValue(j, k, 
                                a[j][k] - (f * e[k] + g * a[i][k]));
                }
            }
        } 
		else 
		{
            e[i] = a[i][l];
        }
        d.setValue(i, i, h);
    }

    d.setValue(0, 0, double(0));
    e[0] = double(0);
    // Contents of this loop can be omitted if eigenvectors not
    //      wanted except for statement d[i]=a[i][i]; 
    for (i = 0; i < (int)a.row(); i++) 
	{
        l = i - 1;
        if (d[i][i]) 
		{
            for (j = 0; j <= l; j++) 
			{
                g = double(0);
                for (k = 0; k <= l; k++)
                    g += a[i][k] * a[k][j];
                for (k = 0; k <= l; k++)
                    a.setValue(k, j, a[k][j] - g * a[k][i]);
            }
        }
        d.setValue(i, i, a[i][i]);
        a.setValue(i, i, double(1));
        for (j = 0; j <= l; j++) 
		{
            a.setValue(j, i, double(0));
            a.setValue(i, j, double(0));
        }
    }
    for (i = 0; i < (int)d.column() -1; i++) 
	{
        d.setValue(i, i + 1, e[i+1]);
        d.setValue(i + 1, i, e[i+1]);
    }
}

void 
AQLMatrix::tqli(AQLMatrix& d, std::vector<double>& e,  AQLMatrix& z)
{
    /*unsigned*/ int m,l,iter,i,k;
    double s,r,p,g,f,dd,c,b;
    
    for (i = 1; i < (int)e.size(); i++) e[i - 1] = e[i];
    e[i - 1] = double(0);
    
    for (l=0;l< (int)z.row();l++) {
        iter=0;
        do {
            for (m=l;m< (int)z.column()-1;m++) {
                dd=double(AQLMath::abs(double(d[m][m]))+ 
                                    AQLMath::abs(double(d[m+1][m+1])));
                if ((double(AQLMath::abs(double(e[m])))+dd) == dd) 
                                break;
            }
            if (m != l) {
                if (iter++ == ITERATION) {
                    throw AQLCoreNumericalError(
                        "Error in tqli. Too many iterations in tqli", 
                            __FILE__, __LINE__);
                }
                g=(d[l+1][l+1]-d[l][l])/(double(2)*e[l]);
                r=AQLMath::pythag(g,double(1));
                g=d[m][m]-d[l][l]+e[l]/(g+AQLMath::sign(r,g));
                s=c=double(1);  
                p = double(0);
                for (i=m-1;i>=l;i--) {
                    f=s*e[i];
                    b=c*e[i];
                    e[i+1]=(r=AQLMath::pythag(f,g));
                    if (r == double(0)) {
                        d.setValue(i+1, i+1, d[i+1][i+1] - p);
                        e[m]=double(0);
                        break;
                    }
                    s=f/r;
                    c=g/r;
                    g=d[i+1][i+1]-p;
                    r=(d[i][i]-g)*s+2.0*c*b;
                    d.setValue(i+1, i+1, g+(p=s*r));
                    g=c*r-b;
                    for (k=0;k<(int)z.row();k++) {
                        f=z[k][i+1];
                        z.setValue(k, i+1, s*z[k][i]+c*f);
                        z.setValue(k, i, c*z[k][i]-s*f);
                    }
                }
                if (r == double(0) && i >= l) continue;
                d.setValue(l, l, d[l][l] - p);
                e[l]=g;
                e[m]=0.0;
            }
        } while (m != l);
    }
}

#if 0
void AQLMatrix::eigsrt(AQLMatrix& d1, AQLMatrix& v1)
{
    int k,j,i;
    double p;
    AQLMatrixData& d = *(d1.pData_);
    AQLMatrixData& v = *(v1.pData_);

    for (i=0;i< d.col();i++) {
        p=d[0][k=i];
        for (j=i+1;j< v.col();j++)
            if (AQLMath::abs(double(d[0][j])) >= 
                    AQLMath::abs(double(p))) p=d[0][k=j];
        if (k != i) {
            d[0][k]=d[0][i];
            d[0][i]=p;
            for (j=0;j<v.row();j++) {
                p=v[j][i];
                v[j][i]=v[j][k];
                v[j][k]=p;
            }
        }
    }
}
#endif

#ifdef USE_QUANTLIB_SVD
void
AQLMatrix::SVD(int m_, int n_, std::vector<double>& _a, std::vector<double>& _u, std::vector<double>& s_, std::vector<double>& _v){
    int i, j, k;

    using std::swap;

	AQLMatrix A(m_, n_);
	AQLMatrix U_(m_, n_);
	AQLMatrix V_(m_, n_);

    for (i = 0; i < m_; i++) {
        for (j = 0; j < m_; j++) {
            A.setValue(i, j, _a[i * n_ + j]);
        }
    }
        
    //Matrix A;

    /* The implementation requires that rows > columns.
       If this is not the case, we decompose M^T instead.
       Swapping the resulting U and V gives the desired
       result for M as

       M^T = U S V^T           (decomposition of M^T)

       M = (U S V^T)^T         (transpose)

       M = (V^T^T S^T U^T)     ((AB)^T = B^T A^T)

       M = V S U^T             (idempotence of transposition,
       symmetry of diagonal matrix S)

    */
	U_.setValue(0.0);
	V_.setValue(0.0);
    //Array e(n_);
    vector<double> e;
	e.resize(n_);
    //Array work(m_);
    vector<double> work;
	work.resize(m_);


    // Reduce A to bidiagonal form, storing the diagonal elements
    // in s and the super-diagonal elements in e.

    int nct = AQLMath::min(m_ - 1, n_);
    int nrt = AQLMath::max(0, n_ - 2);
    for (k = 0; k < AQLMath::max(nct, nrt); k++) 
	{
        if (k < nct) 
		{
            // Compute the transformation for the k-th column and
            // place the k-th diagonal in s[k].
            // Compute 2-norm of k-th column without under/overflow.
            s_[k] = 0;
            for (i = k; i < m_; i++) 
			{
                s_[k] = AQLMath::pythag(s_[k], A[i][k]);
            }
            if (s_[k] != 0.0)
			{
                if (A[k][k] < 0.0)
				{
                    s_[k] = -s_[k];
                }
                for (i = k; i < m_; i++) 
				{
                    A.setValue(i, k, A[i][k] / s_[k]);
                }
                A.setValue(k, k, A[k][k] + 1.0);
            }
            s_[k] = -s_[k];
        }
        for (j = k + 1; j < n_; j++) 
		{
            if ((k < nct) && (s_[k] != 0.0))
			{
                // Apply the transformation.
                double t = 0;
                for (i = k; i < m_; i++) 
				{
                    t += A[i][k] * A[i][j];
                }
                t = -t / A[k][k];
                for (i = k; i < m_; i++) 
				{
                    A.setValue(i, j, A[i][j] + t * A[i][k]);
                }
            }

            // Place the k-th row of A into e for the
            // subsequent calculation of the row transformation.

            e[j] = A[k][j];
        }
        if (k < nct) 
		{

            // Place the transformation in U for subsequent back
            // multiplication.

            for (i = k; i < m_; i++) 
			{
				U_.setValue(i, k, A[i][k]);    
            }
        }
        if (k < nrt) 
		{
            // Compute the k-th row transformation and place the
            // k-th super-diagonal in e[k].
            // Compute 2-norm without under/overflow.
            e[k] = 0;
            for (i = k+1; i < n_; i++) 
			{
                e[k] = AQLMath::pythag(e[k],e[i]);
            }
            if (e[k] != 0.0) 
			{
                if (e[k+1] < 0.0) 
				{
                    e[k] = -e[k];
                }
                for (i = k+1; i < n_; i++)
				{
                    e[i] /= e[k];
                }
                e[k+1] += 1.0;
            }
            e[k] = -e[k];
            if ((k+1 < m_) & (e[k] != 0.0))
			{
                // Apply the transformation.
                for (i = k+1; i < m_; i++) 
				{
                    work[i] = 0.0;
                }
                for (j = k+1; j < n_; j++)
				{
                    for (i = k+1; i < m_; i++)
					{
                        work[i] += e[j]*A[i][j];
                    }
                }
                for (j = k+1; j < n_; j++)
				{
                    double t = -e[j]/e[k+1];
                    for (i = k+1; i < m_; i++)
					{
						A.setValue(i, j, A[i][j] + t * work[i]);
                    }
                }
            }
            // Place the transformation in V for subsequent
            // back multiplication.
            for (i = k+1; i < n_; i++)
			{
				V_.setValue(i, k, e[i]);
            }
        }
    }

    // Set up the final bidiagonal matrix or order n.
    if (nct < n_)
	{
        s_[nct] = A[nct][nct];
    }
    if (nrt+1 < n_)
	{
        e[nrt] = A[nrt][n_ - 1];
    }
    e[n_-1] = 0.0;

    // generate U

    for (j = nct; j < n_; j++)
	{
        for (i = 0; i < m_; i++)
		{
			U_.setValue(i, j, 0.0);      
        }
		U_.setValue(j, j, 1.0);
    }
    for (k = nct-1; k >= 0; --k)
	{
        if (s_[k] != 0.0)
		{
            for (j = k+1; j < n_; ++j)
			{
                double t = 0;
                for (i = k; i < m_; i++)
				{
                    t += U_[i][k] * U_[i][j];
                }
                t = -t / U_[k][k];
                for (i = k; i < m_; i++)
				{
					U_.setValue(i, j, U_[i][j] + t * U_[i][k]);
                }
            }
            for (i = k; i < m_; i++ )
			{
				U_.setValue(i,k, -U_[i][k]);      
            }
			U_.setValue(k,k, 1.0 + U_[k][k]);
            for (i = 0; i < k-1; i++)
			{
				U_.setValue(i,k, 0.0);
            }
        } 
		else
		{
            for (i = 0; i < m_; i++)
			{
				U_.setValue(i, k, 0.0);
            }
			U_.setValue(k, k, 1.0);
        }
    }

    // generate V
    for (k = n_ - 1; k >= 0; --k)
	{
        if ((k < nrt) & (e[k] != 0.0))
		{
            for (j = k + 1; j < n_; ++j)
			{
                double t = 0;
                for (i = k+1; i < n_; i++)
				{
                    t += V_[i][k] * V_[i][j];
                }
                t = -t / V_[k+1][k];
                for (i = k + 1; i < n_; i++) {
					V_.setValue(i, j, V_[i][j] + t * V_[i][k]);
                }
            }
        }
        for (i = 0; i < n_; i++)
		{
			V_.setValue(i, k, 0.0);  
        }
		V_.setValue(k, k, 1.0);  
    }

    // Main iteration loop for the singular values.
    int p = n_, pp = p-1;
    int iter = 0;
	double eps = AQLMath::pow(2.0,-52.0);
    while (p > 0) {
        int k;
        int kase;

        // Here is where a test for too many iterations would go.

        // This section of the program inspects for
        // negligible elements in the s and e arrays.  On
        // completion the variables kase and k are set as follows.

        // kase = 1     if s(p) and e[k-1] are negligible and k<p
        // kase = 2     if s(k) is negligible and k<p
        // kase = 3     if e[k-1] is negligible, k<p, and
        //              s(k), ..., s(p) are not negligible (qr step).
        // kase = 4     if e(p-1) is negligible (convergence).

        for (k = p - 2; k >= -1; --k)
		{
            if (k == -1)
			{
                break;
            }
            if (AQLMath::abs(e[k]) <= eps*(AQLMath::abs(s_[k]) + AQLMath::abs(s_[k + 1]))) 
			{
                e[k] = 0.0;
                break;
            }
        }
        if (k == p-2) 
		{
            kase = 4;
        }
		else 
		{
            int ks;
            for (ks = p-1; ks >= k; --ks)
			{
                if (ks == k) 
				{
                    break;
                }
                double t = (ks != p ? AQLMath::abs(e[ks]) : 0.) + (ks != k + 1 ? AQLMath::abs(e[ks - 1]) : 0.);
                if (AQLMath::abs(s_[ks]) <= eps * t)
				{
                    s_[ks] = 0.0;
                    break;
                }
            }
            if (ks == k)
			{
                kase = 3;
            }
			else if (ks == p - 1)
			{
                kase = 1;
            }
			else 
			{
                kase = 2;
                k = ks;
            }
        }
        k++;

        // Perform the task indicated by kase.
        switch (kase)
		{
            // Deflate negligible s(p).
            case 1: 
			{
                double f = e[p - 2];
                e[p - 2] = 0.0;
                for (j = p - 2; j >= k; --j)
				{
                    double t = AQLMath::pythag(s_[j], f);
                    double cs = s_[j] / t;
                    double sn = f / t;
                    s_[j] = t;
                    if (j != k) 
					{
                        f = -sn * e[j - 1];
                        e[j - 1] = cs * e[j - 1];
                    }
                    for (i = 0; i < n_; i++)
					{
                        t = cs * V_[i][j] + sn * V_[i][p - 1];
						V_.setValue(i, p - 1, -sn * V_[i][j] + cs * V_[i][p - 1]);            
						V_.setValue(i, j, t);
                    }
                }
            }
            break;

            // Split at negligible s(k).
            case 2: 
			{
                double f = e[k - 1];
                e[k - 1] = 0.0;
                for (j = k; j < p; j++)
				{
                    double t = AQLMath::pythag(s_[j], f);
                    double cs = s_[j] / t;
                    double sn = f / t;
                    s_[j] = t;
                    f = -sn * e[j];
                    e[j] = cs * e[j];
                    for (i = 0; i < m_; i++)
					{
                        t = cs * U_[i][j] + sn * U_[i][k - 1];
						U_.setValue(i, k - 1, -sn * U_[i][j] + cs * U_[i][k - 1]);
						U_.setValue(i, j, t);
                    }
                }
            }
            break;

                // Perform one qr step.
            case 3: 
			{
                // Calculate the shift.
                double scale = AQLMath::max(AQLMath::max(AQLMath::max(
	                            AQLMath::max(AQLMath::abs(s_[p - 1]), AQLMath::abs(s_[p - 2])),
		                            AQLMath::abs(e[p - 2])), AQLMath::abs(s_[k])), AQLMath::abs(e[k]));
                double sp = s_[p - 1] / scale;
                double spm1 = s_[p - 2] / scale;
                double epm1 = e[p - 2] / scale;
                double sk = s_[k] / scale;
                double ek = e[k] / scale;
                double b = ((spm1 + sp) * (spm1 - sp) + epm1 * epm1) / 2.0;
                double c = (sp * epm1) * (sp * epm1);
                double shift = 0.0;
                if ((b != 0.0) | (c != 0.0)) 
				{
                    shift = AQLMath::sqrt(b * b + c);
                    if (b < 0.0) 
					{
                        shift = -shift;
                    }
                    shift = c / (b + shift);
                }
                double f = (sk + sp) * (sk - sp) + shift;
                double g = sk * ek;
                // Chase zeros.
                for (j = k; j < p - 1; j++) 
				{
                    double t = AQLMath::pythag(f, g);
                    double cs = f / t;
                    double sn = g / t;
                    if (j != k) 
					{
                        e[j - 1] = t;
                    }
                    f = cs * s_[j] + sn * e[j];
                    e[j] = cs * e[j] - sn * s_[j];
                    g = sn * s_[j + 1];
                    s_[j + 1] = cs * s_[j + 1];
                    for (i = 0; i < n_; i++)
					{
                        t = cs * V_[i][j] + sn * V_[i][j + 1];
						V_.setValue(i, j + 1, -sn * V_[i][j] + cs * V_[i][j + 1]);
						V_.setValue(i, j, t);
                    }
                    t = AQLMath::pythag(f, g);
                    cs = f / t;
                    sn = g / t;
                    s_[j] = t;
                    f = cs * e[j] + sn * s_[j + 1];
                    s_[j + 1] = -sn * e[j] + cs * s_[j + 1];
                    g = sn * e[j + 1];
                    e[j + 1] = cs * e[j + 1];
                    if (j < m_ -1)
					{
                        for (i = 0; i < m_; i++)
						{
                            t = cs * U_[i][j] + sn * U_[i][j + 1];
							U_.setValue(i, j + 1, -sn * U_[i][j] + cs * U_[i][j + 1]);
							U_.setValue(i, j, t);
                        }
                    }
                }
                e[p - 2] = f;
                iter = iter + 1;
            }
            break;

            // Convergence.
            case 4:
				{
                // Make the singular values positive.
                if (s_[k] <= 0.0)
				{
                    s_[k] = (s_[k] < 0.0 ? -s_[k] : 0.0);
                    for (i = 0; i <= pp; i++)
					{
						V_.setValue(i, k, -V_[i][k]);
                    }
                }

                // Order the singular values.
                while (k < pp) {
                    if (s_[k] >= s_[k + 1])
					{
                        break;
                    }
                    swap(s_[k], s_[k + 1]);
                    if (k < n_ -1)
					{
                        for (i = 0; i < n_; i++)
						{
							double a = V_.getValue(i, k);
							double b = V_.getValue(i, k + 1);
                            swap(a, b);
							V_.setValue(i,k,a);
							V_.setValue(i, k + 1, b);
                        }
                    }
                    if (k < m_-1)
					{
                        for (i = 0; i < m_; i++)
						{
							double c = U_.getValue(i, k);
							double d = U_.getValue(i, k + 1);
                            swap(c, d);
							U_.setValue(i,k,c);
							U_.setValue(i, k + 1, d);
                        }
                    }
                    k++;
                }
                iter = 0;
                --p;
            }
                break;
        }
    }
	for(unsigned i = 0;i < m_; ++i)
	{
		for(unsigned j = 0;j < n_; ++j)
		{
			_u[i * n_ + j] = U_[i][j];
			_v[i * n_ + j] = V_[i][j];
		}
	}
}
#endif
